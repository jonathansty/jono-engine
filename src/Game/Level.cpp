#include "game.stdafx.h"		
#include "Level.h"

Level::Level(Bitmap* bmpPtr, String svgPath)
{
	// nothing to create
	// m_ActCirclePtr->AddContactListener(this);
    m_ActPtr = new PhysicsActor(float2(), 0, BodyType::STATIC);
    m_ActPtr->AddSVGShape(svgPath,0,0);

    m_ActPtr->SetName(String("Level"));

    m_BmpLevelPtr = bmpPtr;

}
Level::Level(Bitmap* bmpPtr, String svgPath, String svgBoundsPath):
Level(bmpPtr,svgPath)
{
    m_ActBoundsPtr = new PhysicsActor(float2(), 0, BodyType::STATIC);
    m_ActBoundsPtr->AddSVGShape(svgBoundsPath,0);
    m_ActBoundsPtr->SetName(String("LevelBounds"));

    b2Filter filter;
    filter.groupIndex = -5;
    m_ActBoundsPtr->SetCollisionFilter(filter);
}
Level::~Level()
{
    delete m_ActPtr;
    m_ActPtr = nullptr;

    delete m_ActBoundsPtr;
    m_ActBoundsPtr = nullptr;
    
}

void Level::Paint(graphics::D2DRenderContext& ctx)
{
    ctx.draw_bitmap(m_BmpLevelPtr);

}
int Level::GetHeight()
{
    return m_BmpLevelPtr->GetHeight();
}
int Level::GetWidth()
{
    return m_BmpLevelPtr->GetWidth();
}
PhysicsActor* Level::GetActor()
{
    return m_ActPtr;
}
PhysicsActor* Level::GetLevelBounds()
{
    if (m_ActBoundsPtr == nullptr)
    {
        return nullptr;
    }
    return m_ActBoundsPtr;
}
Bitmap* Level::GetBgBmpPtr()
{
    return m_BmpLevelPtr;
}