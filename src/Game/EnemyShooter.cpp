#include "stdafx.h"		
	
#include "EnemyShooter.h"
#include "Bullet.h"
#include "Level.h"
#include "Avatar.h"
#include "Animation.h"
#include "AnimationList.h"
#include "EntityDestroy.h"

EnemyShooter::EnemyShooter(float2 position, Bitmap* bmpPtr, double angle) :
Enemy(position),
m_BmpEnemyBodyPtr(bmpPtr),
m_Angle(angle)
{
    m_AnimationListPtr = new AnimationList();
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);

    m_ActPtr = new PhysicsActor(position, angle, BodyType::STATIC);
    m_ActPtr->AddBoxShape(CLIPWIDTH - 10, CLIPHEIGHT ,0);
    m_ActPtr->SetName(String("EnemyShooter"));
    m_BmpEnemyBodyPtr = bmpPtr;

}

EnemyShooter::~EnemyShooter()
{
    //m_ActPtr->RemoveContactListener(this);

    delete m_AnimationListPtr;
    m_AnimationListPtr = nullptr;

    if (m_ActBulletPtr != nullptr)
    {
        m_ActBulletPtr->RemoveContactListener(this);
        delete m_ActBulletPtr;
        m_ActBulletPtr = nullptr;
    }
    if (m_ActPtr != nullptr)
    {

        delete m_ActPtr;
        m_ActPtr = nullptr;
    }
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void EnemyShooter::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if ((actOtherPtr == m_AvatarPtr->GetActor() || actOtherPtr == m_LevelPtr->GetActor()) && actThisPtr == m_ActBulletPtr)
    {
        m_IsHit = true;
    }
    
}

void EnemyShooter::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    
}

void EnemyShooter::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyShooter::Paint(graphics::D2DRenderContext& ctx)
{
    switch (m_State)
    {
    case EnemyShooter::state::SHOOTING:
        m_BoundingBox.left = m_FrameNr * 60;
        m_BoundingBox.top = 0;
        break;
    case EnemyShooter::state::IDLE:
        m_BoundingBox.left = 0;
        m_BoundingBox.top = 0;
        break;
    default:
        break;
    }
    m_BoundingBox.right = m_BoundingBox.left + CLIPWIDTH;
    m_BoundingBox.top = m_BoundingBox.top + CLIPHEIGHT;
    float3x3 matTransform,matPivot, matRotate, matWorldTransform,matScale;
    matTransform = float3x3::translation(m_Position);
    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle() + M_PI/2);
    matPivot = float3x3::translation(float2(-60 / 2, 62 / 2));
    if (m_Mirror)
    {
        matScale = float3x3::scale(1, -1);
    }
    else
    {
        matScale = float3x3::scale(1, 1);
    }

    matWorldTransform = matPivot* matScale * matRotate * matTransform;
    m_AnimationListPtr->Paint(ctx);
    
    ctx.set_world_matrix(matWorldTransform);
    ctx.draw_bitmap(m_BmpEnemyBodyPtr,m_BoundingBox);
    ctx.set_world_matrix(float3x3::identity());

    //Painting the bullet
    if (m_ActBulletPtr != nullptr)
    {
        matTransform = float3x3::translation(m_ActBulletPtr->GetPosition());
        matRotate = float3x3::rotation_z(m_ActBulletPtr->GetAngle());
        matPivot = float3x3::translation(float2(-5, -10));
        ctx.set_world_matrix(matPivot * matRotate * matTransform);
        ctx.draw_rect(0, 0, 10, 20);
        ctx.set_world_matrix(float3x3::identity());
    }
    
}
void EnemyShooter::Tick(double deltaTime)
{
    m_AccuTime += deltaTime;
    if (m_AccuTime > 1.0/ FRAMERATE)
    {

        m_FrameNr++;
        m_FrameNr = m_FrameNr%MAXFRAMES;
        if (m_FrameNr == MAXFRAMES - 1)
        {
            m_State = state::IDLE;
        }
        
        m_AccuTime -= 1.0 / FRAMERATE;
    }
    

    if (m_IsHit)
    {
        float2 position = m_ActBulletPtr->GetPosition();
        m_ActBulletPtr->RemoveContactListener(this);
        delete m_ActBulletPtr;
        m_ActBulletPtr = nullptr;
        EntityDestroy* tmpEntityDestroyAnimation = new EntityDestroy(position);
        tmpEntityDestroyAnimation->SetRadius(40);
        m_AnimationListPtr->Add(tmpEntityDestroyAnimation);
        m_IsHit = false;
    }
    
    float2 position = m_ActPtr->GetPosition();
    DOUBLE angle = m_ActPtr->GetAngle();
    int speed = 120 * 100 ;
    float2 velocity = float2(speed * cos(angle + M_PI_2), speed * sin(angle + M_PI_2));
    velocity *= deltaTime;

    m_AnimationListPtr->Tick(deltaTime);
    if (m_ActBulletPtr == nullptr)
    {
        m_State = state::SHOOTING;
        // Create object and shoot in a direction
        
        
        m_ActBulletPtr = new PhysicsActor(position,0,BodyType::DYNAMIC);
        m_ActBulletPtr->SetTrigger(true);
        m_ActBulletPtr->AddContactListener(this);
        m_ActBulletPtr->AddBoxShape(10, 20);
        m_ActBulletPtr->SetAngle(angle);
        m_ActBulletPtr->SetGravityScale(0);
        m_ActBulletPtr->SetName(String("EnemyBullet"));
        m_ActBulletPtr->SetLinearVelocity(velocity);

    }
    else
    {
        if (m_ActBulletPtr!= nullptr)
        {
            m_ActBulletPtr->SetLinearVelocity(velocity);
        }
        
    }
    
}
PhysicsActor* EnemyShooter::GetActor()
{

    return m_ActBulletPtr;
}
float2 EnemyShooter::GetPosition()
{
    return m_Position;
}
void EnemyShooter::SetSpawnPosition(float2 position)
{
    m_Position = position;
}
void EnemyShooter::Reset()
{
    if (m_ActBulletPtr != nullptr && m_ActBulletPtr->GetContactListener() != nullptr)
    {
        m_ActBulletPtr->RemoveContactListener(this);
        delete m_ActBulletPtr;
        m_ActBulletPtr = nullptr;
    }
}
void EnemyShooter::SetMirror(bool mirror)
{
    m_Mirror = mirror;
}

void EnemyShooter::RemoveContactListener()
{
    if (m_ActBulletPtr != nullptr)
    {
        m_ActBulletPtr->RemoveContactListener(this);
    }
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
    }
    
}
