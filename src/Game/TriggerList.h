#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
//-----------------------------------------------------
// TriggerList Class									
//-----------------------------------------------------
class CameraTrigger;
class TriggerList //: public ContactListener
{
public:
	TriggerList( );
	virtual ~TriggerList( );

	// C++11 make the class non-copyable
	TriggerList( const TriggerList& ) = delete;
	TriggerList& operator=( const TriggerList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Add(CameraTrigger* triggerPtr);
    void RemoveAll();
    void Tick(double deltaTime);
    void Reset();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

    std::vector<CameraTrigger*>m_CameraTriggersPtrArr;
};

 
