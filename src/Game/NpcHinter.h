#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

#include "Entity.h"
//-----------------------------------------------------
// NpcHinter Class									
//-----------------------------------------------------
class NpcHinter : public Entity
{
public:
    NpcHinter(DOUBLE2 position, String tipText);
	NpcHinter(DOUBLE2 position, String tipText, Bitmap* bmpCharacterPtr);
	virtual ~NpcHinter( );

	// C++11 make the class non-copyable
	NpcHinter( const NpcHinter& ) = delete;
	NpcHinter& operator=( const NpcHinter& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);

    virtual void PaintDebug(graphics::D2DRenderContext& ctx) override;
	virtual void Paint(graphics::D2DRenderContext &ctx) override;

    void Tick(double deltaTime);
    void Reset();

    void SetFacingDirection(String facingDirection);

private: 
    static const int MAXFRAMES = 10;
    static const int FRAMESPERSECOND = 4;
    static const int TALKRADIUS = 250;
    static const int DISPLAYTIME = 5;

    double m_Accutime = 0;
    double m_TipDisplayTime = 0;
    unsigned char m_Opacity = 0;

    int m_FrameNr = 0;
    Bitmap* m_BmpBodyPtr = nullptr;
    String m_TipText;
    String m_FacingDirection;
    bool m_IsArmed = false;
};

 
