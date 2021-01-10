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
//-----------------------------------------------------
// EntityList Class									
//-----------------------------------------------------
class Entity;
class EntityList //: public ContactListener
{
public:
	EntityList( );
	virtual ~EntityList( );

	// C++11 make the class non-copyable
	EntityList( const EntityList& ) = delete;
	EntityList& operator=( const EntityList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint();
    void PaintDebug();
    void Tick(double deltaTime);

    void Add(Entity* tmpEntityPtr);
    void Remove(Entity* tmpEntityPtr);
    void RemoveAll();
    void Reset();
    Entity* isHit();
    Entity* IsDead();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    std::vector<Entity* >m_EntityPtrArr;


};

 
