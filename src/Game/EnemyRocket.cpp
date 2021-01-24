#include "stdafx.h"		
	
#include "EnemyRocket.h"
#include "Avatar.h"
#include "Trail.h"

EnemyRocket::EnemyRocket(float2 startPosition, float2 startVelocity):
Enemy(startPosition),
m_Direction(startVelocity)
{

    float2 normVelocity = hlslpp::normalize(startVelocity);
    double angle = hlslpp::acos(hlslpp::dot(float2(1, 0), normVelocity));
    m_AdjustedVelocity = startVelocity;
    m_Angle = angle;
    m_ActPtr = new PhysicsActor(startPosition, angle, BodyType::DYNAMIC);
    m_ActPtr->AddBoxShape(WIDTH, HEIGHT);
    m_ActPtr->AddContactListener(this);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->SetName(String("EnemyRocket"));
    b2Filter collisionFilter;
    collisionFilter.groupIndex = -2;
    m_ActPtr->SetCollisionFilter(collisionFilter);
   
    m_TrailPtr = new Trail(startPosition);
    m_TrailPtr->SetSize(HEIGHT - 10);
    m_TrailPtr->SetInterpolation(3);
    m_TrailPtr->SetOpacity(255);

}

EnemyRocket::~EnemyRocket()
{
    
    if (m_ActPtr != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
        delete m_ActPtr;
        m_ActPtr = nullptr;
    }

    delete m_TrailPtr;
    m_TrailPtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void EnemyRocket::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (m_AvatarPtr != nullptr && m_AvatarPtr->GetLifes() > 0 && actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHitByAvatar = true;
    }
}

void EnemyRocket::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void EnemyRocket::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void EnemyRocket::Tick(double deltaTime)
{
    if (m_IsHitByAvatar)
    {
        m_ActPtr->RemoveContactListener(this);
    }
    m_Position = m_ActPtr->GetPosition();
    float2 vectorToAvatar = m_AvatarPtr->GetPosition() - m_ActPtr->GetPosition();
	vectorToAvatar = hlslpp::normalize(vectorToAvatar);

    float2 unitVector(1,0);
    float dotProduct = hlslpp::dot(hlslpp::normalize(m_AdjustedVelocity),vectorToAvatar);
	float targetAngle = hlslpp::acos(hlslpp::float1(dotProduct));
    /*GameEngine::instance()->ConsolePrintString(String("Dot: ") + String(dotProduct));
    GameEngine::instance()->ConsolePrintString(String("Angle: ") + String(targetAngle));*/
    //m_ActPtr->SetAngularVelocity(deltaTime);
    m_AdjustedVelocity.x = cos(m_Angle);
    m_AdjustedVelocity.y = sin(m_Angle);
    
    if (m_AvatarPtr->GetMoveState() != Avatar::moveState::DYING)
    {
        if (dotProduct > 0)
        {
            if (targetAngle > 0.1)
            {
                m_Angle += m_AngularVelocity*deltaTime;
            }
            else if (targetAngle < -0.1)
            {

                m_Angle -= m_AngularVelocity*deltaTime;
            }
        }
        if (dotProduct < 0)
        {
            if (targetAngle > 0.1)
            {

                m_Angle += m_AngularVelocity*deltaTime;
            }
            else if (targetAngle < -0.1)
            {
                m_Angle -= m_AngularVelocity*deltaTime;
            }
        }
    }
    
    m_ActPtr->SetAngle(m_Angle);
    m_ActPtr->SetLinearVelocity(m_Speed*m_AdjustedVelocity);
}
void EnemyRocket::Paint(graphics::D2DRenderContext& ctx)
{

    m_TrailPtr->Paint(ctx,m_ActPtr->GetPosition());
    float3x3 matTranslate, matRotate, matPivot, matWorldTransform;
    matTranslate= float3x3::translation(m_ActPtr->GetPosition());
    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle());
    matPivot= float3x3::translation(float2(- WIDTH /2,-HEIGHT/2));
    matWorldTransform = matPivot * matRotate * matTranslate;
    ctx.set_world_matrix(matWorldTransform);
    std::vector<float2>trianglePointsArr;
    trianglePointsArr.push_back(float2(0,0));
    trianglePointsArr.push_back(float2(WIDTH,HEIGHT/2));
    trianglePointsArr.push_back(float2(0,HEIGHT));
    ctx.fill_polygon(trianglePointsArr, 3);
    ctx.set_world_matrix(float3x3::identity());
    
    /*GameEngine::instance()->DrawLine(m_Position.x, m_Position.y, m_AvatarPtr->GetPosition().x, m_AvatarPtr->GetPosition().y);
    GameEngine::instance()->DrawLine(m_Position.x, m_Position.y, m_Position.x + 10*m_AdjustedVelocity.x, m_Position.y + 10*m_AdjustedVelocity.y);*/

}
PhysicsActor* EnemyRocket::GetActor()
{
    return m_ActPtr;
}
void EnemyRocket::Reset()
{
    //m_ActPtr->RemoveContactListener(this);
}
void EnemyRocket::SetSpeed(double speed)
{
    m_Speed = speed;
}
bool EnemyRocket::GetAttackByAvatar()
{
    return m_IsHitByAvatar;
}

