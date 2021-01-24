#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "ContactListener.h"
//-----------------------------------------------------
// PickUp Class									
//-----------------------------------------------------
class PickUp : public ContactListener
{
public:
	PickUp(float2 position);
	virtual ~PickUp( );

	// C++11 make the class non-copyable
	PickUp( const PickUp& ) = delete;
	PickUp& operator=( const PickUp& ) = delete;

	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr) override{};
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr) override{};
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse) override{};

    virtual void Tick(double deltaTime) = 0;
    virtual void Paint(graphics::D2DRenderContext& ctx) = 0;
    virtual bool IsHit();

    virtual PhysicsActor* GetActor();
    virtual void SetName(String name);
    virtual String GetName();

protected:
    String m_Name = String("NaN");
    PhysicsActor* m_ActPtr = nullptr;
    float2 m_Position;
    bool m_IsHit = false;
    
    sound* m_SndHitPtr = nullptr;
    b2Filter m_CollisionFilter;

};

 
