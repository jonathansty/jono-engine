//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE5
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "StartMenu.h"
#include "FileManager.h"
#include "conio.h"

//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())

//---------------------------
// Constructor & Destructor
//---------------------------
StartMenu::StartMenu()
{
    m_BgMusicPtr = SoundManager::GetSingleton()->LoadMusic(String("Resources/Sound/BgMusic/main.mp3"));
    m_BgMusicPtr->Play();
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
            m_BtnApplyChangesPtr->SetBounds(GAME_ENGINE->GetWidth()/2 - 50, GAME_ENGINE->GetHeight()/2 +40, 100, 30);
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
        break;
    case StartMenu::menuState::OPTIONS:
        if (m_BtnOptionsPtr->IsPressed())
        {
            m_MenuState = menuState::MAIN;
            for (int i = 0; i < m_TxtInputKeyPtrArr.size(); i++)
            {
                delete m_TxtInputKeyPtrArr[i];
            }
            m_TxtInputKeyPtrArr.clear();
            m_BtnOptionsPtr->SetText(String("Options"));

            delete m_BtnApplyChangesPtr;
            m_BtnApplyChangesPtr = nullptr;
            EnableButtons();

        }
        if (m_BtnApplyChangesPtr != nullptr && m_BtnApplyChangesPtr->IsPressed())
        {
            for (int i = 0; i < m_TxtInputKeyPtrArr.size(); i++)
            {
            String tmpInput = m_TxtInputKeyPtrArr[i]->GetText();
            TCHAR keyBind = String(tmpInput).At(0);
            if (tmpInput.ToInteger() != 0)
            {
                keyBind = String(tmpInput).ToInteger();
            }

            m_KeybindsArr[i].second = keyBind;
            }
            m_FileManagerPtr->SetKeyBinds(m_KeybindsArr,String("Resources/cfg/config.txt"));
            m_BtnApplyChangesPtr->SetEnabled(false);
        }

        for (int i = 0; i < m_TxtInputKeyPtrArr.size(); i++)
        {
            if (m_TxtInputKeyPtrArr[i]->GetText() != String(m_KeybindsArr[i].second))
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
            if (m_TxtInputKeyPtrArr[i]->IsFocus())
            {
                for (int j = 0; j < 250 + 1; j++)
                {
                    
                    if (GAME_ENGINE->IsKeyboardKeyPressed(j))
                    {
                        if (j >= VK_F1 && j <= VK_F12)
                        {
                            m_TxtInputKeyPtrArr[i]->SetText(String(j));
                        }
                        else if (j > 32 && j < 127)
                        {
                            m_TxtInputKeyPtrArr[i]->SetText(String(TCHAR(j)));
                        }
                        else
                        {
                            m_TxtInputKeyPtrArr[i]->SetText(String(j));
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
        if (GAME_ENGINE->IsKeyboardKeyPressed(VK_DOWN))
        {
            if (m_HighScoreOffsetCounter < m_SessionStatsArr.size() - MAXELEMENTSONSCREEN && m_HighScoreOffsetCounter >= 0)
            {
                m_HighScoreOffsetCounter++;
            }
            
        }
        if (GAME_ENGINE->IsKeyboardKeyPressed(VK_UP))
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
    GAME_ENGINE->DrawBitmap(m_BmpPtr);
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
                    GAME_ENGINE->DrawString(String("v"), DOUBLE2(100, 430));
                }
                if (m_HighScoreOffsetCounter >0)
                {
                    GAME_ENGINE->DrawString(String("^"), DOUBLE2(100, 120));
                }
                Font* tmpFntPtr = new Font(String("KenVector Future"), (float)KEYLISTFNTSIZE - 8);
                GAME_ENGINE->SetFont(tmpFntPtr);
                GAME_ENGINE->DrawString(String("id"), DOUBLE2(INDEXSTARTXPOS, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("Start Session"), DOUBLE2(DATESTARTPOS + 75, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("End Session"),DOUBLE2(DATEENDPOS+75, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("Deaths"), DOUBLE2(DEATHPOS, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("Money"), DOUBLE2(MONEYPOS, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("Lifetime"), DOUBLE2(LIFETIMEPOS, TABLELEGENDYPOS));
                GAME_ENGINE->DrawString(String("Last level"), DOUBLE2(LASTLEVELPOS, TABLELEGENDYPOS));

                
                int halfScreenWidth = GAME_ENGINE->GetWidth() / 2;
                double gap = 50;
                
                tm beginTime = m_SessionStatsArr[index]->beginTime;
                tm endTime = m_SessionStatsArr[index]->endTime;
                int deaths = m_SessionStatsArr[index]->amountOfDeaths;
                int amountOfMoney = m_SessionStatsArr[index]->money;
                int lastLevel = m_SessionStatsArr[index]->lastLevel;
                double lifeTime = m_SessionStatsArr[index]->lifeTime;

                std::wstringstream compoundString;
                compoundString << index << L".";
                GAME_ENGINE->DrawString(String(compoundString.str().c_str()),DOUBLE2(INDEXSTARTXPOS,INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");
                
                compoundString << L"\t" << beginTime.tm_mday << L"/" << beginTime.tm_mon << L"\t" << beginTime.tm_hour << ":" << beginTime.tm_min << ":" << beginTime.tm_sec;
                GAME_ENGINE->DrawString(String(compoundString.str().c_str()), DOUBLE2(DATESTARTPOS, INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");

                compoundString << L"\t" << endTime.tm_mday << L"/" << endTime.tm_mon << L"\t" << endTime.tm_hour << ":" << endTime.tm_min << ":" << endTime.tm_sec;
                GAME_ENGINE->DrawString(String(compoundString.str().c_str()), DOUBLE2(DATEENDPOS, INDEXSTARTYPOS + double(i*gap)));
                compoundString.str(L"");

                GAME_ENGINE->DrawString(String(deaths),DOUBLE2(DEATHPOS,INDEXSTARTYPOS + double(i)*gap));
                GAME_ENGINE->DrawString(String(amountOfMoney), DOUBLE2(MONEYPOS, INDEXSTARTYPOS + double(i)*gap));
                GAME_ENGINE->DrawString(String(lifeTime), DOUBLE2(LIFETIMEPOS, INDEXSTARTYPOS + double(i)*gap));
                GAME_ENGINE->DrawString(String(lastLevel), DOUBLE2(LASTLEVELPOS, INDEXSTARTYPOS + double(i)*gap));
                GAME_ENGINE->SetDefaultFont();
                delete tmpFntPtr;
            }
            
        }
        break;
    case StartMenu::menuState::MAIN:
        if (m_BmpPtr != nullptr)
        {
            Font* tmpFntPtr = new Font(String("KenVector Future"), (float)KEYLISTFNTSIZE);
            GAME_ENGINE->SetFont(tmpFntPtr);
            MATRIX3X2 matTranslateControl;
            matTranslateControl.SetAsTranslate(100, CONTROLLISTYPOS - (float)KEYLISTFNTSIZE);
            GAME_ENGINE->SetWorldMatrix(matTranslateControl);
            GAME_ENGINE->DrawString(String("Controls: "), DOUBLE2());
            GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
            for (size_t i = 0; i < m_KeybindsArr.size(); i++)
            {
                MATRIX3X2 matTranslate;
                matTranslate.SetAsTranslate(DOUBLE2(150, CONTROLLISTYPOS + KEYLISTFNTSIZE + i*(float)KEYLISTFNTSIZE));
                GAME_ENGINE->SetWorldMatrix(matTranslate);
                String KeyBind = String(m_KeybindsArr[i].second);
                switch (m_KeybindsArr[i].second)
                {
                case 37:
                    KeyBind = String("Left Arrow");
                    break;
                case 39:
                    KeyBind = String("Right Arrow");
                    break;
                default:
                    String(m_KeybindsArr[i].second);
                    break;
                }
                GAME_ENGINE->DrawString(m_KeybindsArr[i].first + String(": ") + KeyBind, DOUBLE2());
                GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());


            }
            GAME_ENGINE->SetDefaultFont();
            delete tmpFntPtr;
        }
        break;
    case StartMenu::menuState::OPTIONS:
        int buttonHeight = 40;
        int buttonWidth = 200;
        int widthScreen = GAME_ENGINE->GetWidth();
        int height = GAME_ENGINE->GetHeight();

        int amountOfBoxesNeeded = int(m_KeybindsArr.size());
        int boundingHeight = 250;
        double positionsOfEachBox = (double)boundingHeight / amountOfBoxesNeeded;
        Font* tmpFntPtr = new Font(String("KenVector Future"), KEYLISTFNTSIZE);
        GAME_ENGINE->SetFont(tmpFntPtr);
        for (int i = 0; i < amountOfBoxesNeeded; i++)
        {
            MATRIX3X2 matTranslate;
            matTranslate.SetAsTranslate(DOUBLE2(widthScreen / 2 - buttonWidth - buttonWidth / 8, 100 + buttonHeight*1.5 + i*positionsOfEachBox));
            GAME_ENGINE->SetWorldMatrix(matTranslate);
            GAME_ENGINE->DrawString(m_KeybindsArr[i].first,DOUBLE2());
            GAME_ENGINE->SetWorldMatrix(MATRIX3X2::CreateIdentityMatrix());
        }
        GAME_ENGINE->SetDefaultFont();
        delete tmpFntPtr;
        break;
    }
    
        
    
}
bool StartMenu::startPressed()
{
    if (m_BtnStartPtr != nullptr)
    {
        if (m_BtnStartPtr->IsPressed())
        {
            Remove();
            return true;
        }
    }
    return false;
}
bool StartMenu::quitPressed()
{
    if (m_BtnQuitPtr != nullptr)
    {
        if (m_BtnQuitPtr->IsPressed())
        {
            Remove();
            return true;
        }
    }
    return false;
}
void StartMenu::Remove()
{
    m_BgMusicPtr->Stop();
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
    }
    m_TxtInputKeyPtrArr.clear();

    delete m_BtnApplyChangesPtr;
    m_BtnApplyChangesPtr = nullptr;
    delete m_BmpPtr;
    m_BmpPtr = nullptr;

}
void StartMenu::EnableButtons()
{
    int buttonHeight = 40;
    int buttonWidth = 200;
    int width = GAME_ENGINE->GetWidth();
    int height = GAME_ENGINE->GetHeight();
    if (m_BmpPtr == nullptr)
    {
        m_BmpPtr = new Bitmap(String("Resources/Menu/StartMenu.png"));
    }
    if (m_BtnQuitPtr == nullptr)
    {
        m_BtnQuitPtr = new Button(String("Quit Game"));
        m_BtnQuitPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2), buttonWidth, buttonHeight);
        
    }
    if (m_BtnStartPtr == nullptr)
    {
        m_BtnStartPtr = new Button(String("Start Game"));
        m_BtnStartPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) - 2 * buttonHeight, buttonWidth, buttonHeight);
    }
    
    if (m_BtnOptionsPtr == nullptr)
    {
        m_BtnOptionsPtr = new Button(String("Options"));
        m_BtnOptionsPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) + 2 * buttonHeight, buttonWidth, buttonHeight);
    }
    if (m_BtnHighScoresPtr == nullptr)
    {
        m_BtnHighScoresPtr = new Button(String("Stats"));
        m_BtnHighScoresPtr->SetBounds(-buttonWidth / 2 + width / 2, (height / 2) + 4 * buttonHeight, buttonWidth, buttonHeight);
    }

}

void StartMenu::ReadKeyBindsForMenu(std::vector<std::pair<String, TCHAR>> tmpKeybindsArr)
{
    m_KeybindsArr = tmpKeybindsArr;
}
void StartMenu::CreateKeyBindTextBoxes()
{
    int buttonHeight = 40;
    int buttonWidth = 40;
    int width = GAME_ENGINE->GetWidth();
    int height = GAME_ENGINE->GetHeight();

    int amountOfBoxesNeeded = int(m_KeybindsArr.size());
    int boundingHeight = 250;
    double positionsOfEachBox = (double)boundingHeight / amountOfBoxesNeeded;
    for (size_t i = 0; i < m_KeybindsArr.size(); i++)
    {
        TextBox* tmpTextBox = new TextBox(String(m_KeybindsArr[i].second));
        tmpTextBox->SetBackColor(COLOR(255, 255, 255, 0));

        tmpTextBox->SetBounds((int)(width / 2 - buttonWidth/2), (int)(150 + positionsOfEachBox*i), (int)buttonWidth, (int)buttonHeight);
        m_TxtInputKeyPtrArr.push_back(tmpTextBox);
    }
}
std::vector<std::pair<String, TCHAR>> StartMenu::GetKeyBinds()
{
    return m_KeybindsArr;
}
void StartMenu::SetFileManager(FileManager* tmpFileManagerPtr)
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