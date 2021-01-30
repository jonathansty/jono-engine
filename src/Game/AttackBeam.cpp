//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "AttackBeam.h"
#include "Level.h"
#include "BitmapManager.h"


//---------------------------
// Constructor & Destructor
//---------------------------
AttackBeam::AttackBeam(float2 position)
		: m_Position(position) {
    m_Position = position;
    m_Color = COLOR(255, 255, 255, 255);
    m_ActPtr = new PhysicsActor(position, 0, BodyType::STATIC);
    

    
}

AttackBeam::~AttackBeam()
{
    delete m_ActPtr;
    m_ActPtr = nullptr;
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
//void AttackBeam::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AttackBeam::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
//{
//
//}
//
//void AttackBeam::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
//{
//
//}
void AttackBeam::Paint(graphics::D2DRenderContext& ctx)
{
    ctx.set_color(m_Color);
    if (float(m_TopPosition.y) < float(m_Position.y))
    {
        ctx.fill_rect((int)(-m_Width + m_Position.x), (int)m_TopPosition.y, (int)(m_Width + m_Position.x), (int)m_Position.y);
    }
    
    ctx.set_color(COLOR(0, 0, 0,255));

    float3x3 matTranslate, matPivot;
    matTranslate = float3x3::translation(m_Position);
    matPivot = float3x3::translation(-m_BmpGroundPtr->GetWidth() / 2, -m_BmpGroundPtr->GetHeight());
    ctx.set_world_matrix(matPivot * matTranslate);
    ctx.draw_bitmap(m_BmpGroundPtr);
   
    ctx.set_world_matrix(float3x3::identity());
}
void AttackBeam::Tick(double deltaTime)
{

    m_AccuTime += deltaTime;
    if (m_Width < MAXWIDTH)
    {
        m_Width += 2;
    }
    m_RayStart = m_Position;
    m_RayEnd = m_Position + float2(0, 600);

    float2 intersection, normal,intersectionTop;
    double levelFraction = 0, levelTopFraction = 0;
    bool isLevelHit = false;
    bool isLevelHitTop = false;
    if (m_LevelPtr != nullptr)
    {
		isLevelHit = m_LevelPtr->GetActor()->Raycast(m_RayStart - float2(0, 5), m_RayEnd, intersection, normal, levelFraction);
        m_RayEnd = m_Position + float2(0, -600);
		isLevelHitTop = m_LevelPtr->GetActor()->Raycast(m_RayStart - float2(0, 10), m_RayEnd, intersectionTop, normal, levelTopFraction);
    }
    if (isLevelHit)
    {
        m_Position.y = intersection.y;
    }
    else
    {
        m_Position.y += 300;
    }
    if (isLevelHitTop)
    {
        m_TopPosition.y = intersectionTop.y;
    }
    else
    {
        m_TopPosition.y -= 300;
    }
    
    if (LIFETIME - m_AccuTime >= 0)
    {
        if (m_Color.alpha > 0)
        {
            m_Color.alpha = (unsigned char)(255 - (m_AccuTime * 255 / (LIFETIME - 2)));
        }
        if (m_BmpGroundPtr->GetOpacity() > 0.01)
        {
            m_BmpGroundPtr->SetOpacity(1 - (m_AccuTime / LIFETIME));
        }
        
        
    }
}
void AttackBeam::SetPosition(float2 position)
{
    m_Position = position;
}
bool AttackBeam::isVisible()
{
    if (m_Color.alpha > 0)
    {
        return true;
    }
    return false;
}

double AttackBeam::GetLifeTime()
{
    return m_AccuTime;
}
void AttackBeam::SetLevel(Level* levelPtr)
{
    m_LevelPtr = levelPtr;
}
void AttackBeam::SetGroundBitmap(String bitmapPath)
{
    m_BmpGroundPtr = BitmapManager::instance()->load_image(bitmapPath);
}