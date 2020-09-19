#include "stdafx.h"		
	
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
    m_BmpMoneyDisplayPtr = bitmap_manager::instance()->LoadBitmapFile(String("Resources/UI/ui_MoneyDisplay.png"));
    m_BmpDeathIconPtr = bitmap_manager::instance()->LoadBitmapFile(String("Resources/UI/ui_DeathIcon.png"));
    m_BmpGameOverPtr = bitmap_manager::instance()->LoadBitmapFile(String("Resources/Menu/gameOver.png"));
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
    game_engine::instance()->set_font(m_FntPtr);
    PaintMoneyWindow(DOUBLE2(10, 10));
    PaintDeathCounterWindow(DOUBLE2(game_engine::instance()->get_width() - 100, 50));
    
    game_engine::instance()->SetWorldMatrix(game_engine::instance()->GetViewMatrix().Inverse());
    game_engine::instance()->DrawString(String(m_AccuTime, 2), game_engine::instance()->get_width() / 2, 10);
    game_engine::instance()->set_default_font(); 
    
    PaintLeverInfo();
    game_engine::instance()->SetViewMatrix(game_engine::instance()->GetViewMatrix());
    
}
void HUD::Tick(double deltaTime)
{

    XMFLOAT2 mousePosition = game_engine::instance()->get_mouse_pos_in_viewport();
    if (m_BtnSndMutePtr != nullptr && m_BtnSndMutePtr->IsPressed())
    {
        if (m_Muted)
        {
            Bitmap* tmpBitmapPressed = bitmap_manager::instance()->LoadBitmapFile(String("Resources/UI/audioOn.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            sound_manager::instance()->UnMuteAll();
        }
        else
        {
            Bitmap* tmpBitmapPressed = bitmap_manager::instance()->LoadBitmapFile(String("Resources/UI/audioOff.png"));
            m_BtnSndMutePtr->SetPressedBitmap(tmpBitmapPressed);
            m_BtnSndMutePtr->SetReleasedBitmap(tmpBitmapPressed);
            sound_manager::instance()->MuteAll();
        }
        m_Muted = !m_Muted;
    }


    if (m_BtnQuitGamePtr!= nullptr && m_BtnQuitGamePtr->IsPressed())
    {
        game_engine::instance()->quit_game();
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
    game_engine::instance()->set_default_font();
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

    game_engine* eng = game_engine::instance();
    eng->SetWorldMatrix(matTranslate * game_engine::instance()->GetViewMatrix().Inverse());
    eng->DrawBitmap(m_BmpMoneyDisplayPtr);
    eng->set_color(COLOR(255, 255, 255));
    eng->DrawString(String(amountOfGold), (int)(bitmapWidth - 30), (int)(5 ));
    eng->DrawString(String(amountOfSilver), (int)(bitmapWidth - 70), (int)(5 ));
    eng->DrawString(String(amountOfCopper), (int)(bitmapWidth - 110), (int)( 5 ));
    eng->set_color(COLOR(0, 0, 0));
    eng->set_font(m_FntPtr);
    eng->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());

}

void HUD::PaintDeathCounterWindow(DOUBLE2 position)
{
    MATRIX3X2 matTranslate, matPivot;
    DOUBLE2 positionIcon = DOUBLE2(-m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2);
    matTranslate.SetAsTranslate(position);
    matPivot.SetAsTranslate(positionIcon);

    game_engine* eng = game_engine::instance();
    eng->SetWorldMatrix(matPivot * matTranslate * game_engine::instance()->GetViewMatrix().Inverse());
    eng->DrawBitmap(m_BmpDeathIconPtr);

    matPivot.SetAsTranslate(DOUBLE2(20 + m_BmpDeathIconPtr->GetWidth() / 2, -m_BmpDeathIconPtr->GetHeight() / 2));
    eng->SetWorldMatrix(matPivot * matTranslate * game_engine::instance()->GetViewMatrix().Inverse());
    eng->DrawString(String(m_AvatarPtr->GetDeaths()),DOUBLE2());
    eng->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
}
void HUD::CreatePauseMenu()
{
    int dy = 30;
    m_BtnQuitGamePtr = new Button(String("Quit to desktop!"));
    m_BtnQuitGamePtr->SetBounds(game_engine::instance()->get_width()/2 - 100, game_engine::instance()->get_height() / 2, 200, 25);
    m_BtnQuitToMenuPtr = new Button(String("Quit to main menu!"));
    m_BtnQuitToMenuPtr->SetBounds(game_engine::instance()->get_width()/2 - 100, game_engine::instance()->get_height() / 2 - dy, 200, 25);
    
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
    game_engine::instance()->SetWorldMatrix(matPivot*matTranslate * game_engine::instance()->GetViewMatrix().Inverse());
    game_engine::instance()->set_color(COLOR(0, 0, 0, m_GameOverOpacity));
    game_engine::instance()->DrawBitmap(m_BmpGameOverPtr);
    m_IsGameOverDrawn = true;
    game_engine::instance()->SetWorldMatrix(game_engine::instance()->GetViewMatrix().Inverse());
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
    Bitmap* tmpBitmapPressed = bitmap_manager::instance()->LoadBitmapFile(String("Resources/UI/audioOn.png"));
    m_BtnSndMutePtr = new Button(String("Mute Sound"));
    m_BtnSndMutePtr->SetBounds(game_engine::instance()->get_width() - 60, game_engine::instance()->get_height() - 60, tmpBitmapPressed->GetWidth(), tmpBitmapPressed->GetHeight());
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
            game_engine::instance()->DrawString(String("Lever ") + String(i) + String(" is hit!"), 10, 150 + (int)i * 15);
        }
        else
        {
            game_engine::instance()->DrawString(String("Lever ") + String(i) + String(" is not hit!"), 10, 150 + (int)i * 15);
        }
    }
}