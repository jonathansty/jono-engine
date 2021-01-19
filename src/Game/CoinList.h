#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
//-----------------------------------------------------
// CoinList Class									
//-----------------------------------------------------
class Coin;
class CoinList //: public ContactListener
{
public:
	CoinList( );
	virtual ~CoinList( );

	// C++11 make the class non-copyable
	CoinList( const CoinList& ) = delete;
	CoinList& operator=( const CoinList& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Tick(double deltaTime);
    void Paint(graphics::D2DRenderContext& ctx);
    void PaintDebug();

    void Add(Coin* coinPtr);
    void Remove(Coin* coinPtr);
    void RemoveAll();
    void RemoveAllContactListeners();


    int GetValue();
    int IsHit();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    int m_AmountOfCoins = 0;
    std::vector<Coin*>m_CoinsPtrArr;
    
};

 
