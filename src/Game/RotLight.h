#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "Animation.h"
//-----------------------------------------------------
// RotLight Class									
//-----------------------------------------------------
class RotLight : public Animation
{
public:
	RotLight(float2 position);
	virtual ~RotLight( );

	// C++11 make the class non-copyable
	RotLight( const RotLight& ) = delete;
	RotLight& operator=( const RotLight& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    enum class drawState{
        SPAWNING,
        MAXRADIUS
    };
    void Paint(graphics::D2DRenderContext& ctx);
    void Tick(double deltaTime);

    void SetColor(COLOR color);
    void SetRotSpeed(int speed);
    void SetRadius(double radius);
    void SetPosition(float2 position);
    void SetDrawState(drawState drawState);
    float2 GetPosition();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int MINRADIUS = 20;
    double m_Angle = 0;
    double m_Radius = 10;
    double m_MaxRadius = 100;
    double m_SpawnSpeed = 250;
    int m_RotSpeed = 2;
    COLOR m_Color= COLOR(0, 0, 0);
    drawState m_DrawState = drawState::SPAWNING;
};

 
