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
#include "Entity.h"

//-----------------------------------------------------
// LevelEnd Class									
//-----------------------------------------------------
class Lever;
class LevelEnd : public Entity
{
public:
	LevelEnd(float2 position,String nextLevel);
	virtual ~LevelEnd( );

	// C++11 make the class non-copyable
	LevelEnd( const LevelEnd& ) = delete;
	LevelEnd& operator=( const LevelEnd& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Tick(double deltaTime);
    void Paint(graphics::D2DRenderContext& ctx);
    void Reset();
    virtual bool isHit();
    std::vector<Lever*> GetLeversArray();
    bool AreLeversHit();
    String GetNextLevel();
    void SetAvatar(Avatar* avatarPtr);

    void Add(Lever* leverPtr);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    double m_Angle = 0;
    double m_RotationSpeed = 10;
    static const int HEIGHT = 50;
    static const int WIDTH = 50;
    Bitmap* m_BmpPtr = nullptr;
    bool m_IsHit = false;
    bool m_AreAllLeversHit = false;
    String m_NextLevelPath;


    ///MAKE IT SO WE USE AN ARRAY OF LEVERS
    ///PASS THE LEVER ARRAY WHEN CREATING THE OBJECT
    std::vector<Lever*>m_LeversPtrArr;

};

 
