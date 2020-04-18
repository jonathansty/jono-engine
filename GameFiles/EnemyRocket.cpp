#include "stdafx.h"		
	
#include "EnemyRocket.h"
#include "Avatar.h"
#include "Trail.h"

EnemyRocket::EnemyRocket(DOUBLE2 startPosition, DOUBLE2 startVelocity):
Enemy(startPosition),
m_Direction(startVelocity)
{

    DOUBLE2 normVelocity = startVelocity.Normalized();
    double angle = DOUBLE2(1, 0).AngleWith(normVelocity);
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
    DOUBLE2 vectorToAvatar = m_AvatarPtr->GetPosition() - m_ActPtr->GetPosition();
    vectorToAvatar = vectorToAvatar.Normalized();

    DOUBLE2 unitVector(1,0);
    double targetAngle = m_AdjustedVelocity.Normalized().AngleWith(vectorToAvatar);
    double dotProduct = m_AdjustedVelocity.Normalized().DotProduct(vectorToAvatar);
    /*game_engine::instance()->ConsolePrintString(String("Dot: ") + String(dotProduct));
    game_engine::instance()->ConsolePrintString(String("Angle: ") + String(targetAngle));*/
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
void EnemyRocket::Paint()
{

    m_TrailPtr->Paint(m_ActPtr->GetPosition());
    MATRIX3X2 matTranslate, matRotate, matPivot, matWorldTransform;
    matTranslate.SetAsTranslate(m_ActPtr->GetPosition());
    matRotate.SetAsRotate(m_ActPtr->GetAngle());
    matPivot.SetAsTranslate(DOUBLE2(- WIDTH /2,-HEIGHT/2));
    matWorldTransform = matPivot * matRotate * matTranslate;
    game_engine::instance()->SetWorldMatrix(matWorldTransform);
    std::vector<DOUBLE2>trianglePointsArr;
    trianglePointsArr.push_back(DOUBLE2(0,0));
    trianglePointsArr.push_back(DOUBLE2(WIDTH,HEIGHT/2));
    trianglePointsArr.push_back(DOUBLE2(0,HEIGHT));
    game_engine::instance()->FillPolygon(trianglePointsArr, 3);
    game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
    
    /*game_engine::instance()->DrawLine(m_Position.x, m_Position.y, m_AvatarPtr->GetPosition().x, m_AvatarPtr->GetPosition().y);
    game_engine::instance()->DrawLine(m_Position.x, m_Position.y, m_Position.x + 10*m_AdjustedVelocity.x, m_Position.y + 10*m_AdjustedVelocity.y);*/

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

