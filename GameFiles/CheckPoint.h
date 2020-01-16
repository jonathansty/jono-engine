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
#include "Entity.h"
//-----------------------------------------------------
// CheckPoint Class									
//-----------------------------------------------------
class RotLight;
class CheckPoint : public Entity
{
public:
	CheckPoint(DOUBLE2 position, Bitmap* bmpFlagPtr);
	virtual ~CheckPoint( );

	// C++11 make the class non-copyable
	CheckPoint( const CheckPoint& ) = delete;
	CheckPoint& operator=( const CheckPoint& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual PhysicsActor* GetActor();
    virtual void Reset();
    virtual void SetSpawnPosition(DOUBLE2 respawnPosition);
    void Paint();
    void Tick(double dTime);
    bool isHit();
    double GetCameraAngle();
    void SetCameraAngle(double angle);

    void SetCameraPosition(DOUBLE2 position);
    DOUBLE2 GetCameraPosition();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    double m_CameraAngle = 0;
    DOUBLE2 m_CameraPosition;
    Sound* m_SndPtr = nullptr;
    Bitmap* m_BmpFlagPtr = nullptr;
    RotLight* m_RotLightPtr = nullptr;
    bool m_IsHit = false;
};

 
