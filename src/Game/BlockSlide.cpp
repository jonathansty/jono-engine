#include "stdafx.h"		
	
#include "BlockSlide.h"
#include "Avatar.h"

BlockSlide::BlockSlide(DOUBLE2 position, int width, int height) 
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
void BlockSlide::Paint()
{
    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-m_Width / 2, -m_Height / 2));
    GameEngine::instance()->set_world_matrix(matPivot * matTranslate);
    GameEngine::instance()->fill_rect(0, 0, m_Width, m_Height);
    GameEngine::instance()->set_color(COLOR(255,255,255));
    GameEngine::instance()->fill_rect(0, 0, m_Width, 5);
    GameEngine::instance()->set_color(COLOR(0, 0, 0));
    GameEngine::instance()->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void BlockSlide::Tick(double deltaTime)
{
    
}
void BlockSlide::Reset()
{

}


