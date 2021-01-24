#pragma once

#include "Entity.h"

class Arrow : public Entity
{
public:
	Arrow(float2 position, Bitmap* bmpPtr);
	virtual ~Arrow( );

	// C++11 make the class non-copyable
	Arrow( const Arrow& ) = delete;
	Arrow& operator=( const Arrow& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    
    virtual void Paint(graphics::D2DRenderContext& ctx) override;
    virtual void Tick(double deltaTime) override;

    PhysicsActor* GetActor();
    void SetPushPower(double pushPower);
private:
    Bitmap* m_BmpPtr = nullptr;
    PhysicsActor* m_ActBottomTriggerPtr = nullptr;
    double m_PushPower = 1200;
    static int m_InstanceCounter;

    //Sounds
    sound* m_SndJumpPtr = nullptr;

};

 
