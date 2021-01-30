#include "game.stdafx.h"		
	
#include "BlockSlide.h"
#include "Avatar.h"

BlockSlide::BlockSlide(float2 position, int width, int height) 
    : Entity(position)
    , m_Width(width)
    , m_Height(height)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(width, height, 0, 1);
    m_ActPtr->SetName(String("BlockSlide"));
    m_Position = position;
}

BlockSlide::~BlockSlide()
{
	
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void BlockSlide::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() && actThisPtr == m_ActPtr)
    {
        m_IsHit = true;
    }
}

void BlockSlide::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor() && actThisPtr == m_ActPtr)
    {
        m_IsHit = false;
    }
}

void BlockSlide::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void BlockSlide::Paint(graphics::D2DRenderContext& ctx)
{
    float3x3 matTranslate, matPivot;
    matTranslate= float3x3::translation(m_Position);
    matPivot= float3x3::translation(float2(-m_Width / 2, -m_Height / 2));
    ctx.set_world_matrix(matPivot * matTranslate);
    ctx.fill_rect(0, 0, m_Width, m_Height);
    ctx.set_color(COLOR(255,255,255));
    ctx.fill_rect(0, 0, m_Width, 5);
    ctx.set_color(COLOR(0, 0, 0));
    ctx.set_world_matrix(float3x3::identity());
}
void BlockSlide::Tick(double deltaTime)
{
    
}
void BlockSlide::Reset()
{

}


