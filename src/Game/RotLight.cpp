//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "RotLight.h"


RotLight::RotLight(float2 position):
Animation(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

RotLight::~RotLight()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void RotLight::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void RotLight::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void RotLight::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void RotLight::Paint(graphics::D2DRenderContext& ctx)
{

    float3x3 matOrbitRadius, matOrbitCenter;
    matOrbitRadius= float3x3::translation(float2(20, 0));
    matOrbitCenter= float3x3::translation(float2(m_Position));
    ctx.set_color(m_Color);
    for (double angle = 0; angle < 2 * M_PI; angle += (2*M_PI)/6)
    {
        float3x3 matRectOrbitRotate, matWorldTransform;
        //matRectOrbitRotate = float3x3::rotation_z(angle + m_Angle);
        matWorldTransform = hlslpp::mul(hlslpp::mul(matOrbitRadius , matRectOrbitRotate) , matOrbitCenter);
        ctx.set_world_matrix(matWorldTransform);
		std::vector<float2> tmpPointsArr{};
        tmpPointsArr.push_back(float2(0,-10));
        tmpPointsArr.push_back(float2(m_Radius,-m_Radius/4));
        tmpPointsArr.push_back(float2(m_Radius + 10, -m_Radius / 6));
        tmpPointsArr.push_back(float2(m_Radius + 15, 0));
        tmpPointsArr.push_back(float2(m_Radius + 10, m_Radius / 6));
        tmpPointsArr.push_back(float2(m_Radius,m_Radius/4));
        tmpPointsArr.push_back(float2(0,10));
        ctx.fill_polygon(tmpPointsArr, 7);
    }

   
    ctx.set_color(COLOR(0, 0, 0));
    ctx.set_world_matrix(float3x3::identity());
}
void RotLight::Tick(double deltaTime)
{
    m_Angle += m_RotSpeed * deltaTime;
    if (m_Radius > m_MaxRadius)
    {
        m_DrawState = drawState::MAXRADIUS;
    }
    if (m_DrawState == drawState::SPAWNING)
    {
        m_Radius += m_SpawnSpeed*deltaTime;
    }
    
}
void RotLight::SetColor(COLOR color)
{
    m_Color = color;
}
void RotLight::SetRotSpeed(int speed)
{
    m_RotSpeed = speed;
}
void RotLight::SetRadius(double radius)
{
    m_MaxRadius = radius;
}
void RotLight::SetPosition(float2 position)
{
    m_Position= position;
}
void RotLight::SetDrawState(drawState drawstate)
{
    if (m_DrawState == drawState::MAXRADIUS)
    {
        if (drawstate == drawState::SPAWNING)
        {
            m_Radius = MINRADIUS;
        }
    }
    m_DrawState = drawstate;
}
float2 RotLight::GetPosition()
{
    return m_Position;
}