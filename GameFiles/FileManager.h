#pragma once
//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------

//#include "ContactListener.h"
//-----------------------------------------------------
// FileManager Class									
//-----------------------------------------------------
class Avatar;
class Entity;
class HUD;
class Enemy;
class Level;
class Camera;
class Arrow;
class BitmapManager;
class EnemyShooter;
class CheckPoint;
class RotLight;
class EnemyRotater;
class EnemyHorizontal;
class Coin;
class CoinList;
class AttackBeamList;
class EntityList;
class Teleport;
class CheckPointBg;
class EnemyList;
class EnemyRocketLauncher;
class EnemyRocket;
class Animation;
class AnimationList;
class CombRotLightCpBg;
class LevelEnd;
class TriggerList;
class LevelList;
class Game;

class FileManager //: public ContactListener
{
public:
	FileManager( );
	virtual ~FileManager( );

	// C++11 make the class non-copyable
	FileManager( const FileManager& ) = delete;
	FileManager& operator=( const FileManager& ) = delete;

    struct sessionStats
    {
        double  lifeTime = 0;
        int     amountOfDeaths = 0;
        int     money = 0;
        int     sessionNumber = 0;
        tm      beginTime;
        tm      endTime;
        int     lastLevel = 0;
    };

    //! Reads the level file
    void ReadGameInit(const String& filePath);
    //! Reads the level file only for certain objects
    void ReadGameInitForObject(const String& filePath, const String& objectName);
    //! Loads the background music
    void LoadGameMusic(const String& filePath);
    //! Loads all the avatar keybindings from the filePath
    //! Make sure the config file layout is correct.
    std::vector<std::pair<String, TCHAR>> LoadAvatarKeybinds(const String& filePath);
    //! Saves the game result in the file GameResults.txt
    void SaveGameResults(tm beginTime, tm endTime, Game* gamePtr);
    //! Reads the file GameResults.txt
    void ReadGameResults(std::vector<sessionStats*> &tmpSessionStatsArrPtr);
    //! Removes everything
    void RemoveAll();
    //! Removes all lists and clears them
    void ClearLists();
    //! Loads a list of levels from the filePath
    LevelList* LoadLevels(const String& filePath);

    //! Returns the avatar
    Avatar* GetAvatar();
    //! Returns the level
    Level* GetLevel();
    //! Returns the camera
    Camera* GetCamera();
    //! Returns the triggerList
    TriggerList* GetTriggers();
    //! Returns the levelEnd
    LevelEnd* GetLevelEnd();
    //! Returns the coinList
    CoinList* GetCoinList();
    //! Returns the attackBeamList
    AttackBeamList* GetAttackBeamList();
    //! Returns the EntityList
    EntityList* GetEntityList();
    //! Returns the EnemyList
    EnemyList* GetEnemyList();
    //! Returns the animationList
    AnimationList* GetAnimationList();
    //! Returns the background Music
    Sound* GetBgMusic();

    //! Returns the keybinds as a vector
    std::vector<std::pair<String, TCHAR>> GetKeyBinds();
    //! Sets the keysbinds for the avatar
    void FileManager::SetKeyBinds(std::vector < std::pair<String, TCHAR>> tmpKeyBindsArr, String filePath);

    //! Updates the m_LastLevel variable for use when saving and reading results
    void UpdateLastLevel(int lastLevel);

private: 
    //! Creates all the objects.
    void CreateObjects(const std::wstring& expressionStringRef);
    //! creates a specific object
    void CreateSpecificObject(const std::wstring& expressionStringRef, const String& objectName);
    //! Gets the value of a property
    std::wstring GetValue(const std::wstring& nameRef, const std::wstring& objectRef);
    //! Converts a string to double2
    DOUBLE2 StringToDouble2(const std::wstring& valueRef);
    //! Converts a string to bool
    bool StringToBool(const std::wstring& valueRef);
    //! Reads a start block
    void ReadGameResultsStart(std::wstring &expressionStringRef, sessionStats* tmpSessionStat);
    //! Read an end block
    void ReadGameResultsEnd(std::wstring &expressionStringRef, sessionStats* tmpSessionStat);
    //! Gets the value of a block
    std::wstring GetValueOfBlock(const std::wstring& nameRef, const std::wstring& objectRef);

    //Read File Methods
    void ExtractBgMusic(const std::wstring& expressionStringRef);
    void ExtractAvatar(const std::wstring& expressionStringRef);
    void ExtractLevel(const std::wstring& expressionStringRef);
    void ExtractLevelEnd(const std::wstring& expressionStringRef);
    void ExtractCamera(const std::wstring& expressionStringRef);
    void ExtractBlockSlide(const std::wstring& expressionStringRef);
    void ExtractGate(const std::wstring& expressionStringRef);
    void ExtractLaser(const std::wstring& expressionStringRef);
    void ExtractArrowShooter(const std::wstring& expressionStringRef);
    void ExtractArrow(const std::wstring& expressionStringRef);
    void ExtractEnemyShooter(const std::wstring& expressionStringRef);
    void ExtractCheckPoint(const std::wstring& expressionStringRef);
    void ExtractEnemyRotater(const std::wstring& expressionStringRef);
    void ExtractEnemyHorizontal(const std::wstring& expressionStringRef);
    void ExtractCoin(const std::wstring& expressionStringRef);
    void ExtractTeleport(const std::wstring& expressionStringRef);
    void ExtractEnemyRocketLauncher(const std::wstring& expressionStringRef);
    void ExtractMetalFan(const std::wstring& expressionStringRef);
    void ExtractStickyWall(const std::wstring& expressionStringRef);
    void ExtractSlicer(const std::wstring& expressionStringRef);
    void ExtractNpcHinter(const std::wstring& expressionStringRef);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
    Avatar* m_AvatarPtr = nullptr;
    std::vector<std::pair<String, TCHAR>> m_KeyBindsArr;
    Level* m_LevelPtr = nullptr;
    Camera* m_CameraPtr = nullptr;
    
    int m_LastLevel = 0;
    AnimationList* m_AnimationListPtr = nullptr;

    TriggerList* m_TriggerListPtr = nullptr;
    LevelEnd* m_LevelEndPtr = nullptr;
    CoinList* m_CoinListPtr = nullptr;
    EntityList* m_EntityListPtr = nullptr;
    EnemyList* m_EnemyListPtr = nullptr;

    Sound* m_SndBgMusicPtr = nullptr;
};

 
