#include "game.stdafx.h"		
	
#include "BitmapManager.h"
#include "ArrowShooter.h"
#include "Arrow.h"
#include "Avatar.h"

ArrowShooter::ArrowShooter(float2 position, float2 direction,double intervalTime)
    : Entity(position)
    , _direction(hlslpp::normalize(direction))
    , _interval_time(intervalTime)
{
    double angle = hlslpp::acos(hlslpp::dot(float2(1, 0),hlslpp::normalize(direction)));
    _angle = angle;
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
    _timer += deltaTime;

    if (_timer > _interval_time)
    {
        Arrow* tmpArrowPtr = new Arrow(m_Position + _direction*30,BitmapManager::instance()->load_image(String("Resources/Interactions/arrowUp.png")));
        tmpArrowPtr->GetActor()->SetLinearVelocity(_direction * SPEED);
        tmpArrowPtr->GetActor()->SetAngle(_angle);
        tmpArrowPtr->SetPushPower(_push_power);
        Add(tmpArrowPtr);
        _timer -= _interval_time;
    }

    for (int i = 0; i < _n_arrows; i++)
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
void ArrowShooter::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate, matPivot, matRotate;
    matTranslate= float3x3::translation(m_Position);
    matPivot= float3x3::translation(float2(-WIDTH/2,-HEIGHT/2));
    matRotate = float3x3::rotation_z(m_ActPtr->GetAngle());
    ctx.set_world_matrix(matPivot * matRotate * matTranslate);
    ctx.fill_rect(0, 0, WIDTH, HEIGHT);
    ctx.fill_rect(-10, HEIGHT, 0, HEIGHT + 10);
    ctx.fill_rect(WIDTH, HEIGHT, WIDTH + 10, HEIGHT + 10);
    ctx.set_world_matrix(float3x3::identity());
    for (int i = 0; i < _n_arrows; i++)
    {
        if (m_ArrowsPtrArr[i] != nullptr)
        {
            m_ArrowsPtrArr[i]->Paint(ctx);
        }
    }
}
void ArrowShooter::Reset()
{

}
void ArrowShooter::Add(Arrow* tmpArrowPtr)
{
    for (int i = 0; i < _n_arrows; i++)
    {
        if (m_ArrowsPtrArr[i] == nullptr)
        {
            m_ArrowsPtrArr[i] = tmpArrowPtr;
            return;
        }
    }
    m_ArrowsPtrArr.push_back(tmpArrowPtr);
    _n_arrows++;
}
void ArrowShooter::SetPushPower(int pushPower)
{
    _push_power = pushPower;
}
