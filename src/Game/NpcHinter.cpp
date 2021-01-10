#include "stdafx.h"		
	
#include "NpcHinter.h"
#include "Avatar.h"
#include "BitmapManager.h"

NpcHinter::NpcHinter(DOUBLE2 position, String tipText, Bitmap* bmpCharacterBmp):
Entity(position), m_TipText(tipText), m_BmpBodyPtr(bmpCharacterBmp)
{

}
NpcHinter::NpcHinter(DOUBLE2 position, String tipText):
Entity(position), m_TipText(tipText)
{
    m_BmpBodyPtr = bitmap_manager::instance()->LoadBitmapFile(String("Resources/Entity/NpcIdle.png"));
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
void NpcHinter::PaintDebug()
{
    MATRIX3X2 matPosition;
    matPosition.SetAsTranslate(m_Position);
    game_engine::instance()->SetWorldMatrix(matPosition);
    game_engine::instance()->DrawEllipse(DOUBLE2(), TALKRADIUS, TALKRADIUS);
    game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void NpcHinter::Paint()
{
    MATRIX3X2 matTranslate, matTextTranslate;
   
    matTranslate.SetAsTranslate(m_Position);
    
    RECT boundingBox;
    int clipWidth = m_BmpBodyPtr->GetWidth()/MAXFRAMES;
    int clipHeight = m_BmpBodyPtr->GetHeight();
    boundingBox.left = m_FrameNr*clipWidth + 4;
    boundingBox.top = 0;
    boundingBox.right = boundingBox.left + clipWidth  - 2;
    boundingBox.bottom = boundingBox.top + clipHeight;

    
    
    MATRIX3X2 matPivot, matNpcHitBoxTransform,matMirror;
    matPivot.SetAsTranslate(DOUBLE2(-clipWidth/2, -clipHeight/2));
    matNpcHitBoxTransform = matPivot * matTranslate;
    if (m_FacingDirection == String("LEFT"))
    {
        matMirror.SetAsScale(-1, 1);
    }
    else
    {
        matMirror.SetAsScale(1, 1);
    }
    
    game_engine::instance()->SetWorldMatrix(matPivot * matMirror*matTranslate);
    game_engine::instance()->DrawBitmap(m_BmpBodyPtr, boundingBox);
    game_engine::instance()->SetWorldMatrix(matNpcHitBoxTransform);
    Font* fntTmpPtr = new Font(String(""), 16);
    fntTmpPtr->SetAlignHCenter();
    game_engine::instance()->set_font(fntTmpPtr);
    matPivot.SetAsTranslate(DOUBLE2(-(m_TipText.Length() * 16) / 2, -50));

    matTextTranslate.SetAsTranslate(DOUBLE2(0, 0));
    game_engine::instance()->SetWorldMatrix(matPivot* matTextTranslate*matNpcHitBoxTransform);
    if (m_IsArmed)
    {
        
        game_engine::instance()->set_color(COLOR(255,255,255, m_Opacity));
        game_engine::instance()->DrawString(m_TipText, RECT2(0, 0, m_TipText.Length() * 16, 50));
    }
   
    game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
    game_engine::instance()->set_color(COLOR(0, 0, 0, 255));
    game_engine::instance()->set_default_font();
    delete fntTmpPtr;
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
    if (DOUBLE2(m_AvatarPtr->GetPosition() - m_Position).Length() < TALKRADIUS)
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
        game_engine::instance()->message_box(String("Please fix your config file! At object ") + m_Name);
    }
    m_FacingDirection = facingDirection;
}


