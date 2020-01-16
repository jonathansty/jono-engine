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
// Sprite Class									
//-----------------------------------------------------
class Sprite //: public ContactListener
{
public:
	Sprite(String spritesheetPath, int amountRows, int amountCols, int maxFrame);
	virtual ~Sprite( );

	// C++11 make the class non-copyable
	Sprite( const Sprite& ) = delete;
	Sprite& operator=( const Sprite& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint(int rowNumber);
    void Paint(int rowNumber, int maxFrame);
    void Tick(double deltaTime);
    void SetFrameRate(double frameRate);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    int m_Rows = 0;
    int m_Cols = 0;
    int m_MaxFrame = 0;
    int m_ClipWidth = 0;
    int m_ClipHeight = 0;
    double m_FrameRate = 12.0;
    double m_AccuTime = 0;
    int m_FrameNr = 0;
    Bitmap* m_BmpSpriteSheetPtr = nullptr;
};

 
