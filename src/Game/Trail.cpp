#include "stdafx.h"		
	
#include "Trail.h"

Trail::Trail(float2 position) :
m_Position(position)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
}

Trail::~Trail()
{
    m_deqTrailPtrArr.clear();
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void Trail::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void Trail::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void Trail::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void Trail::Paint(graphics::D2DRenderContext& ctx, float2 position)
{
    m_Position = position;
    m_deqTrailPtrArr.push_front(m_Position);

    if (m_deqTrailPtrArr.size() > m_TrailLength)
    {
        m_deqTrailPtrArr.pop_back();
    }
    ctx.set_color(COLOR(255, 255, 255, m_Opacity));
    for (std::size_t i = 0, n = m_deqTrailPtrArr.size(); i < n - 1; i++)
    {
        float2 pos = m_deqTrailPtrArr[i];
        float2 pos2 = m_deqTrailPtrArr[i + 1];
        float2 midPos = (pos2 + pos) / 2;
        double size = m_Size;
        if (size - (size / m_TrailLength)*(double)i > 0)
        {

            ctx.fill_ellipse(pos, size - (size / m_TrailLength)*i, size - (size / m_TrailLength)*i);
            float2 vector = pos2 - pos;
            if (float(hlslpp::length(vector)) > 5.0f)
            {
                for (int j = 0; j < m_AmountOfInterpolation; j++)
                {
                    double spaceBetween = hlslpp::length(vector) / m_AmountOfInterpolation;
                    float2 normVector = hlslpp::normalize(vector);
                    midPos = normVector*j*spaceBetween;
                    ctx.fill_ellipse(pos + midPos, size - (size / m_TrailLength)*i, size - (size / m_TrailLength)*i);
                }
            }
        }

    }
    ctx.set_color(COLOR(0, 0, 0,255));
}
void Trail::SetSize(double size)
{
    m_Size = size;
}
void Trail::SetTrailLength(double length)
{
    m_TrailLength = length;
}
void Trail::SetInterpolation(int numberOfCircles)
{
    m_AmountOfInterpolation = numberOfCircles;
}
void Trail::SetOpacity(int opacity)
{
    m_Opacity = opacity;
}
