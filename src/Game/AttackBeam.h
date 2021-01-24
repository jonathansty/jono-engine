#pragma once

#include "Animation.h"
class Level;

class AttackBeam 
{
public:
	explicit AttackBeam(float2 position );
	virtual ~AttackBeam( );

	// C++11 make the class non-copyable
	AttackBeam( const AttackBeam& ) = delete;
	AttackBeam& operator=( const AttackBeam& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Paint(graphics::D2DRenderContext& ctx);
    void Tick(double deltaTime);
    void SetLevel(Level* levelPtr);
    void SetPosition(float2 position);
    void SetGroundBitmap(String bitmapPath);
    bool isVisible();
    double GetLifeTime();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    static const int LIFETIME = 4;
    static const int MAXWIDTH = 15;
    float2 m_Position, m_TopPosition;
    double m_Width = 1;
    COLOR m_Color;
    double m_AccuTime = 0;
    PhysicsActor* m_ActPtr = nullptr;
    Level* m_LevelPtr = nullptr;
    Bitmap* m_BmpGroundPtr = nullptr;
    float2 m_RayStart, m_RayEnd;
};

 
