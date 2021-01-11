#include "stdafx.h"		
	
#include "SoundManager.h"
#include "HUD.h"
#include "BitmapManager.h"
#include "Avatar.h"
#include "Slider.h"
#include "Game.h"
#include "Lever.h"


HUD::HUD(Game* gamePtr)
{
    m_AvatarPtr = gamePtr->GetAvatar();
    m_LevelPtr = gamePtr->GetLevel();
    m_BmpMoneyDisplayPtr = BitmapManager::instance()->load_image(String("Resources/UI/ui_MoneyDisplay.png"));
    m_BmpDeathIconPtr = BitmapManager::instance()->load_image(String("Resources/UI/ui_DeathIcon.png"));
    m_BmpGameOverPtr = BitmapManager::instance()->load_image(String("Resources/Menu/gameOver.png"));
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
    GameEngine::instance()->set_font(m_FntPtr);
    PaintMoneyWindow(DOUBLE2(10, 10));
    PaintDeathCounterWindow(DOUBLE2(GameEngine::instance()->get_width() - 100, 50));
    
    GameEngine::instance()->set_world_matrix(GameEngine::instance()->get_view_matrix().Inverse());
    GameEngine::instance()->DrawString(String(m_AccuTime, 2), GameEngine::instance()->get_width() / 2, 10);
    GameEngine::instance()->set_default_font(); 
    
    PaintLeverInfo();
    GameEngine::instance()->set_view_matrix(GameEngine::instance()->get_view_matrix());
    
}
void HUD::Tick(double deltaTime)
{

    XMFLOAT2 mousePosition = GameEngine::instance()->get_mouse_pos_in_viewport();
    if (m_BtnSndMutePtr != nullptr && m_BtnSndMutePtr->IsPressed())
    {
        if (m_Muted)
        {
            Bitmap* tmpBitmapPressed = BitmapManager::instance()->load_image(String("Resources/UI/audioOn.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            SoundManager::instance()->UnMuteAll();
        }
        else
        {
            Bitmap* tmpBitmapPressed = BitmapManager::instance()->load_image(String("Resources/UI/audioOff.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            SoundManager::instance()->MuteAll();
        }
        m_Muted = !m_Muted;
    }


    if (m_BtnQuitGamePtr!= nullptr && m_BtnQuitGamePtr->IsPressed())
    {
        GameEngine::instance()->quit_game();
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
    GameEngine::instance()->set_default_font();
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

    GameEngine* eng = GameEngine::instance();
    eng->set_world_matrix(matTranslate * GameEngine::instance()->get_view_matrix().Inverse());
    eng->DrawBitmap(m_BmpMoneyDisplayPtr);
    eng->set_color(COLOR(255, 255, 255));
    eng->DrawString(String(amountOfGold), (int)(bitmapWidth - 30), (int)(5 ));
    eng->DrawString(String(amountOfSilver), (int)(bitmapWidth - 70), (int)(5 ));
    eng->DrawString(String(amountOfCopper), (int)(bitmapWidth - 110), (int)( 5 ));
    eng->set_color(COLOR(0, 0, 0));
    eng->set_font(m_FntPtr);
    eng->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());

}

void HUD::PaintDeathCounterWindow(DOUBLE2 position)
{
    MATRIX3X2 matTranslate, matPivot;
    DOUBLE2 positionIcon = DOUBLE2(-m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2);
    matTranslate.SetAsTranslate(position);
    matPivot.SetAsTranslate(positionIcon);

    GameEngine* eng = GameEngine::instance();
    eng->set_world_matrix(matPivot * matTranslate * GameEngine::instance()->get_view_matrix().Inverse());
    eng->DrawBitmap(m_BmpDeathIconPtr);

    matPivot.SetAsTranslate(DOUBLE2(20 + m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2));
    eng->set_world_matrix(matPivot * matTranslate * GameEngine::instance()->get_view_matrix().Inverse());
    eng->DrawString(String(m_AvatarPtr->GetDeaths()),DOUBLE2());
    eng->set_world_matrix(MATRIX3X2::CreateIdentityMatrix());
}
void HUD::CreatePauseMenu()
{
    int dy = 30;
    m_BtnQuitGamePtr = new Button(String("Quit to desktop!"));
    m_BtnQuitGamePtr->SetBounds(GameEngine::instance()->get_width()/2 - 100, GameEngine::instance()->get_height() / 2, 200, 25);
    m_BtnQuitToMenuPtr = new Button(String("Quit to main menu!"));
    m_BtnQuitToMenuPtr->SetBounds(GameEngine::instance()->get_width()/2 - 100, GameEngine::instance()->get_height() / 2 - dy, 200, 25);
    
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
    GameEngine::instance()->set_world_matrix(matPivot*matTranslate * GameEngine::instance()->get_view_matrix().Inverse());
    GameEngine::instance()->set_color(COLOR(0, 0, 0, m_GameOverOpacity));
    GameEngine::instance()->DrawBitmap(m_BmpGameOverPtr);
    m_IsGameOverDrawn = true;
    GameEngine::instance()->set_world_matrix(GameEngine::instance()->get_view_matrix().Inverse());
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
    Bitmap* tmpBitmapPressed = BitmapManager::instance()->load_image(String("Resources/UI/audioOn.png"));
    m_BtnSndMutePtr = new Button(String("Mute Sound"));
    m_BtnSndMutePtr->SetBounds(GameEngine::instance()->get_width() - 60, GameEngine::instance()->get_height() - 60, tmpBitmapPressed->GetWidth(), tmpBitmapPressed->GetHeight());
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
            GameEngine::instance()->DrawString(String("Lever ") + String(i) + String(" is hit!"), 10, 150 + (int)i * 15);
        }
        else
        {
            GameEngine::instance()->DrawString(String("Lever ") + String(i) + String(" is not hit!"), 10, 150 + (int)i * 15);
        }
    }
}