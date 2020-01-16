//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "RotLight.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
RotLight::RotLight(DOUBLE2 position):
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
void RotLight::Paint()
{

    std::vector<DOUBLE2>tmpPointsArr;
    //GAME_ENGINE->SetWorldMatrix(matWorldTransform);
    MATRIX3X2 matOrbitRadius, matOrbitCenter;
    matOrbitRadius.SetAsTranslate(DOUBLE2(20, 0));
    matOrbitCenter.SetAsTranslate(DOUBLE2(m_Position));
    GAME_ENGINE->SetColor(m_Color);
    for (double angle = 0; angle < 2 * M_PI; angle += (2*M_PI)/6)
    {
        MATRIX3X2 matRectOrbitRotate, matWorldTransform;
        matRectOrbitRotate.SetAsRotate(angle + m_Angle);
        matWorldTransform = matOrbitRadius * matRectOrbitRotate * matOrbitCenter;
        GAME_ENGINE->SetWorldMatrix(matWorldTransform);
        //GAME_ENGINE->DrawRect(0, -10, 100, 10);
        std::vector<DOUBLE2>tmpPointsArr;
        tmpPointsArr.push_back(DOUBLE2(0,-10));
        tmpPointsArr.push_back(DOUBLE2(m_Radius,-m_Radius/4));
        tmpPointsArr.push_back(DOUBLE2(m_Radius + 10, -m_Radius / 6));
        tmpPointsArr.push_back(DOUBLE2(m_Radius + 15, 0));
        tmpPointsArr.push_back(DOUBLE2(m_Radius + 10, m_Radius / 6));
        tmpPointsArr.push_back(DOUBLE2(m_Radius,m_Radius/4));
        tmpPointsArr.push_back(DOUBLE2(0,10));
        GAME_ENGINE->FillPolygon(tmpPointsArr, 7);
    }

   
    GAME_ENGINE->SetColor(COLOR(0, 0, 0));
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
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
void RotLight::SetPosition(DOUBLE2 position)
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
DOUBLE2 RotLight::GetPosition()
{
    return m_Position;
}