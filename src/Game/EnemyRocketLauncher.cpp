//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "EnemyRocketLauncher.h"
#include "Avatar.h"
#include "EnemyRocket.h"
#include "EnemyList.h"
#include "EntityDestroy.h"
#include "Animation.h"
#include "AnimationList.h"
#include "BitmapManager.h"


EnemyRocketLauncher::EnemyRocketLauncher(DOUBLE2 position, double angle):
Enemy(position)
{
    m_EnemyListPtr = new EnemyList();
    m_AnimationListPtr = new AnimationList();
    m_ActPtr = new PhysicsActor(position, angle, BodyType::STATIC);
    m_Angle = angle;
    m_Direction = DOUBLE2(cos(angle - M_PI_2), sin(angle - M_PI_2));
    m_ActPtr->AddBoxShape(WIDTH, HEIGHT,0,0);
    m_ActPtr->SetName(String("EnemyRocketLauncher"));
    m_BmpPtr = BitmapManager::instance()->load_image(String("Resources/Enemy/RocketLauncher.png"));
}

EnemyRocketLauncher::~EnemyRocketLauncher()
{
    m_EnemyListPtr->RemoveAll();
    delete m_EnemyListPtr;
    m_EnemyListPtr = nullptr;

    delete m_AnimationListPtr;
    m_AnimationListPtr = nullptr;


    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void EnemyRocketLauncher::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyRocketLauncher::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyRocketLauncher::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyRocketLauncher::Tick(double deltaTime)
{
    if (m_IsReset)
    {
        m_EnemyListPtr->RemoveAll();
        m_IsReset = false;
    }
    m_AccuTime += deltaTime;
    if (m_AccuTime > 1.0 / FRAMERATE)
    {
        m_FrameNr++;
        m_FrameNr = m_FrameNr % MAXFRAMES;
        m_AccuTime -= 1.0 / FRAMERATE;
    }
    int distance = (int)((m_Position - m_AvatarPtr->GetPosition()).Length());

    if (distance < DETECTIONZONE)
    {
        if (m_AccuShootTime > m_IntervalTime)
        {
            m_ActionState = actionState::SHOOTING;
        }
        m_AccuShootTime += deltaTime;
        
    }
    if (distance > DETECTIONZONE)
    {
        m_AccuShootTime = 0;
    }

    if (m_ActionState == actionState::SHOOTING && m_FrameNr > 8 && m_AccuShootTime > m_IntervalTime && m_AmountOfRockets < MAXROCKETS && distance < DETECTIONZONE)
    {
        m_AccuShootTime -= m_IntervalTime;
        m_AmountOfRockets++;
        //m_Direction = DOUBLE2(1, 0);
        EnemyRocket* tmpRocket = new EnemyRocket(m_Position + m_Direction*30, m_Direction);
        tmpRocket->SetAvatar(m_AvatarPtr);
        tmpRocket->SetSpeed(rand() % 100 + 300);
        tmpRocket->setName(String("Rocket"));
        m_EnemyListPtr->Add(tmpRocket);
        m_ActionState = actionState::WAITING;
    }
    m_EnemyListPtr->Tick(deltaTime);
    m_AnimationListPtr->Tick(deltaTime);
    m_AmountOfRockets = m_EnemyListPtr->GetSize();
    
    
}
void EnemyRocketLauncher::Paint()
{
    MATRIX3X2 matTranslate, matPivot, matWorldTransform, matRotate;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-WIDTH / 2, -HEIGHT / 2));
    matRotate.SetAsRotate(m_Angle);
    matWorldTransform = matPivot * matRotate * matTranslate;
    m_AnimationListPtr->Paint();

    GameEngine::instance()->set_world_matrix(matWorldTransform);
    RECT boundingBox = updateFrameDisplay(m_FrameNr);
    GameEngine::instance()->DrawBitmap(m_BmpPtr, boundingBox);
    GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void EnemyRocketLauncher::PaintRockets()
{
    m_EnemyListPtr->Paint();
}
RECT EnemyRocketLauncher::updateFrameDisplay(int frameNr)
{
    RECT boundingBox;
    switch (m_ActionState)
    {
    case EnemyRocketLauncher::actionState::SHOOTING:
        boundingBox.left = frameNr * WIDTH;
        boundingBox.top = 0;
        break;
    case EnemyRocketLauncher::actionState::WAITING:
        boundingBox.left = 0;
        boundingBox.top = 0;
        break;
    case EnemyRocketLauncher::actionState::PAUSE:
        boundingBox.left = 0;
        boundingBox.top = 0;
        break;
    default:
        break;
    }
    boundingBox.right = boundingBox.left + WIDTH;
    boundingBox.bottom = boundingBox.top + HEIGHT;
    return boundingBox;
}
void EnemyRocketLauncher::PaintDebug()
{
    Enemy::PaintDebug();
    m_EnemyListPtr->PaintDebug();

    
    GameEngine::instance()->DrawEllipse((int)m_Position.x, (int)m_Position.y, DETECTIONZONE, DETECTIONZONE);

}
void EnemyRocketLauncher::Reset()
{
    m_IsReset = true;
}
PhysicsActor* EnemyRocketLauncher::GetActor()
{
    return m_ActPtr;
}
void EnemyRocketLauncher::SetAvatar(Avatar* avatarPtr)
{
    m_AvatarPtr = avatarPtr;
    m_EnemyListPtr->SetAvatar(avatarPtr);
}

bool EnemyRocketLauncher::GetAttackByAvatar()
{
    Enemy* tmpRocket = m_EnemyListPtr->IsAttackedByAvatar();
    if (tmpRocket != nullptr)
    {
        m_AnimationListPtr->Add(new EntityDestroy(tmpRocket->GetPosition()));
        tmpRocket->RemoveContactListener();
        m_EnemyListPtr->Remove(tmpRocket);
        
        return true;
    }
    return false;
}

void EnemyRocketLauncher::RemoveContactListener()
{
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
    }
    if (m_EnemyListPtr != nullptr)
    {
        m_EnemyListPtr->RemoveAll();
    }
}