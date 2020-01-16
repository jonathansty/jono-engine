//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "NpcHinter.h"
#include "Avatar.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
NpcHinter::NpcHinter(DOUBLE2 position, String tipText, Bitmap* bmpCharacterBmp):
Entity(position), m_TipText(tipText), m_BmpBodyPtr(bmpCharacterBmp)
{

}
NpcHinter::NpcHinter(DOUBLE2 position, String tipText):
Entity(position), m_TipText(tipText)
{
    m_BmpBodyPtr = BitmapManager::GetSingleton()->LoadBitmapFile(String("Resources/Entity/NpcIdle.png"));
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
    GAME_ENGINE->SetWorldMatrix(matPosition);
    GAME_ENGINE->DrawEllipse(DOUBLE2(), TALKRADIUS, TALKRADIUS);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
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
    
    GAME_ENGINE->SetWorldMatrix(matPivot * matMirror*matTranslate);
    GAME_ENGINE->DrawBitmap(m_BmpBodyPtr, boundingBox);
    GAME_ENGINE->SetWorldMatrix(matNpcHitBoxTransform);
    Font* fntTmpPtr = new Font(String(""), 16);
    fntTmpPtr->SetAlignHCenter();
    GAME_ENGINE->SetFont(fntTmpPtr);
    matPivot.SetAsTranslate(DOUBLE2(-(m_TipText.Length() * 16) / 2, -50));

    matTextTranslate.SetAsTranslate(DOUBLE2(0, 0));
    GAME_ENGINE->SetWorldMatrix(matPivot* matTextTranslate*matNpcHitBoxTransform);
    if (m_IsArmed)
    {
        
        GAME_ENGINE->SetColor(COLOR(255,255,255, m_Opacity));
        GAME_ENGINE->DrawString(m_TipText, RECT2(0, 0, m_TipText.Length() * 16, 50));
    }
   
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
    GAME_ENGINE->SetColor(COLOR(0, 0, 0, 255));
    GAME_ENGINE->SetDefaultFont();
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
        GAME_ENGINE->MessageBox(String("Please fix your config file! At object ") + m_Name);
    }
    m_FacingDirection = facingDirection;
}


