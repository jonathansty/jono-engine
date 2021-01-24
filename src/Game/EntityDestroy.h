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
// EntityDestroy Class									
//-----------------------------------------------------
class EntityDestroy : public Animation
{
public:
	EntityDestroy(float2 position );
	virtual ~EntityDestroy( );

	// C++11 make the class non-copyable
	EntityDestroy( const EntityDestroy& ) = delete;
	EntityDestroy& operator=( const EntityDestroy& ) = delete;

    virtual void Tick(double deltaTime);
	virtual void Paint(graphics::D2DRenderContext &ctx);

    double GetOpacity();
    void SetRadius(int radius);
private: 
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

    int m_Radius = 250;
    double m_Opacity = 1;
    double m_Scale = 0.2;
};

 
