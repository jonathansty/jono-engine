#pragma once
//-----------------------------------------------------
// Name:
// First name:
// Group: 1DAE.
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
#include "FileManager.h"
//-----------------------------------------------------
// StartMenu Class									
//-----------------------------------------------------
class FileManager;

class StartMenu //: public ContactListener
{
public:
	StartMenu();
	virtual ~StartMenu( );

	// C++11 make the class non-copyable
	StartMenu( const StartMenu& ) = delete;
	StartMenu& operator=( const StartMenu& ) = delete;

	//--------------------------------------------------------
	// ContactListener overloaded member function declarations
	//--------------------------------------------------------
	//virtual void BeginContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr); 
	//virtual void EndContact(PhysicsActor *actThisPtr, PhysicsActor *actOtherPtr);   
	//virtual void ContactImpulse(PhysicsActor *actThisPtr, double impulse);
    void Paint();
    void Tick(double deltaTime);
    void EnableButtons();

    bool startPressed();
    bool quitPressed();
    void Remove();
    void ReadGameResults();
    void ReadKeyBindsForMenu(std::vector<std::pair<String, TCHAR>> tmpKeybindsArr);
    std::vector<std::pair<String,TCHAR>> GetKeyBinds();
    void SetFileManager(FileManager* fileManagerPtr);
private: 
    
    void CreateKeyBindTextBoxes();
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
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
    std::vector<TextBox*>m_TxtInputKeyPtrArr;
    bool m_IsOptionAdjusted = false;
    Sound* m_BgMusicPtr = nullptr;
    FileManager* m_FileManagerPtr = nullptr;
    std::vector<std::pair<String, TCHAR>>m_KeybindsArr;
};

 
