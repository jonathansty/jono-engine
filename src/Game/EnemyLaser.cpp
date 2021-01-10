//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "EnemyLaser.h"
#include "Level.h"
#include "Avatar.h"

EnemyLaser::EnemyLaser(DOUBLE2 position):
Enemy(position)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::KINEMATIC);
    m_ActPtr->SetName(String("EnemyLaser"));
    m_ActPtr->SetAngularVelocity(m_AngularVelocity);
}

EnemyLaser::~EnemyLaser()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void EnemyLaser::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyLaser::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyLaser::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyLaser::Tick(double deltaTime)
{
    
    // Ray casting to check where the laser is hitting the level.
    DOUBLE2 intersectionLeft, normal, intersectionRight;
    double fraction;
    bool isLevelHitLeft = false;
    bool isLevelHitRight = false;
    if (m_LevelPtr != nullptr)
    {
        DOUBLE2 rayLength = DOUBLE2(RAYLENGTH * cos(m_ActPtr->GetAngle()), RAYLENGTH * sin(m_ActPtr->GetAngle()));
        m_RayStart = m_Position;
        m_RayEnd = m_RayStart - rayLength;
        isLevelHitLeft = m_LevelPtr->GetActor()->Raycast(m_RayStart, m_RayEnd, intersectionLeft, normal, fraction);

        m_RayStart = m_Position;
        m_RayEndRight = m_RayStart + rayLength;
        isLevelHitRight = m_LevelPtr->GetActor()->Raycast(m_RayStart, m_RayEndRight, intersectionRight, normal, fraction);
    }
    if (isLevelHitLeft)
    {
        m_IntersectionLeft = intersectionLeft;
    }
    else
    {
        m_IntersectionLeft = m_RayEnd;
    }
    if (isLevelHitRight)
    {
        m_IntersectionRight = intersectionRight;
    }
    else
    {
        m_IntersectionRight = m_RayEndRight;
    }
    bool isAvatarHitLeft = false;
    bool isAvatarHitRight = false;
    if (m_AvatarPtr != nullptr)
    {
       
        m_RayStart = m_Position;
        DOUBLE2 rayStart = m_Position;
        DOUBLE2 rayEnd = m_IntersectionLeft;

        DOUBLE2 intersection, normal;
        double fraction;
        isAvatarHitLeft = m_AvatarPtr->GetActor()->Raycast(rayStart,rayEnd,intersection,normal,fraction);

        rayEnd = m_IntersectionRight;
        isAvatarHitRight = m_AvatarPtr->GetActor()->Raycast(rayStart, rayEnd, intersection, normal, fraction);
    }
    
    if (isAvatarHitLeft || isAvatarHitRight)
    {
        if (m_AvatarPtr->GetMoveState() != Avatar::moveState::GOD)
        {
            m_AvatarPtr->SetGameOver();
        }
       
    }
}
void EnemyLaser::Paint()
{
    //game_engine::instance()->DrawLine(m_RayStart, m_RayEnd);
    game_engine::instance()->DrawLine(m_IntersectionLeft, m_RayStart);
    game_engine::instance()->DrawLine(m_IntersectionRight, m_RayStart);


    double leftLength = (m_IntersectionLeft - m_Position).Length();
    double rightLength = (m_IntersectionRight - m_Position).Length();
    MATRIX3X2 matTranslate, matRotate;

    matTranslate.SetAsTranslate(m_Position);
    matRotate.SetAsRotate(m_ActPtr->GetAngle());
    game_engine::instance()->set_world_matrix(matRotate * matTranslate);
    game_engine::instance()->set_color(COLOR(125, 125, 125));
    game_engine::instance()->FillEllipse(DOUBLE2(), 20, 20);
    game_engine::instance()->set_color(COLOR(255, 255, 255));
    game_engine::instance()->FillRect((int)-leftLength, -5, 0, 5);
    game_engine::instance()->FillRect(0, -5, (int)rightLength, 5);
    game_engine::instance()->set_color(COLOR(0, 0, 0));
    game_engine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
PhysicsActor* EnemyLaser::GetActor()
{
    return m_ActPtr;
}
void EnemyLaser::Reset()
{

}
void EnemyLaser::SetAngularVelocity(double angularVelocity)
{
    m_ActPtr->SetAngularVelocity(angularVelocity);
    m_AngularVelocity = angularVelocity;
}