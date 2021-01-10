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
// CameraTrigger Class									
//-----------------------------------------------------
class Camera;
class CameraTrigger : public ContactListener
{
public:
	CameraTrigger(DOUBLE2 position,int width, int height);
	virtual ~CameraTrigger( );

	// C++11 make the class non-copyable
	CameraTrigger( const CameraTrigger& ) = delete;
	CameraTrigger& operator=( const CameraTrigger& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    virtual bool IsHit();
    virtual void Tick(double deltaTime) = 0;
    virtual void SetCamera(Camera* cameraPtr);
    virtual void Reset();
protected:
    bool m_IsHit = false;
    DOUBLE2 m_Position;
    int m_Width;
    int m_Height;

    PhysicsActor* m_ActPtr = nullptr;
    Camera* m_CameraPtr = nullptr;
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

};

 
