#include "stdafx.h"		
	
#include "Lever.h"
#include "Avatar.h"

Lever::Lever(DOUBLE2 position, Bitmap* bmpPtr):
Entity(position),
m_BmpPtr(bmpPtr)
{
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    m_ActPtr->AddBoxShape(bmpPtr->GetWidth()/AMOUNTOFFRAMES, bmpPtr->GetHeight());
    m_ActPtr->SetTrigger(true);
    m_ActPtr->AddContactListener(this);
    m_ClipWidth = bmpPtr->GetWidth()/AMOUNTOFFRAMES;
}

Lever::~Lever()
{
	// nothing to destroy
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void Lever::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{
    if (actOtherPtr == m_AvatarPtr->GetActor())
    {
        m_IsHit = true;
    }
}

void Lever::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void Lever::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}
void Lever::Paint(graphics::D2DRenderContext& ctx)
{
    m_Position = m_ActPtr->GetPosition();
    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(m_Position);
    matPivot.SetAsTranslate(DOUBLE2(-(m_BmpPtr->GetWidth()/AMOUNTOFFRAMES) / 2, -m_BmpPtr->GetHeight() / 2));
    ctx.set_world_matrix(matPivot * matTranslate);
    RECT boundingBox;
    boundingBox.left = 0;
    boundingBox.top = 0;
    assert(m_ClipWidth > std::numeric_limits<LONG>::min() && m_ClipWidth < std::numeric_limits<LONG>::max());
    if (m_IsHit)
    {
        boundingBox.left = (LONG)m_ClipWidth;
        boundingBox.top = 0;
    }
    boundingBox.right = boundingBox.left + (LONG)m_ClipWidth;
    boundingBox.bottom = m_BmpPtr->GetHeight();
    ctx.draw_bitmap(m_BmpPtr,boundingBox);
    ctx.set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void Lever::Tick(double deltaTime)
{

}
void Lever::Reset()
{
    m_IsHit = false;
}
void Lever::SetPosition(DOUBLE2 position)
{
    m_ActPtr->SetPosition(position);
}


