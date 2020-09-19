#include "stdafx.h"		
#include "EngineFiles/TextLabel.h"
	
#include "StartMenu.h"
#include "ejj_data_manager.h"
#include "ElectronicJonaJoy.h"
#include "SoundManager.h"
#include <winuser.h>

StartMenu::StartMenu()
{
	m_ComicSansPtr = std::make_shared<Font>(String("Comic Sans MS"), (float)KEYLISTFNTSIZE);
    m_BgMusicPtr = sound_manager::instance()->LoadMusic(String("Resources/Sound/BgMusic/main.mp3"));
    m_BgMusicPtr->play();
    EnableButtons();
    m_FileManagerPtr->ReadGameResults(m_SessionStatsArr);

}

StartMenu::~StartMenu()
{
    for (size_t i = 0; i < m_SessionStatsArr.size(); i++)
    {
        delete m_SessionStatsArr[i];
    }
    m_SessionStatsArr.clear();
    Remove();
}

void StartMenu::Tick(double deltaTime)
{
    // This switch statement handles the changing of menu's
    switch (m_MenuState)
    {
    case StartMenu::menuState::MAIN:
        if (m_BtnOptionsPtr != nullptr && m_BtnOptionsPtr->IsPressed())
        {
            m_MenuState = menuState::OPTIONS;
            delete m_BtnStartPtr;
            m_BtnStartPtr = nullptr;
            delete m_BtnQuitPtr;
            m_BtnQuitPtr = nullptr;
            delete m_BtnHighScoresPtr;
            m_BtnHighScoresPtr = nullptr;
            m_BtnOptionsPtr->SetText(String("Back"));

            
            m_BtnApplyChangesPtr = new Button(String("Apply"));
            m_BtnApplyChangesPtr->SetBounds(game_engine::instance()->get_width()/2 - 50, game_engine::instance()->get_height()/2 +40, 100, 30);
            m_BtnApplyChangesPtr->SetEnabled(false);
            CreateKeyBindTextBoxes();
        }
        if (m_BtnHighScoresPtr != nullptr && m_BtnHighScoresPtr->IsPressed())
        {
            m_MenuState = menuState::HIGHSCORES;
            delete m_BtnStartPtr;
            m_BtnStartPtr = nullptr;
            delete m_BtnQuitPtr;
            m_BtnQuitPtr = nullptr;
            delete m_BtnOptionsPtr;
            m_BtnOptionsPtr = nullptr;

            for (size_t i = 0; i < m_SessionStatsArr.size(); i++)
            {
                delete m_SessionStatsArr[i];
            }
            m_SessionStatsArr.clear();
            m_FileManagerPtr->ReadGameResults(m_SessionStatsArr);
            m_BtnHighScoresPtr->SetText(String("Back"));


            

        }

        if (m_BtnQuitPtr && m_BtnQuitPtr->IsPressed())
        {
            game_engine::instance()->quit_game();
        }

        if (m_BtnStartPtr && m_BtnStartPtr->IsPressed())
        {
            if(_on_start_event) _on_start_event();
        }
        break;
    case StartMenu::menuState::OPTIONS:
        if (m_BtnOptionsPtr->IsPressed())
        {
            m_MenuState = menuState::MAIN;
            for (int i = 0; i < m_TxtInputKeyPtrArr.size(); i++)
            {
                delete m_TxtInputKeyPtrArr[i];
                delete m_TxtLabels[i];
            }
            m_TxtInputKeyPtrArr.clear();
            m_TxtLabels.clear();
            m_ActionToTxtBoxMappings.clear();
            m_BtnOptionsPtr->SetText(String("Options"));

            delete m_BtnApplyChangesPtr;
            m_BtnApplyChangesPtr = nullptr;
            EnableButtons();

        }
        if (m_BtnApplyChangesPtr != nullptr && m_BtnApplyChangesPtr->IsPressed())
        {
			for (auto it = m_ActionToTxtBoxMappings.begin(); it != m_ActionToTxtBoxMappings.end(); ++it)
			{
                std::string action = it->first;
                String k = it->second->GetText();
                UINT32 key_code = k.ToInteger();
				{
                    m_KeybindsArr[action] = key_code;
				}
			}
            m_FileManagerPtr->SetKeyBinds(m_KeybindsArr,ElectronicJonaJoy::CONFIGPATH);
            m_BtnApplyChangesPtr->SetEnabled(false);
        }

		for (auto it = m_ActionToTxtBoxMappings.begin(); it != m_ActionToTxtBoxMappings.end(); ++it)
        {
            if (it->second->GetText() != String(m_KeybindsArr[it->first]))
            {
                if (m_IsOptionAdjusted == false)
                {
                    m_IsOptionAdjusted = true;
                    m_BtnApplyChangesPtr->SetEnabled(true);
                }
                else
                {
                    m_IsOptionAdjusted = false;
                }
                
            }
            if (it->second->IsFocus())
            {
                for (int j = 0; j < 250 + 1; j++)
                {
                    if (game_engine::instance()->IsKeyboardKeyPressed(j))
                    {
                        if (j >= VK_F1 && j <= VK_F12)
                        {
                            it->second->SetText(String(j));
                        }
                        else if (j > 32 && j < 127)
                        {
                            it->second->SetText(String(UINT32(j)));
                        }
                        else
                        {
                            it->second->SetText(String(j));
                        }
                    }
                }
            }
        }
        break;
    case menuState::HIGHSCORES:
        // Using the same button only changed name
        if (m_BtnHighScoresPtr->IsPressed())
        {
            m_MenuState = menuState::MAIN;
            m_BtnHighScoresPtr->SetText(String("Stats"));
            EnableButtons();
            
        }
        if (game_engine::instance()->IsKeyboardKeyPressed(VK_DOWN))
        {
            if (m_HighScoreOffsetCounter < m_SessionStatsArr.size() - MAXELEMENTSONSCREEN && m_HighScoreOffsetCounter >= 0)
            {
                m_HighScoreOffsetCounter++;
            }
            
        }
        if (game_engine::instance()->IsKeyboardKeyPressed(VK_UP))
        {
            if (m_HighScoreOffsetCounter > 0)
            {
                m_HighScoreOffsetCounter--;
            }
            
        }
        break;
    default:
        break;
    }
}

void StartMenu::Paint()
{
    game_engine::instance()->DrawSolidBackground(COLOR(116, 202, 141));
    switch (m_MenuState)
    {
    case StartMenu::menuState::HIGHSCORES:
        for (size_t i = 0, n = m_SessionStatsArr.size(); i < m_SessionStatsArr.size(); i++)
        {
            int index = int(i) + m_HighScoreOffsetCounter;
            if (i < MAXELEMENTSONSCREEN && index < m_SessionStatsArr.size())
            {
                if (m_HighScoreOffsetCounter < m_SessionStatsArr.size() - MAXELEMENTSONSCREEN)
                {
                    game_engine::instance()->DrawString(String("v"), DOUBLE2(100, 430));
                }
                if (m_HighScoreOffsetCounter >0)
                {
                    game_engine::instance()->DrawString(String("^"), DOUBLE2(100, 120));
                }
                Font* tmpFntPtr = new Font(String("KenVector Future"), (float)KEYLISTFNTSIZE - 8);
                game_engine::instance()->set_font(tmpFntPtr);
                game_engine::instance()->DrawString(String("id"), DOUBLE2(INDEXSTARTXPOS, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("Start Session"), DOUBLE2(DATESTARTPOS + 75, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("End Session"),DOUBLE2(DATEENDPOS+75, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("Deaths"), DOUBLE2(DEATHPOS, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("Money"), DOUBLE2(MONEYPOS, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("Lifetime"), DOUBLE2(LIFETIMEPOS, TABLELEGENDYPOS));
                game_engine::instance()->DrawString(String("Last level"), DOUBLE2(LASTLEVELPOS, TABLELEGENDYPOS));

                
                int halfScreenWidth = game_engine::instance()->get_width() / 2;
                double gap = 50;
                
                tm beginTime = m_SessionStatsArr[index]->beginTime;
                tm endTime = m_SessionStatsArr[index]->endTime;
                int deaths = m_SessionStatsArr[index]->amountOfDeaths;
                int amountOfMoney = m_SessionStatsArr[index]->money;
                int lastLevel = m_SessionStatsArr[index]->lastLevel;
                double lifeTime = m_SessionStatsArr[index]->lifeTime;

                std::wstringstream compoundString;
                compoundString << index << L".";
                game_engine::instance()->DrawString(String(compoundString.str().c_str()),DOUBLE2(INDEXSTARTXPOS,INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");
                
                compoundString << L"\t" << beginTime.tm_mday << L"/" << beginTime.tm_mon << L"\t" << beginTime.tm_hour << ":" << beginTime.tm_min << ":" << beginTime.tm_sec;
                game_engine::instance()->DrawString(String(compoundString.str().c_str()), DOUBLE2(DATESTARTPOS, INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");

                compoundString << L"\t" << endTime.tm_mday << L"/" << endTime.tm_mon << L"\t" << endTime.tm_hour << ":" << endTime.tm_min << ":" << endTime.tm_sec;
                game_engine::instance()->DrawString(String(compoundString.str().c_str()), DOUBLE2(DATEENDPOS, INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");

                game_engine::instance()->DrawString(String(deaths),DOUBLE2(DEATHPOS,INDEXSTARTYPOS + double(i)*gap));
                game_engine::instance()->DrawString(String(amountOfMoney), DOUBLE2(MONEYPOS, INDEXSTARTYPOS + double(i)*gap));
                game_engine::instance()->DrawString(String(lifeTime), DOUBLE2(LIFETIMEPOS, INDEXSTARTYPOS + double(i)*gap));
                game_engine::instance()->DrawString(String(lastLevel), DOUBLE2(LASTLEVELPOS, INDEXSTARTYPOS + double(i)*gap));
                game_engine::instance()->set_default_font();
                delete tmpFntPtr;
            }
            
        }
        break;
    case StartMenu::menuState::MAIN:
        if (m_BmpPtr != nullptr)
        {
            game_engine::instance()->set_font(m_ComicSansPtr.get());
            MATRIX3X2 matTranslateControl;
            matTranslateControl.SetAsTranslate(100, CONTROLLISTYPOS - (float)KEYLISTFNTSIZE);
            game_engine::instance()->SetWorldMatrix(matTranslateControl);
            game_engine::instance()->DrawString(String("Controls: "), DOUBLE2());
            game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
            int i = 0;
            for(auto& keybind : m_KeybindsArr)
            {
                MATRIX3X2 matTranslate;
                matTranslate.SetAsTranslate(DOUBLE2(150, CONTROLLISTYPOS + KEYLISTFNTSIZE + i*(float)KEYLISTFNTSIZE));
                game_engine::instance()->SetWorldMatrix(matTranslate);
                String KeyBind = String(keybind.second);
                switch (keybind.second)
                {
                case 37:
                    KeyBind = String("Left Arrow");
                    break;
                case 39:
                    KeyBind = String("Right Arrow");
                    break;
                default:
                    String(TCHAR(keybind.second));
                    break;
                }
                game_engine::instance()->DrawString(String(keybind.first.c_str()) + String(": ") + KeyBind, DOUBLE2());
                game_engine::instance()->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());

                ++i;
            }
            game_engine::instance()->set_default_font();
        }
        break;
    }
    
        
    
}

void StartMenu::Remove()
{
    m_BgMusicPtr->stop();
    delete m_BtnQuitPtr;
    m_BtnQuitPtr = nullptr;
    delete m_BtnStartPtr;
    m_BtnStartPtr = nullptr;
    delete m_BtnOptionsPtr;
    m_BtnOptionsPtr = nullptr;
    delete m_BtnHighScoresPtr;
    m_BtnHighScoresPtr = nullptr;

    for (size_t i = 0; i < m_TxtInputKeyPtrArr.size(); i++)
    {
        delete m_TxtInputKeyPtrArr[i];
        delete m_TxtLabels[i];
    }
    m_TxtInputKeyPtrArr.clear();
    m_TxtLabels.clear();
    m_ActionToTxtBoxMappings.clear();

    delete m_BtnApplyChangesPtr;
    m_BtnApplyChangesPtr = nullptr;
    delete m_BmpPtr;
    m_BmpPtr = nullptr;

}

void StartMenu::EnableButtons()
{
    float width = game_engine::instance()->get_viewport_size().x;
    float height = game_engine::instance()->get_viewport_size().y;
    float spacing = 1.25;
    int buttonHeight = 100;
    int buttonWidth = 100;

    if (m_BmpPtr == nullptr)
    {
        m_BmpPtr = new Bitmap(String("Resources/Menu/StartMenu.png"));
    }
    if (m_BtnQuitPtr == nullptr)
    {
        TextureButton* btn = new TextureButton("Resources/UI/Button_QuitGame_Pressed.png", "Resources/UI/Button_QuitGame_Released.png");
        m_BtnQuitPtr = btn;
        buttonWidth = btn->GetWidth() ;
        buttonHeight = btn->GetHeight() ;
        m_BtnQuitPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2), buttonWidth, buttonHeight);
    }
    if (m_BtnStartPtr == nullptr)
    {
        m_BtnStartPtr = new TextureButton("Resources/UI/Button_StartGame_Pressed.png", "Resources/UI/Button_StartGame_Released.png");
        m_BtnStartPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) - spacing * buttonHeight, buttonWidth, buttonHeight);
    }
    
    if (m_BtnOptionsPtr == nullptr)
    {
        m_BtnOptionsPtr = new TextureButton("Resources/UI/Button_Options_Pressed.png", "Resources/UI/Button_Options_Released.png");
        m_BtnOptionsPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) + spacing * buttonHeight, buttonWidth, buttonHeight);
    }
    if (m_BtnHighScoresPtr == nullptr)
    {
        m_BtnHighScoresPtr = new TextureButton("Resources/UI/Button_HighScores_Pressed.png", "Resources/UI/Button_HighScores_Released.png");
        m_BtnHighScoresPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) + 2 *spacing * buttonHeight, buttonWidth, buttonHeight);
    }

}

void StartMenu::ReadKeyBindsForMenu(ejj_data_manager::KeyMap tmpKeybindsArr)
{
    m_KeybindsArr = tmpKeybindsArr;
}
void StartMenu::CreateKeyBindTextBoxes()
{
    int buttonHeight = 40;
    int buttonWidth = 40;
    int width = game_engine::instance()->get_width();
    int height = game_engine::instance()->get_height();

    int amountOfBoxesNeeded = int(m_KeybindsArr.size());
    int boundingHeight = 250;
    double positionsOfEachBox = (double)boundingHeight / amountOfBoxesNeeded;
    int i = 0;
    for (auto it = m_KeybindsArr.begin(); it != m_KeybindsArr.end(); ++it)
    {
        TextLabel* tmpLabel = new TextLabel(it->first.c_str());
        tmpLabel->SetBounds((int)( (width/2) - (buttonWidth + 100)), (int)(150 + positionsOfEachBox*i), (int)buttonWidth+ 100, (int)buttonHeight);
        m_TxtLabels.push_back(tmpLabel);

        TextBox* tmpTextBox = new TextBox(String(it->second));
        tmpTextBox->SetBackColor(COLOR(255, 255, 255, 0));

        tmpTextBox->SetBounds((int)(width / 2 - buttonWidth/2), (int)(150 + positionsOfEachBox*i), (int)buttonWidth, (int)buttonHeight);
        m_TxtInputKeyPtrArr.push_back(tmpTextBox);
        m_ActionToTxtBoxMappings[it->first] = tmpTextBox;

        ++i;
    }
}

void StartMenu::SetFileManager(ejj_data_manager* tmpFileManagerPtr)
{
    m_FileManagerPtr = tmpFileManagerPtr;
}

void StartMenu::ReadGameResults()
{
    for (int i = 0, n = int(m_SessionStatsArr.size()); i < n; i++)
    {
        delete m_SessionStatsArr[i];
    }
    m_SessionStatsArr.clear();
    m_FileManagerPtr->ReadGameResults(m_SessionStatsArr);
}