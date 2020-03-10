//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "HUD.h"
#include "BitmapManager.h"
#include "Avatar.h"
#include "Slider.h"
#include "Game.h"
#include "Lever.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())
#define BITMAP_MANAGER (BitmapManager::GetSingleton())
#define SND_MANAGER (SoundManager::GetSingleton())
//---------------------------
// Constructor & Destructor
//---------------------------
HUD::HUD(Game* gamePtr)
{
    m_AvatarPtr = gamePtr->GetAvatar();
    m_LevelPtr = gamePtr->GetLevel();
    m_BmpMoneyDisplayPtr = BITMAP_MANAGER->LoadBitmapFile(String("Resources/UI/ui_MoneyDisplay.png"));
    m_BmpDeathIconPtr = BITMAP_MANAGER->LoadBitmapFile(String("Resources/UI/ui_DeathIcon.png"));
    m_BmpGameOverPtr = BITMAP_MANAGER->LoadBitmapFile(String("Resources/Menu/gameOver.png"));
    m_FntPtr = new Font(String("KenVector Future"), 40);
}

HUD::~HUD()
{
    delete m_FntPtr;
    m_FntPtr = nullptr;
    delete m_BtnSndMutePtr;
    m_BtnSndMutePtr = nullptr;
    delete m_BtnQuitGamePtr;
    m_BtnQuitGamePtr = nullptr;
    delete m_BtnQuitToMenuPtr;
    m_BtnQuitToMenuPtr = nullptr;
   

}

void HUD::Paint()
{
    GAME_ENGINE->SetFont(m_FntPtr);
    PaintMoneyWindow(DOUBLE2(10, 10));
    PaintDeathCounterWindow(DOUBLE2(GAME_ENGINE->GetWidth() - 100, 50));
    
    GAME_ENGINE->SetWorldMatrix(GAME_ENGINE->GetViewMatrix().Inverse());
    GAME_ENGINE->DrawString(String(m_AccuTime, 2), GAME_ENGINE->GetWidth() / 2, 10);
    GAME_ENGINE->SetDefaultFont(); 
    
    PaintLeverInfo();
    GAME_ENGINE->SetViewMatrix(GAME_ENGINE->GetViewMatrix());

    //GAME_ENGINE->SetWorldMatrix(GAME_ENGINE->GetViewMatrix().Inverse());
    
}
void HUD::Tick(double deltaTime)
{

    DOUBLE2 mousePosition = GAME_ENGINE->GetMousePositionDOUBLE2();
    if (m_BtnSndMutePtr != nullptr && m_BtnSndMutePtr->IsPressed())
    {
        if (m_Muted)
        {
            Bitmap* tmpBitmapPressed = BITMAP_MANAGER->LoadBitmapFile(String("Resources/UI/audioOn.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            SND_MANAGER->UnMuteAll();
        }
        else
        {
            Bitmap* tmpBitmapPressed = BITMAP_MANAGER->LoadBitmapFile(String("Resources/UI/audioOff.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            SND_MANAGER->MuteAll();
        }
        m_Muted = !m_Muted;
    }


    if (m_BtnQuitGamePtr!= nullptr && m_BtnQuitGamePtr->IsPressed())
    {
        GAME_ENGINE->QuitGame();
    }
    if (m_BtnQuitToMenuPtr != nullptr && m_BtnQuitToMenuPtr->IsPressed())
    {
        m_GoToStartMenu = true;
    }

    if (m_IsGameOverDrawn)
    {
        if (m_GameOverOpacity > 1)
        {
            m_GameOverOpacity--;
        }
    }
    
    
}
void HUD::PaintMoneyWindow(DOUBLE2 position)
{
    GAME_ENGINE->SetDefaultFont();
    int amountOfMoney = 0;
    if (m_AvatarPtr != nullptr)
    {
        amountOfMoney = m_AvatarPtr->GetAmountOfCoins();
    }
    int amountOfGold = 0, amountOfSilver = 0, amountOfCopper = 0;
    while (amountOfMoney > 3600)
    {
        amountOfGold++;
        amountOfMoney -= 3600;
    }
    while (amountOfMoney > 60)
    {
        amountOfSilver++;
        amountOfMoney -= 60;
    }
    amountOfCopper = amountOfMoney;
    
    int bitmapWidth = m_BmpMoneyDisplayPtr->GetWidth();
    int bitmapHeight = m_BmpMoneyDisplayPtr->GetHeight();

    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(position);
    GAME_ENGINE->SetWorldMatrix(matTranslate * GAME_ENGINE->GetViewMatrix().Inverse());
    GAME_ENGINE->DrawBitmap(m_BmpMoneyDisplayPtr);
    GAME_ENGINE->SetColor(COLOR(255, 255, 255));
    GAME_ENGINE->DrawString(String(amountOfGold), (int)(bitmapWidth - 30), (int)(5 ));
    GAME_ENGINE->DrawString(String(amountOfSilver), (int)(bitmapWidth - 70), (int)(5 ));
    GAME_ENGINE->DrawString(String(amountOfCopper), (int)(bitmapWidth - 110), (int)( 5 ));
    GAME_ENGINE->SetColor(COLOR(0, 0, 0));
    GAME_ENGINE->SetFont(m_FntPtr);
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());

}
void HUD::PaintDeathCounterWindow(DOUBLE2 position)
{
    MATRIX3X2 matTranslate, matPivot;
    DOUBLE2 positionIcon = DOUBLE2(-m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2);
    matTranslate.SetAsTranslate(position);
    matPivot.SetAsTranslate(positionIcon);
    GAME_ENGINE->SetWorldMatrix(matPivot * matTranslate * GAME_ENGINE->GetViewMatrix().Inverse());
    GAME_ENGINE->DrawBitmap(m_BmpDeathIconPtr);

    matPivot.SetAsTranslate(DOUBLE2(20 + m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2));
    GAME_ENGINE->SetWorldMatrix(matPivot * matTranslate * GAME_ENGINE->GetViewMatrix().Inverse());
    GAME_ENGINE->DrawString(String(m_AvatarPtr->GetDeaths()),DOUBLE2());
    GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void HUD::CreatePauseMenu()
{
    int dy = 30;
    m_BtnQuitGamePtr = new Button(String("Quit to desktop!"));
    m_BtnQuitGamePtr->SetBounds(GAME_ENGINE->GetWidth()/2 - 100, GAME_ENGINE->GetHeight() / 2, 200, 25);
    m_BtnQuitToMenuPtr = new Button(String("Quit to main menu!"));
    m_BtnQuitToMenuPtr->SetBounds(GAME_ENGINE->GetWidth()/2 - 100, GAME_ENGINE->GetHeight() / 2 - dy, 200, 25);
    
    m_IsInMenu = true;
}
void HUD::RemovePauseMenu()
{
    m_IsInMenu = false;
    m_GoToStartMenu = false;
   

    delete m_BtnQuitGamePtr;
    m_BtnQuitGamePtr = nullptr;

    delete m_BtnQuitToMenuPtr;
    m_BtnQuitToMenuPtr = nullptr;
}

void HUD::PaintGameOverWindow(DOUBLE2 position)
{
    MATRIX3X2 matTranslate, matPivot;
    matTranslate.SetAsTranslate(position);
    matPivot.SetAsTranslate(DOUBLE2(-m_BmpGameOverPtr->GetWidth() / 2, -m_BmpGameOverPtr->GetHeight() / 2));
    GAME_ENGINE->SetWorldMatrix(matPivot*matTranslate * GAME_ENGINE->GetViewMatrix().Inverse());
    GAME_ENGINE->SetColor(COLOR(0, 0, 0, m_GameOverOpacity));
    GAME_ENGINE->DrawBitmap(m_BmpGameOverPtr);
    m_IsGameOverDrawn = true;
    GAME_ENGINE->SetWorldMatrix(GAME_ENGINE->GetViewMatrix().Inverse());
}
bool HUD::IsGoToStartMenu()
{
    return m_GoToStartMenu;
}
void HUD::ResetIsInMenu()
{
    m_IsInMenu = false;
}
void HUD::CreateSoundMuteBtn()
{
    Bitmap* tmpBitmapPressed = BITMAP_MANAGER->LoadBitmapFile(String("Resources/UI/audioOn.png"));
    m_BtnSndMutePtr = new Button(String("Mute Sound"));
    m_BtnSndMutePtr->SetBounds(GAME_ENGINE->GetWidth() - 60, GAME_ENGINE->GetHeight() - 60, tmpBitmapPressed->GetWidth(), tmpBitmapPressed->GetHeight());
    m_BtnSndMutePtr->SetImageMode(true);
    m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
    m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
}
void HUD::RemoveSoundMuteBtn()
{
    if (m_BtnSndMutePtr != nullptr)
    {
        delete m_BtnSndMutePtr;
        m_BtnSndMutePtr = nullptr;
    }
    
}
void HUD::ResetTimer()
{
    m_AccuTime = 0;
}
double HUD::GetTime()
{
    return m_AccuTime;
}
void HUD::SetTime(double time)
{
    m_AccuTime = time;
}
void HUD::LinkLevers(std::vector<Lever*>tmpArray)
{

    m_AreThereLevers = true;
    m_LeversPtrArr = tmpArray;
}
void HUD::PaintLeverInfo()
{

    for (size_t i = 0; i < m_LeversPtrArr.size(); i++)
    {
        if (m_LeversPtrArr[i]->isHit())
        {
            GAME_ENGINE->DrawString(String("Lever ") + String(i) + String(" is hit!"), 10, 150 + (int)i * 15);
        }
        else
        {
            GAME_ENGINE->DrawString(String("Lever ") + String(i) + String(" is not hit!"), 10, 150 + (int)i * 15);
        }
    }
}