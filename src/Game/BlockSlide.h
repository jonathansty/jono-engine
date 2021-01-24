#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "Entity.h"
//-----------------------------------------------------
// BlockSlide Class									
//-----------------------------------------------------
class BlockSlide : public Entity
{
public:
	BlockSlide(float2 position,int width, int height );
	virtual ~BlockSlide( );

	// C++11 make the class non-copyable
	BlockSlide( const BlockSlide& ) = delete;
	BlockSlide& operator=( const BlockSlide& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual void Paint(graphics::D2DRenderContext& ctx);
    virtual void Tick(double deltaTime);
    virtual void Reset();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    int m_Width;
    int m_Height;
};

 
