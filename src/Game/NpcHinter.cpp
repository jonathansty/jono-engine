#include "game.stdafx.h"		
	
#include "NpcHinter.h"
#include "Avatar.h"
#include "BitmapManager.h"

NpcHinter::NpcHinter(float2 position, String tipText, Bitmap* bmpCharacterBmp):
Entity(position), m_TipText(tipText), m_BmpBodyPtr(bmpCharacterBmp)
{

}
NpcHinter::NpcHinter(float2 position, String tipText):
Entity(position), m_TipText(tipText)
{
    m_BmpBodyPtr = BitmapManager::instance()->load_image(String("Resources/Entity/NpcIdle.png"));
}
NpcHinter::~NpcHinter()
{
 
}

//-------------------------------------------------------
// ContactListener overloaded member function definitions
//-------------------------------------------------------
void NpcHinter::BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void NpcHinter::EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr)
{

}

void NpcHinter::ContactImpulse(PhysicsActor *actThisPtr, double impulse)
{

}

//CREATE A SPRITE AND TEST IT
void NpcHinter::PaintDebug(graphics::D2DRenderContext& ctx)
{
    float3x3 matPosition;
    matPosition= float3x3::translation(m_Position);
    ctx.set_world_matrix(matPosition);
    ctx.draw_ellipse(float2(), TALKRADIUS, TALKRADIUS);
    ctx.set_world_matrix(float3x3::identity());
}
void NpcHinter::Paint(graphics::D2DRenderContext &ctx) {
    float3x3 matTranslate, matTextTranslate;
   
    matTranslate= float3x3::translation(m_Position);
    
    RECT boundingBox;
    int clipWidth = m_BmpBodyPtr->GetWidth()/MAXFRAMES;
    int clipHeight = m_BmpBodyPtr->GetHeight();
    boundingBox.left = m_FrameNr*clipWidth + 4;
    boundingBox.top = 0;
    boundingBox.right = boundingBox.left + clipWidth  - 2;
    boundingBox.bottom = boundingBox.top + clipHeight;

    
    
    float3x3 matPivot, matNpcHitBoxTransform,matMirror;
    matPivot= float3x3::translation(float2(-clipWidth/2, -clipHeight/2));
    matNpcHitBoxTransform = hlslpp::mul(matPivot, matTranslate);
    if (m_FacingDirection == String("LEFT"))
    {
        matMirror = float3x3::scale(-1, 1);
    }
    else
    {
        matMirror = float3x3::scale(1, 1);
    }
    
    ctx.set_world_matrix(hlslpp::mul(matPivot, hlslpp::mul(matMirror,matTranslate)));
    ctx.draw_bitmap(m_BmpBodyPtr, boundingBox);
    ctx.set_world_matrix(matNpcHitBoxTransform);
	ctx.set_font(nullptr);
    matPivot= float3x3::translation(float2(-(m_TipText.Length() * 16) / 2, -50));

    if (m_IsArmed)
    {
		matTextTranslate= float3x3::translation(float2(0, 0));
		ctx.set_world_matrix(hlslpp::mul(matPivot, hlslpp::mul(matTextTranslate, matNpcHitBoxTransform)));
        ctx.set_color(COLOR(255,255,255, m_Opacity));

        // TODO: Fix validation error 
        ctx.draw_string(m_TipText, RECT2(0, 0, m_TipText.Length() * 16, 50));

		ctx.set_world_matrix(float3x3::identity());
    }
   
    ctx.set_color(COLOR(0, 0, 0, 255));
    ctx.set_font(nullptr);
}
void NpcHinter::Tick(double deltaTime)
{
    m_Accutime+= deltaTime;
    if (m_Accutime > (double)1/FRAMESPERSECOND)
    {
        m_FrameNr++;
        m_FrameNr %= MAXFRAMES;
        m_Accutime -= (double)1 / FRAMESPERSECOND;
    }
    if (m_IsArmed)
    {
        m_TipDisplayTime += deltaTime;
    }
    if (float(hlslpp::length(float2(m_AvatarPtr->GetPosition() - m_Position))) < float(TALKRADIUS))
    {
        if (m_IsArmed == false)
        {
            m_IsArmed = true;
        }
    }
    if (m_TipDisplayTime < DISPLAYTIME && m_IsArmed)
    {
        if (m_Opacity > 255)
        {
            m_Opacity = 255;
        }
        else if (m_Opacity < 245)
        {
            m_Opacity += 10;
        }
    }

    if (m_TipDisplayTime > DISPLAYTIME)
    {
        if (m_Opacity > 10)
        {
            m_Opacity -= 10;
        }
        else if (m_Opacity < 10)
        {
            m_Opacity = 0;
        }
        
        
    }


    
}
void NpcHinter::Reset()
{

}
void NpcHinter::SetFacingDirection(String facingDirection)
{
    if (facingDirection != String("LEFT") &&
        facingDirection != String("RIGHT"))
    {
        GameEngine::instance()->message_box(String("Please fix your config file! At object ") + m_Name);
    }
    m_FacingDirection = facingDirection;
}


