#pragma once

class Animation 
{
public:
	Animation(float2 position);
	virtual ~Animation( );

	// C++11 make the class non-copyable
	Animation( const Animation& ) = delete;
	Animation& operator=( const Animation& ) = delete;

    virtual void Tick(double deltaTime) = 0;
    virtual void Paint(graphics::D2DRenderContext& ctx) = 0;
    virtual bool IsEnded();

protected:
    float2 m_Position;
    bool m_IsEnded = false;

};

 
