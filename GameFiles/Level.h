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
// Level Class									
//-----------------------------------------------------
class Level //: public ContactListener
{
public:
	Level(Bitmap* bmpPtr, String svgPath);
    Level(Bitmap* bmpPtr, String svgPath, String svgBoundsPath);
	virtual ~Level( );

	// C++11 make the class non-copyable
	Level( const Level& ) = delete;
	Level& operator=( const Level& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint();
    int GetWidth();
    int GetHeight();
    PhysicsActor* GetActor();
    PhysicsActor* GetLevelBounds();
    Bitmap* GetBgBmpPtr();

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
   
    PhysicsActor* m_ActPtr = nullptr;
    PhysicsActor* m_ActBoundsPtr = nullptr;
    Bitmap* m_BmpLevelPtr = nullptr;
};

 
