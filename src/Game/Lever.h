#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE1
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "Entity.h"
//-----------------------------------------------------
// Lever Class									
//-----------------------------------------------------
class Lever : public Entity
{
public:
	Lever(float2 position, Bitmap* bmpPtr);
	virtual ~Lever( );

	// C++11 make the class non-copyable
	Lever( const Lever& ) = delete;
	Lever& operator=( const Lever& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void Paint(graphics::D2DRenderContext& ctx) override;
    virtual void Tick(double deltaTime) override;
    void Reset();
    void SetPosition(float2 position);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int AMOUNTOFFRAMES = 2;
    double m_ClipWidth = 0;
    Bitmap* m_BmpPtr = nullptr;
};

 
