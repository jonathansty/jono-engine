//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "EnemyHorizontal.h"
#include "Avatar.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
EnemyHorizontal::EnemyHorizontal(DOUBLE2 position,Bitmap* bmpEnemyPtr, Avatar* avatarPtr) :
Enemy(position),
m_StartPosition(position),
m_AvatarPtr(avatarPtr),
m_BmpPtr(bmpEnemyPtr)
{
    m_Velocity = DOUBLE2(50, 0);
    m_ActPtr = new PhysicsActor(position, 0, BodyType::KINEMATIC);
    m_ActPtr->AddBoxShape(bmpEnemyPtr->GetWidth(), bmpEnemyPtr->GetHeight(),0.5);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->SetFixedRotation(true);
    m_ActPtr->AddContactListener(this);
    m_ActPtr->SetLinearVelocity(m_Velocity);
    m_ActPtr->SetName(String("EnemyHorizontal"));
    

}

EnemyHorizontal::~EnemyHorizontal()
{
    if (m_ActPtr != nullptr)
    {

        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
    
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void EnemyHorizontal::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr->GetName() == String("Level"))
    {
        m_Velocity.x *= -1;
        m_ActPtr->SetLinearVelocity(m_Velocity);
    }
    if (actOtherPtr == m_AvatarPtr->GetActor() && m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK)
    {
        m_boolAttackContact = true;
        m_Lifes--;
        if (m_Lifes <= 1)
        {
            m_ActPtr->SetTrigger(true);
        }
        GAME_ENGINE->ConsolePrintString(m_Name + String(" has ") + String(m_Lifes) + String("left. "));
    }
}

void EnemyHorizontal::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() && m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK)
    {
        m_boolAttackContact = false;
        m_AvatarPtr->SetMoveState(Avatar::moveState::JUMPING);
        //GAME_ENGINE->ConsolePrintString(String(m_boolAttackContact));
    }
}

void EnemyHorizontal::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyHorizontal::Tick(double deltatime)
{
    if (m_ActPtr != nullptr)
    {
        m_Position = m_ActPtr->GetPosition();
        if (m_Position.x  < m_StartPosition.x - m_OffSet.x)
        {
            m_Velocity.x *= -1;
            m_ActPtr->SetLinearVelocity(m_Velocity);
        }
        if (m_Position.x  > m_StartPosition.x + m_OffSet.x)
        {
            m_Velocity.x *= -1;
            m_ActPtr->SetLinearVelocity(m_Velocity);
        }
        if (m_Position.y  < m_StartPosition.y - m_OffSet.y)
        {
            m_Velocity.y *= -1;
            m_ActPtr->SetLinearVelocity(m_Velocity);
        }
        if (m_Position.y  > m_StartPosition.y + m_OffSet.y)
        {
            m_Velocity.y *= -1;
            m_ActPtr->SetLinearVelocity(m_Velocity);
        }
    }
    
}
void EnemyHorizontal::Paint()
{
    MATRIX3X2 matTranslate, matCenter;
    matTranslate.SetAsTranslate(m_ActPtr->GetPosition());
    matCenter.SetAsTranslate(DOUBLE2(-m_BmpPtr->GetWidth() / 2, -m_BmpPtr->GetHeight() / 2));
    GAME_ENGINE->SetWorldMatrix(matCenter * matTranslate);
    GAME_ENGINE->DrawBitmap(m_BmpPtr);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
PhysicsActor* EnemyHorizontal::GetActor()
{
    return m_ActPtr;
}
void EnemyHorizontal::Reset()
{

}
void EnemyHorizontal::SetOffSet(DOUBLE2 offset)
{
    m_OffSet = offset;
}
void EnemyHorizontal::SetAvatar(Avatar* avatarPtr)
{
    m_AvatarPtr = avatarPtr;
}
bool EnemyHorizontal::GetAttackByAvatar()
{
    return m_boolAttackContact;
}
void EnemyHorizontal::SetVelocity(DOUBLE2 velocity)
{
    m_Velocity = velocity;
}
int EnemyHorizontal::GetLifes()
{
    return m_Lifes;
}
void EnemyHorizontal::SetLifes(int lifes)
{
    m_Lifes = lifes;
    if (m_Lifes == 1)
    {
        m_ActPtr->SetTrigger(true);
    }
}