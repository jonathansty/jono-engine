#include "stdafx.h"		
#include "Coin.h"

Coin::Coin(float2 position, Bitmap* bmpPtr)
    : PickUp(position)
    , m_BmpCoinPtr(bmpPtr)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_ActPtr = new PhysicsActor(position, 0, BodyType::DYNAMIC);
    m_ActPtr->AddContactListener(this);
    m_ActPtr->AddCircleShape(bmpPtr->GetWidth()/2);
    m_ActPtr->SetTrigger(true);
    m_ActPtr->SetGravityScale(0);
    m_ActPtr->SetName(String("Coin"));

    b2Filter filter;
    filter.groupIndex = -1;
    m_ActPtr->SetCollisionFilter(filter);
}

Coin::~Coin()
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
void Coin::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr->GetName() == String("Avatar") && actOtherPtr->GetName() != String("Coin"))
    {
        m_IsHit = true;
        if (m_SndHitPtr != nullptr)
        {
            GameEngine::instance()->print_string(String("Coin sound Played!"));
            if (m_SndHitPtr->get_volume() > 0)
            {
                m_SndHitPtr->play(sound::play_mode::Immediate);
            }
            
        }
        
    }
}

void Coin::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr->GetName() != String("Coin") && actOtherPtr->GetName() == String("Avatar"))
    {
        m_IsHit = false;
    }
}

void Coin::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Coin::Tick(double deltaTime)
{

}
void Coin::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate,matPivot;
    matPivot = float3x3::translation(-m_BmpCoinPtr->GetWidth() / 2, -m_BmpCoinPtr->GetHeight() / 2);
    matTranslate = float3x3::translation(m_Position);
    ctx.set_world_matrix(matPivot* matTranslate);
    ctx.draw_bitmap(m_BmpCoinPtr);
    ctx.set_world_matrix(float3x3::identity());
}
bool Coin::IsHit()
{
    return m_IsHit;
}
void Coin::SetCoinValue(int value)
{
    m_CoinValue = value;
}
int Coin::GetCoinValue()
{
    return m_CoinValue;
}
void Coin::SetIndex(int number)
{
    m_Index = number;
}
void Coin::SetName(String name)
{
    PickUp::SetName(name);
}
String Coin::GetName()
{
    return PickUp::GetName();
}
float2 Coin::GetPosition()
{
    return m_ActPtr->GetPosition();
}

void Coin::RemoveContactListener()
{
    if (m_ActPtr != nullptr && m_ActPtr->GetContactListener() != nullptr)
    {
        m_ActPtr->RemoveContactListener(this);
    }
}