#pragma once

#include "FileManager.h"

class FileManager;

class StartMenu 
{
public:
	static const int MAXELEMENTSONSCREEN = 5;
	static const int TABLELEGENDYPOS = 150;
	static const int INDEXSTARTXPOS = 150;
	static const int DATESTARTPOS = 180;
	static const int DATEENDPOS = 370;
	static const int DEATHPOS = 650;
	static const int MONEYPOS = 780;
	static const int LIFETIMEPOS = 900;
	static const int LASTLEVELPOS = 1100;
	static const int INDEXSTARTYPOS = 200;

	StartMenu();
	virtual ~StartMenu( );

	// C++11 make the class non-copyable
	StartMenu( const StartMenu& ) = delete;
	StartMenu& operator=( const StartMenu& ) = delete;

    void Paint();
    void Tick(double deltaTime);
    void EnableButtons();
    void Remove();
    void ReadGameResults();
    void ReadKeyBindsForMenu(FileManager::KeyMap tmpKeybindsArr);
    void SetFileManager(FileManager* fileManagerPtr);

    std::function<void()> _on_start_event;
private: 
    
    void CreateKeyBindTextBoxes();

private:
    enum class menuState
    {
        MAIN,
        OPTIONS,
        HIGHSCORES
    };
    int m_HighScoreOffsetCounter = 0;
    std::vector<FileManager::sessionStats*>m_SessionStatsArr;
    menuState m_MenuState = menuState::MAIN;
    static const int KEYLISTFNTSIZE = 25;
    static const int CONTROLLISTYPOS = 230;
    Bitmap* m_BmpPtr = nullptr;
    Button* m_BtnStartPtr = nullptr;
    Button* m_BtnQuitPtr = nullptr;
    Button* m_BtnHighScoresPtr = nullptr;
    Button* m_BtnOptionsPtr = nullptr;
    Button* m_BtnApplyChangesPtr = nullptr;

    std::map<std::string, class TextBox*> m_ActionToTxtBoxMappings;
    std::vector<class TextBox*>m_TxtInputKeyPtrArr;
    std::vector<class TextLabel*>m_TxtLabels;

    bool m_IsOptionAdjusted = false;
    Sound* m_BgMusicPtr = nullptr;
    FileManager* m_FileManagerPtr = nullptr;
    FileManager::KeyMap m_KeybindsArr;

    std::shared_ptr<Font> m_ComicSansPtr = nullptr;

};

 
