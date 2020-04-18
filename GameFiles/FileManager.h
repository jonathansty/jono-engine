#pragma once


class Avatar;
class Entity;
class HUD;
class Enemy;
class Level;
class Camera;
class Arrow;
class bitmap_manager;
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

class FileManager 
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
    using KeyMap = std::map<std::string, UINT32>;

    //! Reads the level file
    void ReadGameInit(const std::string& filePath);
    //! Reads the level file only for certain objects
    void ReadGameInitForObject(const std::string& filePath, const std::string& objectName);
    //! Loads the background music
    void LoadGameMusic(const std::string& filePath);
    //! Loads all the avatar keybindings from the filePath
    //! Make sure the config file layout is correct.
    KeyMap LoadAvatarKeybinds(const std::string& filePath);
    //! Saves the game result in the file GameResults.txt
    void SaveGameResults(tm beginTime, tm endTime, Game* gamePtr);
    //! Reads the file GameResults.txt
    void ReadGameResults(std::vector<sessionStats*> &tmpSessionStatsArrPtr);
    //! Removes everything
    void RemoveAll();
    //! Removes all lists and clears them
    void ClearLists();
    //! Loads a list of levels from the filePath
    LevelList* LoadLevels(const std::string& filePath);

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
    KeyMap const& GetKeyBinds();
    //! Sets the keysbinds for the avatar
    void SetKeyBinds(KeyMap& tmpKeyBindsArr, std::string filePath);

    //! Updates the m_LastLevel variable for use when saving and reading results
    void UpdateLastLevel(int lastLevel);

private: 
    //! Creates all the objects.
    void CreateObject(tinyxml2::XMLElement* element);
    //! creates a specific object
    void CreateSpecificObject(const std::wstring& expressionStringRef, const String& objectName);
    //! Gets the value of a property
    std::wstring GetValue(const std::wstring& nameRef, const std::wstring& objectRef);
    std::string GetValue(const std::string& nameRef, tinyxml2::XMLElement* el);
    DOUBLE2 StringToDouble2(const std::wstring& valueRef);
    DOUBLE2 StringToDouble2(const std::string& valueRef);
    //! Reads a start block
    void ReadGameResultsStart(std::wstring &expressionStringRef, sessionStats* tmpSessionStat);
    //! Read an end block
    void ReadGameResultsEnd(std::wstring &expressionStringRef, sessionStats* tmpSessionStat);
    //! Gets the value of a block
    std::wstring GetValueOfBlock(const std::wstring& nameRef, const std::wstring& objectRef);

    //Read File Methods
    void ExtractBgMusic(tinyxml2::XMLElement* element);
    void ExtractAvatar(tinyxml2::XMLElement* element);
    void ExtractLevel(tinyxml2::XMLElement* element);
    void ExtractLevelEnd(tinyxml2::XMLElement* element);
    void ExtractCamera(tinyxml2::XMLElement* element);
    void ExtractBlockSlide(tinyxml2::XMLElement* element);
    void ExtractGate(tinyxml2::XMLElement* element);
    void ExtractLaser(tinyxml2::XMLElement* element);
    void ExtractArrowShooter(tinyxml2::XMLElement* element);
    void ExtractArrow(tinyxml2::XMLElement* element);
    void ExtractEnemyShooter(tinyxml2::XMLElement* element);
    void ExtractCheckPoint(tinyxml2::XMLElement* element);
    void ExtractEnemyRotater(tinyxml2::XMLElement* element);
    void ExtractEnemyHorizontal(tinyxml2::XMLElement* element);
    void ExtractCoin(tinyxml2::XMLElement* element);
    void ExtractTeleport(tinyxml2::XMLElement* element);
    void ExtractEnemyRocketLauncher(tinyxml2::XMLElement* element);
    void ExtractMetalFan(tinyxml2::XMLElement* element);
    void ExtractStickyWall(tinyxml2::XMLElement* element);
    void ExtractSlicer(tinyxml2::XMLElement* element);
    void ExtractNpcHinter(tinyxml2::XMLElement* element);

    Avatar* m_AvatarPtr = nullptr;
    KeyMap m_KeyBindsArr;
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

 
