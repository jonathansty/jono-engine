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
#include "PickUp.h"
//-----------------------------------------------------
// Coin Class									
//-----------------------------------------------------
class Coin : public PickUp
{
public:
	Coin(DOUBLE2 position, Bitmap* bmpPtr);
	virtual ~Coin( );

	// C++11 make the class non-copyable
	Coin( const Coin& ) = delete;
	Coin& operator=( const Coin& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    void Tick(double deltaTime);
    void Paint();
    bool IsHit();

    void SetCoinValue(int value);

    DOUBLE2 GetPosition();
    int GetCoinValue();
    void SetIndex(int number);

    void RemoveContactListener();
    virtual void SetName(String name);
    virtual String GetName();
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    int m_Index = 0;
    Bitmap* m_BmpCoinPtr = nullptr;
    int m_CoinValue = 0;
};

 
