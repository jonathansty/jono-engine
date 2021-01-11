#include "stdafx.h"		
	
#include "BitmapManager.h"
#include "ArrowShooter.h"
#include "Arrow.h"
#include "Avatar.h"

ArrowShooter::ArrowShooter(DOUBLE2 position, DOUBLE2 direction,double intervalTime)
    : Entity(position)
    , m_direction(direction.Normalized())
    , m_IntervalTime(intervalTime)
{
    double angle = DOUBLE2(1, 0).AngleWith(direction);
    m_Angle = angle;
    m_ActPtr = new PhysicsActor(position,angle - M_PI_2,BodyType::STATIC);
    m_ActPtr->AddBoxShape(WIDTH, HEIGHT);
}

ArrowShooter::~ArrowShooter()
{
    for (size_t i = 0; i < m_ArrowsPtrArr.size(); i++)
    {
        if (m_ArrowsPtrArr[i] != nullptr)
        {
            delete m_ArrowsPtrArr[i];
        }
    }
    m_ArrowsPtrArr.clear();
    delete m_ActPtr;
    m_ActPtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void ArrowShooter::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void ArrowShooter::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void ArrowShooter::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void ArrowShooter::Tick(double deltaTime)
{
    m_AccuTime += deltaTime;

    if (m_AccuTime > m_IntervalTime)
    {
        Arrow* tmpArrowPtr = new Arrow(m_Position + m_direction*30,BitmapManager::instance()->load_image(String("Resources/Interactions/arrowUp.png")));
        tmpArrowPtr->GetActor()->SetLinearVelocity(m_direction * SPEED);
        tmpArrowPtr->GetActor()->SetAngle(m_Angle);
        tmpArrowPtr->SetPushPower(m_PushPower);
        Add(tmpArrowPtr);
        m_AccuTime -= m_IntervalTime;
    }

    for (int i = 0; i < m_AmountOfArrows; i++)
    {
        if (m_ArrowsPtrArr[i] != nullptr)
        {
            m_ArrowsPtrArr[i]->Tick(deltaTime);
            if (m_ArrowsPtrArr[i]->isHit())
            {
                delete m_ArrowsPtrArr[i];
                m_ArrowsPtrArr[i] = nullptr;
            }
        }
    }
}
void ArrowShooter::Paint()
{
    MATRIX3X2 matTranslate, matPivot, matRotate;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-WIDTH/2,-HEIGHT/2));
    matRotate.SetAsRotate(m_ActPtr->GetAngle());
    GameEngine::instance()->set_world_matrix(matPivot * matRotate * matTranslate);
    GameEngine::instance()->FillRect(0, 0, WIDTH, HEIGHT);
    GameEngine::instance()->FillRect(-10, HEIGHT, 0, HEIGHT + 10);
    GameEngine::instance()->FillRect(WIDTH, HEIGHT, WIDTH + 10, HEIGHT + 10);
    GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
    for (int i = 0; i < m_AmountOfArrows; i++)
    {
        if (m_ArrowsPtrArr[i] != nullptr)
        {
            m_ArrowsPtrArr[i]->Paint();
        }
    }
}
void ArrowShooter::Reset()
{

}
void ArrowShooter::Add(Arrow* tmpArrowPtr)
{
    for (int i = 0; i < m_AmountOfArrows; i++)
    {
        if (m_ArrowsPtrArr[i] == nullptr)
        {
            m_ArrowsPtrArr[i] = tmpArrowPtr;
            return;
        }
    }
    m_ArrowsPtrArr.push_back(tmpArrowPtr);
    m_AmountOfArrows++;
}
void ArrowShooter::SetPushPower(int pushPower)
{
    m_PushPower = pushPower;
}
