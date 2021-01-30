//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"		
	
#include "EnemyLaser.h"
#include "Level.h"
#include "Avatar.h"

EnemyLaser::EnemyLaser(float2 position):
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
    float2 intersectionLeft, normal, intersectionRight;
    double fraction;
    bool isLevelHitLeft = false;
    bool isLevelHitRight = false;
    if (m_LevelPtr != nullptr)
    {
        float2 rayLength = float2(RAYLENGTH * cos(m_ActPtr->GetAngle()), RAYLENGTH * sin(m_ActPtr->GetAngle()));
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
        float2 rayStart = m_Position;
        float2 rayEnd = m_IntersectionLeft;

        float2 intersection, normal;
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
void EnemyLaser::Paint(graphics::D2DRenderContext& ctx)
{
    //ctx.DrawLine(m_RayStart, m_RayEnd);
    ctx.draw_line(m_IntersectionLeft, m_RayStart);
    ctx.draw_line(m_IntersectionRight, m_RayStart);


    double leftLength = hlslpp::length(m_IntersectionLeft - m_Position);
	double rightLength = hlslpp::length(m_IntersectionRight - m_Position);
    float3x3 matTranslate, matRotate;

    matTranslate= float3x3::translation(m_Position);
    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle());
    ctx.set_world_matrix(matRotate * matTranslate);
    ctx.set_color(COLOR(125, 125, 125));
    ctx.fill_ellipse(float2(), 20, 20);
    ctx.set_color(COLOR(255, 255, 255));
    ctx.fill_rect((int)-leftLength, -5, 0, 5);
    ctx.fill_rect(0, -5, (int)rightLength, 5);
    ctx.set_color(COLOR(0, 0, 0));
    ctx.set_world_matrix(float3x3::identity());
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