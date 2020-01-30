//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
#include "Sprite.h"

#define GAME_ENGINE (GameEngine::GetSingleton())

Sprite::Sprite(String spritesheetPath, int amountRows, int amountCols, int maxFrame) :
m_Rows(amountRows),
m_Cols(amountCols),
m_MaxFrame(maxFrame)
{
    m_BmpSpriteSheetPtr = new Bitmap(spritesheetPath);
    m_ClipWidth = m_BmpSpriteSheetPtr->GetWidth() / amountCols;
    m_ClipHeight = m_BmpSpriteSheetPtr->GetHeight() / amountRows;
}

Sprite::~Sprite()
{
    if (m_BmpSpriteSheetPtr)
    {
        delete m_BmpSpriteSheetPtr;
    }
}

void Sprite::Paint(int rowNumber)
{
    int clipWidth = m_BmpSpriteSheetPtr->GetWidth() / m_Cols;
    int clipHeight = m_BmpSpriteSheetPtr->GetHeight() / m_Rows;
    RECT boundingBox;
    boundingBox.left = m_FrameNr%m_Cols * clipWidth;
    boundingBox.top = rowNumber * clipHeight;
    boundingBox.right = boundingBox.left + clipWidth;
    boundingBox.bottom = boundingBox.top + clipHeight;
    GAME_ENGINE->DrawBitmap(m_BmpSpriteSheetPtr,boundingBox);
}
void Sprite::Paint(int rowNumber, int maxFrame)
{
    int clipWidth = m_BmpSpriteSheetPtr->GetWidth() / m_Cols;
    int clipHeight = m_BmpSpriteSheetPtr->GetHeight() / m_Rows;
    RECT boundingBox;
    boundingBox.left = m_FrameNr%maxFrame * clipWidth;
    boundingBox.top = rowNumber * clipHeight;
    boundingBox.right = boundingBox.left + clipWidth;
    boundingBox.bottom = boundingBox.top + clipHeight;
    GAME_ENGINE->DrawBitmap(m_BmpSpriteSheetPtr, boundingBox);
}
void Sprite::SetFrameRate(double frameRate)
{
    m_FrameRate = frameRate;
}
void Sprite::Tick(double deltaTime)
{
    m_AccuTime += deltaTime;
    if (m_AccuTime > 1 /m_FrameRate)
    {
        m_FrameNr++;
        m_FrameNr = m_FrameNr% m_MaxFrame;
        m_AccuTime -= 1 / m_FrameRate;
    }
}
