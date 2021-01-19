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
#include "Animation.h"
//-----------------------------------------------------
// CombRotLightCpBg Class									
//-----------------------------------------------------
class RotLight;
class CheckPointBg;
class CombRotLightCpBg : public Animation
{
public:
	CombRotLightCpBg(DOUBLE2 position,int radius,Bitmap* bitmap, COLOR color);
	virtual ~CombRotLightCpBg( );

	// C++11 make the class non-copyable
	CombRotLightCpBg( const CombRotLightCpBg& ) = delete;
	CombRotLightCpBg& operator=( const CombRotLightCpBg& ) = delete;

	virtual void Paint(graphics::D2DRenderContext &ctx);
    virtual void Tick(double deltaTime);

private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    RotLight *m_RotLightPtr = nullptr;
    CheckPointBg *m_CheckPointBgPtr = nullptr;
};

 
