#include "stdafx.h"		

#include "Avatar.h"
#include "Level.h"
#include "SoundManager.h"
#include "EntityList.h"
#include "EnemyList.h"

#include "FileManager.h"

#define SND_MANAGER sound_manager::instance()

Avatar::Avatar(DOUBLE2 position, Bitmap* BmpPtr, Bitmap* bmpEpicModePtr) :
Entity(position), 
m_BmpPtr(BmpPtr),
m_BmpEpicModePtr(bmpEpicModePtr)
{
    m_RespawnPosition = position;
    m_ActPtr = new PhysicsActor(position, 0, BodyType::DYNAMIC);
    m_ActPtr->SetBullet(true);
    m_ActPtr->AddBoxShape(CLIP_WIDTH - CLIP_WIDTH/4, CLIP_HEIGHT , 0, 0);
    m_ActPtr->SetFixedRotation(true);
    m_ActPtr->SetName(String("Avatar"));
    m_ActPtr->AddContactListener(this);
    
    m_ActPtr->SetGravityScale(m_GravityScale);
    m_ActTriggerPtr = new PhysicsActor(position + DOUBLE2(0, 20 + CLIP_HEIGHT / 2), 0, BodyType::DYNAMIC);
    m_ActTriggerPtr->SetName(String("AvatarGroundTrigger"));
    m_ActTriggerPtr->AddBoxShape(CLIP_WIDTH - 30, 10, 0, 0);
    m_ActTriggerPtr->SetTrigger(true);
    m_ActTriggerPtr->SetGravityScale(0);
    m_ActTriggerPtr->AddContactListener(this);
    
    /*
    * 0 -> Bronze coins
    * 1 -> Silver Coins
    * 2 -> Gold Coins
    * 60 bronze = 1 silver
    * 60 silver = 1 gold
    */
    m_AmountOfCoinsArr[0] = 0;
    m_AmountOfCoinsArr[1] = 0;
    m_AmountOfCoinsArr[2] = 0;

    
    m_SndAttackPtr = sound_manager::instance()->LoadSound(String("Resources/Sound/Entity/Attack.wav"));
    m_SndJumpPtr = SND_MANAGER->LoadSound(String("Resources/Sound/Entity/Jump.wav"));
    m_SndJumpPtr->SetVolume(0.5);
    m_SndWalkPtr = SND_MANAGER->LoadSound(String("Resources/Sound/Entity/running.wav"));

}


Avatar::~Avatar()
{
    
    m_ActTriggerPtr->RemoveContactListener(this);
    delete m_ActTriggerPtr;
    m_ActTriggerPtr = nullptr;

    m_ActPtr->RemoveContactListener(this);
    delete m_ActPtr;
    m_ActPtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Avatar::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actThisPtr == m_ActTriggerPtr && actOtherPtr == m_LevelPtr->GetActor() && m_moveState != moveState::HANGING && m_moveState != moveState::DYING)
    {
        m_NumberOfContactPointsWithLevel++;
        m_moveState = moveState::STANDING;
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("EnemyShooter"))
    {
        SetGameOver();
    }
    if (actThisPtr == m_ActTriggerPtr && actOtherPtr != nullptr && !(actOtherPtr->GetName() == String("Bullet")) && actOtherPtr != m_ActPtr)
    {
        m_JumpCounter = 0;
    }
    if (actThisPtr == m_ActPtr && actOtherPtr->GetName() == String("EnemyBullet"))
    {
        SetGameOver();
    }
    if (actThisPtr != nullptr && actOtherPtr != nullptr && actThisPtr == m_ActPtr &&
        (actOtherPtr->GetName() == String("EnemyBullet")
        || actOtherPtr->GetName() == String("EnemyRotater")
        || actOtherPtr->GetName() == String("EnemyHorizontal")
        || actOtherPtr->GetName() == String("Enemy")
        || actOtherPtr->GetName() == String("EnemyRocket")
        || actOtherPtr->GetName() == String("CameraDeathBounds")
        || actOtherPtr->GetName() == String("Slicer")
        || actOtherPtr == m_LevelPtr->GetLevelBounds()))
    {
        if (m_moveState != moveState::ATTACK)
        {
            SetGameOver();
        }
        if (actOtherPtr->GetName() == String("EnemyRotater") || actOtherPtr->GetName() == String("LevelBounds"))
        {
            SetGameOver();
        }
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("BlockSlide"))
    {
        m_moveState = moveState::SLIDINGSTANDING;
        m_ActPtr->SetLinearVelocity(DOUBLE2(m_ActPtr->GetLinearVelocity().x, 0));
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("MetalFansTrigger"))
    {
        m_moveState = moveState::SLIDINGJUMPING;
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("StickyWall") && actThisPtr == m_ActPtr)
    {
        m_JumpCounter = 0;
        m_moveState = moveState::HANGING;
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("StickyWall") && actThisPtr == m_ActTriggerPtr)
    {
        m_moveState = moveState::STANDING;
    }

}
void Avatar::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_LevelPtr->GetActor() && actThisPtr == m_ActTriggerPtr && actOtherPtr->GetName() != String("EnemyBullet"))
    {
        m_NumberOfContactPointsWithLevel--;
        if (m_NumberOfContactPointsWithLevel < 0)
        {
            m_NumberOfContactPointsWithLevel = 0;
        }
        
        if (m_NumberOfContactPointsWithLevel == 0 && m_moveState != moveState::GOD && m_moveState!= moveState::DYING && m_IsDead == false)
        {
            m_moveState = moveState::JUMPING;
        }
        game_engine::instance()->ConsolePrintString(actThisPtr->GetName() + String(" ended contact with ") + actOtherPtr->GetName() + String(m_NumberOfContactPointsWithLevel));
    }
    if (actOtherPtr != nullptr && actOtherPtr->GetName() == String("StickyWall"))
    {
        m_JumpCounter = 0;
        m_moveState = moveState::JUMPING;
    }
}
void Avatar::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Avatar::Tick(double deltaTime)
{
    m_Position = m_ActPtr->GetPosition();
    if (m_moveState != moveState::DYING)
    {
        m_AccuTime += deltaTime;

        switch (m_moveState)
        {
        
        case Avatar::moveState::STANDING:
            if (m_AccuTime > 1.0 / IDLEFRAMERATE)
            {
                m_FrameNr++;
                m_AccuTime -= 1.0 / IDLEFRAMERATE;
            }
            break;
        case Avatar::moveState::RUNNING:
        case Avatar::moveState::JUMPING:
        case Avatar::moveState::HANGING:
        case Avatar::moveState::ATTACK:
        case Avatar::moveState::DIE:
        case Avatar::moveState::DYING:
        case Avatar::moveState::SLIDINGSTANDING:
        case Avatar::moveState::SLIDINGWALKING:
        case Avatar::moveState::SLIDINGJUMPING:
        case Avatar::moveState::GOD:
            if (m_AccuTime > 1 / m_FrameRate)
            {
                m_FrameNr++;
                m_AccuTime -= 1 / m_FrameRate;
            }
            break;
        default:
            break;
        }
        checkKeys(deltaTime);

        if (m_moveState != moveState::GOD)
        {
            m_ActPtr->SetGravityScale(m_GravityScale);
        }
    }
    else
    {
        m_ActPtr->SetGhost(true);
        m_ActPtr->SetLinearVelocity(DOUBLE2(0, -350));
    }
    m_Position = m_ActPtr->GetPosition();
    m_ActTriggerPtr->SetPosition(m_ActPtr->GetPosition() + DOUBLE2(0, CLIP_HEIGHT / 2));

}
void Avatar::PaintDebug()
{
    MATRIX3X2 matTranslate;
    matTranslate.SetAsTranslate(m_ActPtr->GetPosition());
    game_engine::instance()->SetWorldMatrix(matTranslate);
    switch (m_moveState)
    {
    case Avatar::moveState::RUNNING:
        game_engine::instance()->DrawString(String("Running"), DOUBLE2());
        break;
    case Avatar::moveState::STANDING:
        game_engine::instance()->DrawString(String("Standing"), DOUBLE2());
        break;
    case Avatar::moveState::JUMPING:
        game_engine::instance()->DrawString(String("Jumping"), DOUBLE2());
        break;
    case Avatar::moveState::HANGING:
        game_engine::instance()->DrawString(String("Hanging"), DOUBLE2());
        break;
    case Avatar::moveState::ATTACK:
        game_engine::instance()->DrawString(String("Attack"), DOUBLE2());
        break;
    case Avatar::moveState::DIE:
        game_engine::instance()->DrawString(String("DEAD"), DOUBLE2());
        break;
    case Avatar::moveState::DYING:
        game_engine::instance()->DrawString(String("Dying"), DOUBLE2());
        break;
    case Avatar::moveState::SLIDINGSTANDING:
        game_engine::instance()->DrawString(String("SlidingStanding"), DOUBLE2());
        break;
    case Avatar::moveState::SLIDINGWALKING:
        game_engine::instance()->DrawString(String("SlidingWalking"), DOUBLE2());
        break;
    case Avatar::moveState::SLIDINGJUMPING:
        game_engine::instance()->DrawString(String("SlidingJumping"), DOUBLE2());
        break;
    case Avatar::moveState::GOD:
        game_engine::instance()->DrawString(String("God"), DOUBLE2());
        break;
    default:
        break;
    }
    game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void Avatar::Paint()
{
    MATRIX3X2 matWorldTransform, matScale, matTranslate, matPivot;
    matTranslate.SetAsTranslate(m_ActPtr->GetPosition());
    matPivot.SetAsTranslate(DOUBLE2(-CLIP_WIDTH / 2, -CLIP_HEIGHT / 2));
    matScale.SetAsScale(1);
    if (m_Mirror)
    {
        matScale.SetAsScale(-1,1);
    }
    matWorldTransform = (matPivot * matScale * matTranslate);
    if (game_engine::instance()->IsKeyboardKeyDown(VK_LEFT))
    {     
        m_Mirror = true;
    }
    if (game_engine::instance()->IsKeyboardKeyDown(VK_RIGHT))
    {
        m_Mirror = false;
    }
    
    RECT boundingBox = updateFrameDisplay(m_FrameNr);
    PaintTrail();
    game_engine::instance()->SetWorldMatrix(matWorldTransform);
    
    
    
    game_engine::instance()->DrawBitmap(m_BmpPtr, boundingBox);
    if (m_IsEpicModeOn)
    {
        game_engine::instance()->DrawBitmap(m_BmpEpicModePtr, boundingBox);
    }
    
    
    game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
    matTranslate.SetAsTranslate(DOUBLE2(m_ActPtr->GetPosition().x, 0));
    game_engine::instance()->SetWorldMatrix(matTranslate);
    game_engine::instance()->SetColor(COLOR(0, 0, 0));
}
void Avatar::PaintTrail()
{
    m_deqTrail.push_front(m_ActPtr->GetPosition());

    if (m_deqTrail.size() > MAX_TRAIL)
    {
        m_deqTrail.pop_back();
    }
    game_engine::instance()->SetColor(COLOR(255, 255, 255, 10));
    for (int i = 0, n = int(m_deqTrail.size()); i < n - 1; i++)
    {
        DOUBLE2 pos = m_deqTrail[i];
        DOUBLE2 pos2 = m_deqTrail[i + 1];
        DOUBLE2 midPos = (pos2 + pos) / 2;
        double size = CLIP_WIDTH - 40;
        if (size - (size / MAX_TRAIL)*i > 0)
        {
            
            game_engine::instance()->FillEllipse(pos, size - (size / MAX_TRAIL)*i, size - (size / MAX_TRAIL)*i);
            DOUBLE2 vector = pos2 - pos;
            int numberOfCircles = 4;
            if (vector.Length() > 5)
            {
                for (int j = 0; j < numberOfCircles; j++)
                {
                    double spaceBetween = vector.Length() / numberOfCircles;
                    DOUBLE2 normVector = vector.Normalized();
                    midPos = normVector*j*spaceBetween;
                    game_engine::instance()->FillEllipse(pos + midPos, size - (size / MAX_TRAIL)*i, size - (size / MAX_TRAIL)*i);
                }
            }
        }

    }
}

//! Handles all the keypressed done by the player
void Avatar::checkKeys(double dTime)
{
    if (m_moveState == moveState::GOD)
    {
        GodMoveState(dTime);
    }
    else if (m_moveState == moveState::SLIDINGSTANDING || m_moveState == moveState::SLIDINGWALKING || m_moveState == moveState::SLIDINGJUMPING)
    {
        SlidingMoveState(dTime);
        
    }
    else if (m_moveState != moveState::GOD)
    {
        NormalMoveState(dTime);       
    }   

    if (game_engine::instance()->IsKeyboardKeyPressed(m_God))
    {
        if (m_moveState == moveState::GOD)
        {
            m_ActPtr->SetGravityScale(m_GravityScale);
            m_ActPtr->SetGhost(false);
            m_ActTriggerPtr->SetGhost(false);
            m_moveState = moveState::STANDING;
        }
        else
        {
            m_ActPtr->SetGravityScale(0);
            m_ActPtr->SetGhost(true);
            m_ActTriggerPtr->SetGhost(true);
            m_moveState = moveState::GOD;
        }
    }
    if (game_engine::instance()->IsKeyboardKeyPressed('K') && game_engine::instance()->IsKeyboardKeyPressed('L'))
    {
        m_IsEpicModeOn = !m_IsEpicModeOn;
    }
}
void Avatar::GodMoveState(double dTime)
{
    DOUBLE2 position = m_ActPtr->GetPosition();
    m_ActPtr->SetLinearVelocity(DOUBLE2());
    DOUBLE2 desiredPosition = DOUBLE2();
    if (game_engine::instance()->IsKeyboardKeyDown(VK_RIGHT))    desiredPosition.x = GODMODE_SPEED   * dTime;
    if (game_engine::instance()->IsKeyboardKeyDown(VK_LEFT))    desiredPosition.x = -GODMODE_SPEED  * dTime;
    if (game_engine::instance()->IsKeyboardKeyDown(VK_UP))    desiredPosition.y = -GODMODE_SPEED  * dTime;
    if (game_engine::instance()->IsKeyboardKeyDown(VK_DOWN))    desiredPosition.y = GODMODE_SPEED   * dTime;
    m_ActPtr->SetPosition(position + desiredPosition);
}
void Avatar::SlidingMoveState(double dTime)
{
    if (game_engine::instance()->IsKeyboardKeyDown(m_Right))
    {
        m_ActPtr->ApplyLinearImpulse(DOUBLE2(1, 0) * m_Speed);
    }
    if (game_engine::instance()->IsKeyboardKeyDown(m_Left))
    {
        m_ActPtr->ApplyLinearImpulse(DOUBLE2(-1, 0) * m_Speed);
    }
    DOUBLE2 actVelocity = m_ActPtr->GetLinearVelocity();
    if (game_engine::instance()->IsKeyboardKeyPressed(m_Jump) && (m_moveState == moveState::SLIDINGSTANDING || m_moveState == moveState::SLIDINGWALKING))
    {
        m_moveState = moveState::SLIDINGJUMPING;
        m_SndJumpPtr->Play();
        m_ActPtr->ApplyLinearImpulse(-DOUBLE2(0, m_JumpHeight) * m_ActPtr->GetMass() / PhysicsActor::SCALE);
        m_NumberOfContactPointsWithLevel = 0;

        m_JumpCounter++;
    }
    if (m_moveState == moveState::SLIDINGJUMPING)
    {
        m_ActPtr->ApplyLinearImpulse(DOUBLE2(-actVelocity.x * dTime * 10, 0));
    }
    // Updating the moveState
    if (m_moveState == moveState::SLIDINGSTANDING || m_moveState == moveState::SLIDINGWALKING)
    {
        m_moveState = moveState::SLIDINGSTANDING;
        if (game_engine::instance()->IsKeyboardKeyDown(m_Left))    m_moveState = moveState::SLIDINGWALKING;
        if (game_engine::instance()->IsKeyboardKeyDown(m_Right))    m_moveState = moveState::SLIDINGWALKING;
    }
}
void Avatar::NormalMoveState(double dTime)
{
    DOUBLE2 oldVelocity = m_ActPtr->GetLinearVelocity();
    DOUBLE2 desiredVelocity = DOUBLE2();
    DOUBLE2 dVelocity;

    if (game_engine::instance()->IsKeyboardKeyDown(m_Right))
    {
        desiredVelocity.x = m_Speed;
    }
    if (game_engine::instance()->IsKeyboardKeyDown(m_Left))
    {
        desiredVelocity.x = -m_Speed;
    }
    
    if (game_engine::instance()->IsKeyboardKeyPressed(m_Attack) && m_NumberOfContactPointsWithLevel == 0)
    {
        m_ActPtr->SetLinearVelocity(DOUBLE2(oldVelocity.x, 0));
        m_ActPtr->ApplyLinearImpulse(2 * m_JumpHeight*m_ActPtr->GetMass() / PhysicsActor::SCALE*DOUBLE2(0, 1));
        m_moveState = moveState::ATTACK;
        m_SndAttackPtr->Play();
    }
    DOUBLE2 actVelocity = m_ActPtr->GetLinearVelocity();
    if (game_engine::instance()->IsKeyboardKeyPressed(m_Jump) && m_JumpCounter < MAX_JUMPS && m_moveState != moveState::JUMPING )
    {
        //m_moveState = moveState::JUMPING;
        m_SndJumpPtr->Play();
        desiredVelocity.y = -m_JumpHeight;
        m_ActPtr->SetFriction(0);
        m_NumberOfContactPointsWithLevel = 0;
        m_JumpCounter++;
    }
    dVelocity.x = desiredVelocity.x - oldVelocity.x;
    dVelocity.y = desiredVelocity.y;
    m_ActPtr->ApplyLinearImpulse(m_ActPtr->GetMass() * dVelocity / PhysicsActor::SCALE);

    //Updating the moveState
    if (m_moveState == moveState::RUNNING || m_moveState == moveState::STANDING)
    {
        m_moveState = moveState::STANDING;
        if (game_engine::instance()->IsKeyboardKeyDown(m_Left))    m_moveState = moveState::RUNNING;
        if (game_engine::instance()->IsKeyboardKeyDown(m_Right))    m_moveState = moveState::RUNNING;
    }
    if (game_engine::instance()->IsKeyboardKeyPressed(m_Attack) && m_JumpCounter > 0)m_moveState = moveState::ATTACK;
}


RECT Avatar::updateFrameDisplay(int frameNr)
{
    RECT boundingBox;
    switch (m_moveState)
    {
    case Avatar::moveState::RUNNING:
        frameNr = frameNr % 11;
        boundingBox.left = (frameNr%COLS * CLIP_WIDTH);
        boundingBox.top = ((frameNr / COLS) * CLIP_HEIGHT) + 1;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::STANDING:
        boundingBox.left = 0 + (frameNr % COLS)*CLIP_WIDTH;
        boundingBox.top = 5 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::JUMPING:
        boundingBox.left = 0;
        boundingBox.top = 3 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::GOD:
        boundingBox.left = 0;
        boundingBox.top = 4 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::DYING:
        boundingBox.left = 0;
        boundingBox.top = 3 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::HANGING:
        boundingBox.left = CLIP_WIDTH;
        boundingBox.top = 4 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::SLIDINGSTANDING:
        boundingBox.left = CLIP_WIDTH;
        boundingBox.top = 2 * CLIP_HEIGHT;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    case Avatar::moveState::SLIDINGWALKING:
        frameNr = frameNr % 11;
        boundingBox.left = (frameNr%COLS * CLIP_WIDTH);
        boundingBox.top = ((frameNr / COLS) * CLIP_HEIGHT) + 1;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
        break;
    default:
        boundingBox.left = 0;
        boundingBox.top = 0;
        boundingBox.right = boundingBox.left + CLIP_WIDTH;
        boundingBox.bottom = boundingBox.top + CLIP_HEIGHT;
    }

    return boundingBox;
}
//! Add coins to the avatar
void Avatar::AddAmountOfCoins(int amount)
{
    m_AmountOfCoinsArr[0] += amount;
    while (m_AmountOfCoinsArr[0] > 59)
    {
        m_AmountOfCoinsArr[1]++;
        m_AmountOfCoinsArr[0] -= 60;
    }
    while (m_AmountOfCoinsArr[1] > 59)
    {
        m_AmountOfCoinsArr[2]++;
        m_AmountOfCoinsArr[1] -= 60;
    }
    game_engine::instance()->ConsolePrintString(String("Bronze: ") + String(m_AmountOfCoinsArr[0])
        + String(" Silver: ") + String(m_AmountOfCoinsArr[1])
        + String(" Gold: ") + String(m_AmountOfCoinsArr[2]));
}

//! Gets the avatar BoundingBox
PhysicsActor* Avatar::GetActor()
{
    return m_ActPtr;
}

//! Gets the avatar Position
DOUBLE2 Avatar::GetPosition()
{
    return m_ActPtr->GetPosition();
}

//! Gets the avatarRespawnPosition
DOUBLE2 Avatar::GetRespawnPosition()
{
    return m_RespawnPosition;
}

//! Gets amount of lifes the avatar currently has
int Avatar::GetLifes()
{
    return m_Lifes;
}

//! Gets amount of deaths the avatar has experienced.
int Avatar::GetDeaths()
{
    return m_Deaths;
}

//! Gets raw amount of coins. (No subdivision between copper, silver and gold)
int Avatar::GetAmountOfCoins()
{
    int amountOfCoins = m_AmountOfCoinsArr[0] + 60 * m_AmountOfCoinsArr[1] + 3600 * m_AmountOfCoinsArr[2];
    return amountOfCoins;
}

//! Gets the moveState of the avatar.
Avatar::moveState Avatar::GetMoveState()
{
    return m_moveState;
}

//! Sets the moveState.
//! Possible values: RUNNING,STANDING,JUMPING,HANGING,ATTACK,
//! DIE,DYING,SLIDINGSTANDING,SLIDINGWALKING,SLIDINGJUMPING,GOD
void Avatar::SetMoveState(Avatar::moveState state)
{
    m_moveState = state;
}

//! Sets the respawn Position for the avatar
void Avatar::SetSpawnPosition(DOUBLE2 spawnPosition)
{
    m_RespawnPosition = spawnPosition;
    game_engine::instance()->ConsolePrintString(String("spawn position set to ") + String(spawnPosition.ToString()));
}

//! Resets the avatar back to original
void Avatar::Reset()
{
    m_IsEpicModeOn = false;
    m_ActPtr->AddContactListener(this);
    m_JumpCounter = 0;
    m_NumberOfContactPointsWithLevel = 0;
    m_Deaths++;
    m_Lifes = MAX_LIFES;
    m_moveState = moveState::STANDING;
    m_ActPtr->SetPosition(m_RespawnPosition + DOUBLE2(0,-10));
    m_ActPtr->SetGhost(false);
}

//! Sets the jumpHeight of the avatar.
void Avatar::SetJumpHeight(double height)
{
    m_JumpHeight = height;
}

//! Directly sets the avatarPosition
void Avatar::SetPosition(DOUBLE2 position)
{
    m_Position = position;
    m_ActPtr->SetPosition(position);
}

//! Set amount of lifes
void Avatar::SetLifes(int lifes)
{
    m_Lifes = lifes;
}

//! Sets amount of deaths 
void Avatar::SetDeaths(int deaths)
{
    m_Deaths = 0;
}

// !Set the game Over state or not.
void Avatar::SetGameOver()
{
    m_ActPtr->RemoveContactListener(this);
    m_IsDead = true;
    m_Lifes--;
    m_AccuTime = 0;
    m_moveState = moveState::DYING;
    OutputDebugString(String("You died ") + String(m_Deaths) + String(" times. \n"));
}

//! Sets how many times to avatar has jumped
void Avatar::SetNrJumps(int nr)
{
    m_JumpCounter = nr;
}
//! Sets the keybinds the player specified
void Avatar::SetKeyBinds(FileManager::KeyMap tmpKeyBindsArr)
{
    // Player actions
	m_Jump = tmpKeyBindsArr["jump"];
	m_Attack = tmpKeyBindsArr["attack"];
	m_Left = tmpKeyBindsArr["left"];
	m_Right = tmpKeyBindsArr["right"];
	m_God = tmpKeyBindsArr["god"];
}