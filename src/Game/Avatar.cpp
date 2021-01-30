#include "game.stdafx.h"		

#include "Avatar.h"
#include "Level.h"
#include "SoundManager.h"
#include "EntityList.h"
#include "EnemyList.h"

#include "DataManager.h"

Avatar::Avatar(float2 position, Bitmap* BmpPtr, Bitmap* bmpEpicModePtr) :
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
    m_ActTriggerPtr = new PhysicsActor(position + float2(0, 20 + CLIP_HEIGHT / 2), 0, BodyType::DYNAMIC);
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

    
    m_SndAttackPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/Attack.wav"));
    m_SndJumpPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/Jump.wav"));
    m_SndJumpPtr->set_volume(0.5);
    m_SndWalkPtr = SoundManager::instance()->LoadSound(String("Resources/Sound/Entity/running.wav"));

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
        m_ActPtr->SetLinearVelocity(float2(m_ActPtr->GetLinearVelocity().x, 0));
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
        GameEngine::instance()->print_string(actThisPtr->GetName() + String(" ended contact with ") + actOtherPtr->GetName() + String(m_NumberOfContactPointsWithLevel));
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
        m_ActPtr->SetLinearVelocity(float2(0, -350));
    }
    m_Position = m_ActPtr->GetPosition();
    m_ActTriggerPtr->SetPosition(m_ActPtr->GetPosition() + float2(0, CLIP_HEIGHT / 2));

}
void Avatar::PaintDebug(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate;
    matTranslate = float3x3::translation(m_ActPtr->GetPosition());
    ctx.set_world_matrix(matTranslate);
    switch (m_moveState)
    {
    case Avatar::moveState::RUNNING:
        ctx.draw_string(String("Running"), float2());
        break;
    case Avatar::moveState::STANDING:
        ctx.draw_string(String("Standing"), float2());
        break;
    case Avatar::moveState::JUMPING:
        ctx.draw_string(String("Jumping"), float2());
        break;
    case Avatar::moveState::HANGING:
        ctx.draw_string(String("Hanging"), float2());
        break;
    case Avatar::moveState::ATTACK:
        ctx.draw_string(String("Attack"), float2());
        break;
    case Avatar::moveState::DIE:
        ctx.draw_string(String("DEAD"), float2());
        break;
    case Avatar::moveState::DYING:
        ctx.draw_string(String("Dying"), float2());
        break;
    case Avatar::moveState::SLIDINGSTANDING:
        ctx.draw_string(String("SlidingStanding"), float2());
        break;
    case Avatar::moveState::SLIDINGWALKING:
        ctx.draw_string(String("SlidingWalking"), float2());
        break;
    case Avatar::moveState::SLIDINGJUMPING:
        ctx.draw_string(String("SlidingJumping"), float2());
        break;
    case Avatar::moveState::GOD:
        ctx.draw_string(String("God"), float2());
        break;
    default:
        break;
    }
    ctx.set_world_matrix(float3x3::identity());
}
void Avatar::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matWorldTransform, matScale, matTranslate, matPivot;
    matTranslate = float3x3::translation(m_ActPtr->GetPosition());
    matPivot = float3x3::translation(float2(-CLIP_WIDTH / 2, -CLIP_HEIGHT / 2));
    matScale = float3x3::scale(1);
    if (m_Mirror)
    {
        matScale = float3x3::scale(-1,1);
    }
    // #TODO: Fix avatar mirroring
	auto mat = hlslpp::mul(matPivot, matScale);
	matWorldTransform = hlslpp::mul(mat, matTranslate);
    if (GameEngine::instance()->is_key_down(VK_LEFT))
    {     
        m_Mirror = true;
    }
    if (GameEngine::instance()->is_key_down(VK_RIGHT))
    {
        m_Mirror = false;
    }
    
    RECT boundingBox = updateFrameDisplay(m_FrameNr);
    PaintTrail(ctx);
    ctx.set_world_matrix(matWorldTransform);
    ctx.draw_bitmap(m_BmpPtr, boundingBox);
    if (m_IsEpicModeOn)
    {
        ctx.draw_bitmap(m_BmpEpicModePtr, boundingBox);
    }
    
    
    ctx.set_world_matrix(float3x3::identity());
    matTranslate = float3x3::translation(float2(m_ActPtr->GetPosition().x, 0));
    ctx.set_world_matrix(matTranslate);
    ctx.set_color(COLOR(0, 0, 0));
}
void Avatar::PaintTrail(graphics::D2DRenderContext& ctx)
{
    m_deqTrail.push_front(m_ActPtr->GetPosition());

    if (m_deqTrail.size() > MAX_TRAIL)
    {
        m_deqTrail.pop_back();
    }
    ctx.set_color(COLOR(255, 255, 255, 10));
    for (int i = 0, n = int(m_deqTrail.size()); i < n - 1; i++)
    {
        float2 pos = m_deqTrail[i];
        float2 pos2 = m_deqTrail[i + 1];
        float2 midPos = (pos2 + pos) / 2;
        double size = CLIP_WIDTH - 40;
        if (size - (size / MAX_TRAIL)*i > 0)
        {
            
            ctx.fill_ellipse(pos, size - (size / MAX_TRAIL)*i, size - (size / MAX_TRAIL)*i);
            float2 vector = pos2 - pos;
            int numberOfCircles = 4;
			if (float(hlslpp::length(vector)) > 5.0f)
            {
                for (int j = 0; j < numberOfCircles; j++)
                {
                    double spaceBetween = hlslpp::length(vector) / numberOfCircles;
					float2 normVector = hlslpp::normalize(vector);
                    midPos = normVector*j*spaceBetween;
                    ctx.fill_ellipse(pos + midPos, size - (size / MAX_TRAIL)*i, size - (size / MAX_TRAIL)*i);
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

    if (GameEngine::instance()->is_key_pressed(m_God))
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
    if (GameEngine::instance()->is_key_pressed('K') && GameEngine::instance()->is_key_pressed('L'))
    {
        m_IsEpicModeOn = !m_IsEpicModeOn;
    }
}
void Avatar::GodMoveState(double dTime)
{
    float2 position = m_ActPtr->GetPosition();
    m_ActPtr->SetLinearVelocity(float2());
    float2 desiredPosition = float2();
    if (GameEngine::instance()->is_key_down(VK_RIGHT))    desiredPosition.x = GODMODE_SPEED   * dTime;
    if (GameEngine::instance()->is_key_down(VK_LEFT))    desiredPosition.x = -GODMODE_SPEED  * dTime;
    if (GameEngine::instance()->is_key_down(VK_UP))    desiredPosition.y = -GODMODE_SPEED  * dTime;
    if (GameEngine::instance()->is_key_down(VK_DOWN))    desiredPosition.y = GODMODE_SPEED   * dTime;
    m_ActPtr->SetPosition(position + desiredPosition);
}
void Avatar::SlidingMoveState(double dTime)
{
    if (GameEngine::instance()->is_key_down(m_Right))
    {
        m_ActPtr->ApplyLinearImpulse(float2(1, 0) * m_Speed);
    }
    if (GameEngine::instance()->is_key_down(m_Left))
    {
        m_ActPtr->ApplyLinearImpulse(float2(-1, 0) * m_Speed);
    }
    float2 actVelocity = m_ActPtr->GetLinearVelocity();
    if (GameEngine::instance()->is_key_pressed(m_Jump) && (m_moveState == moveState::SLIDINGSTANDING || m_moveState == moveState::SLIDINGWALKING))
    {
        m_moveState = moveState::SLIDINGJUMPING;
        m_SndJumpPtr->play();
        m_ActPtr->ApplyLinearImpulse(-float2(0, m_JumpHeight) * m_ActPtr->GetMass() / PhysicsActor::SCALE);
        m_NumberOfContactPointsWithLevel = 0;

        m_JumpCounter++;
    }
    if (m_moveState == moveState::SLIDINGJUMPING)
    {
        m_ActPtr->ApplyLinearImpulse(float2(-actVelocity.x * dTime * 10, 0));
    }
    // Updating the moveState
    if (m_moveState == moveState::SLIDINGSTANDING || m_moveState == moveState::SLIDINGWALKING)
    {
        m_moveState = moveState::SLIDINGSTANDING;
        if (GameEngine::instance()->is_key_down(m_Left))    m_moveState = moveState::SLIDINGWALKING;
        if (GameEngine::instance()->is_key_down(m_Right))    m_moveState = moveState::SLIDINGWALKING;
    }
}
void Avatar::NormalMoveState(double dTime)
{
    float2 oldVelocity = m_ActPtr->GetLinearVelocity();
    float2 desiredVelocity = float2();
    float2 dVelocity;

    if (GameEngine::instance()->is_key_down(m_Right))
    {
        desiredVelocity.x = m_Speed;
    }
	if (GameEngine::instance()->is_key_down(m_Left))
    {
        desiredVelocity.x = -m_Speed;
    }
    
    if (GameEngine::instance()->is_key_pressed(m_Attack) && m_NumberOfContactPointsWithLevel == 0)
    {
        m_ActPtr->SetLinearVelocity(float2(oldVelocity.x, 0));
        m_ActPtr->ApplyLinearImpulse(2 * m_JumpHeight*m_ActPtr->GetMass() / PhysicsActor::SCALE*float2(0, 1));
        m_moveState = moveState::ATTACK;
        m_SndAttackPtr->play();
    }
    float2 actVelocity = m_ActPtr->GetLinearVelocity();
	if (GameEngine::instance()->is_key_pressed(m_Jump) && m_JumpCounter < MAX_JUMPS && m_moveState != moveState::JUMPING)
    {
        //m_moveState = moveState::JUMPING;
        m_SndJumpPtr->play();
        desiredVelocity.y = -m_JumpHeight;
        m_ActPtr->SetFriction(0);
        m_NumberOfContactPointsWithLevel = 0;
        m_JumpCounter++;
    }
    dVelocity.x = desiredVelocity.x - oldVelocity.x;
    dVelocity.y = desiredVelocity.y;
    m_ActPtr->ApplyLinearImpulse(m_ActPtr->GetMass() * dVelocity);

    //Updating the moveState
    if (m_moveState == moveState::RUNNING || m_moveState == moveState::STANDING)
    {
        m_moveState = moveState::STANDING;
        if (GameEngine::instance()->is_key_down(m_Left))    m_moveState = moveState::RUNNING;
        if (GameEngine::instance()->is_key_down(m_Right))    m_moveState = moveState::RUNNING;
    }
    if (GameEngine::instance()->is_key_pressed(m_Attack) && m_JumpCounter > 0)m_moveState = moveState::ATTACK;
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
    GameEngine::instance()->print_string(String("Bronze: ") + String(m_AmountOfCoinsArr[0])
        + String(" Silver: ") + String(m_AmountOfCoinsArr[1])
        + String(" Gold: ") + String(m_AmountOfCoinsArr[2]));
}

//! Gets the avatar BoundingBox
PhysicsActor* Avatar::GetActor()
{
    return m_ActPtr;
}

//! Gets the avatarRespawnPosition
float2 Avatar::GetRespawnPosition()
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
void Avatar::SetSpawnPosition(float2 spawnPosition)
{
    m_RespawnPosition = spawnPosition;
    //GameEngine::instance()->print_string(String("spawn position set to ") + String(spawnPosition.ToString()));
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
    m_ActPtr->SetPosition(m_RespawnPosition + float2(0,-10));
    m_ActPtr->SetGhost(false);
}

//! Sets the jumpHeight of the avatar.
void Avatar::SetJumpHeight(double height)
{
    m_JumpHeight = height;
}

//! Directly sets the avatarPosition
void Avatar::SetPosition(float2 position)
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
void Avatar::SetKeyBinds(DataManager::KeyMap tmpKeyBindsArr)
{
    // Player actions
	m_Jump = tmpKeyBindsArr["jump"];
	m_Attack = tmpKeyBindsArr["attack"];
	m_Left = tmpKeyBindsArr["left"];
	m_Right = tmpKeyBindsArr["right"];
	m_God = tmpKeyBindsArr["god"];
}