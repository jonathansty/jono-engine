//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE5
//-----------------------------------------------------
#include "stdafx.h"		
	
//---------------------------
// Includes
//---------------------------
#include "FileManager.h"
#include "ElectronicJonaJoy.h"																				
#include "Entity.h"
#include "Avatar.h"
#include "Level.h"
#include "Camera.h"
#include "Arrow.h"
#include "BitmapManager.h"
#include "EnemyShooter.h"
#include "CheckPoint.h"
#include "RotLight.h"
#include "Enemy.h"
#include "EnemyRotater.h"
#include "EnemyHorizontal.h"
#include "Coin.h"
#include "CoinList.h"
#include "AttackBeam.h"
#include "AttackBeamList.h"
#include "EntityList.h"
#include "EnemyList.h"
#include "Teleport.h"
#include "CheckPointBg.h"
#include "HUD.h"
#include "EnemyRocketLauncher.h"
#include "EnemyRocket.h"
#include "AnimationList.h"
#include "CombRotLightCpBg.h"
#include "LevelEnd.h"
#include "EntityDestroy.h"
#include "BlockSlide.h"
#include "Gate.h"
#include "ArrowShooter.h"
#include "EnemyLaser.h"
#include "TriggerList.h"
#include "CameraTrigger.h"
#include "CameraTriggerRotate.h"
#include "MetalFans.h"
#include "StickyWall.h"
#include "Lever.h"
#include "LevelList.h"
#include "Slicer.h"
#include "NpcHinter.h"
#include "Game.h"
//---------------------------
// Defines
//---------------------------
#define GAME_ENGINE (GameEngine::GetSingleton())
#define BITMAP_MANAGER (BitmapManager::GetSingleton())
//---------------------------
// Constructor & Destructor
//---------------------------
FileManager::FileManager()
{
    m_AnimationListPtr = new AnimationList();
    m_TriggerListPtr = new TriggerList();
    m_EnemyListPtr = new EnemyList();
    m_EntityListPtr = new EntityList();
    m_CoinListPtr = new CoinList();
}

FileManager::~FileManager()
{
    delete m_AnimationListPtr;
    m_AnimationListPtr = nullptr;
    delete m_TriggerListPtr;
    m_TriggerListPtr = nullptr;
    delete m_EntityListPtr;
    m_EntityListPtr = nullptr;
    delete m_EnemyListPtr;
    m_EnemyListPtr = nullptr;
    delete m_CoinListPtr;
    m_CoinListPtr = nullptr;

    delete m_CameraPtr;
    m_CameraPtr = nullptr;
    delete m_AvatarPtr;
    m_AvatarPtr = nullptr;
    delete m_LevelPtr;
    m_LevelPtr = nullptr;
    delete m_LevelEndPtr;
    m_LevelEndPtr = nullptr;
}

//! Reads the level file
void FileManager::ReadGameInit(const String& filePath)
{
    std::wifstream inputFile;
    inputFile.open(filePath.C_str());
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully opened GameInit.txt."));
    }
    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {

        std::getline(inputFile, extractedLine);
        expressionStream << extractedLine;
        if (extractedLine.find(L"//") != std::string::npos)
        {
            expressionStream.str(L"");
        }
        else if (extractedLine.find(L"/>") != std::string::npos)
        {
            CreateObjects(expressionStream.str());
            expressionStream.str(L"");

        }
    }
    inputFile.close();
}
//! Reads the level file only for certain objects
void FileManager::ReadGameInitForObject(const String& filePath, const String& objectName)
{
    std::wifstream inputFile;
    inputFile.open(filePath.C_str());
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully loaded GameInit.txt."));
    }
    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {

        std::getline(inputFile, extractedLine);
        expressionStream << extractedLine;
        if (extractedLine.find(L"//") != std::string::npos)
        {
            expressionStream.str(L"");
        }
        else if (extractedLine.find(L"/>") != std::string::npos)
        {
            CreateSpecificObject(expressionStream.str(), objectName);
            expressionStream.str(L"");
                
        }
    }
    inputFile.close();
}
//! Creates all the objects.
void FileManager::CreateObjects(const std::wstring& expressionStringRef)
{
    if (expressionStringRef.find(L"<levelSound") != std::string::npos)
    {
        ExtractBgMusic(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Avatar") != std::string::npos)
    {
        ExtractAvatar(expressionStringRef);
    }
    if (expressionStringRef.find(L"<LevelMap") != std::string::npos)
    {
        ExtractLevel(expressionStringRef);
    }
    if (expressionStringRef.find(L"<LevelEnd") != std::string::npos)
    {
        ExtractLevelEnd(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Camera") != std::string::npos)
    {
        ExtractCamera(expressionStringRef);
    }
    if (expressionStringRef.find(L"<BlockSlide") != std::string::npos)
    {
        ExtractBlockSlide(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Gate") != std::string::npos)
    {
        ExtractGate(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Laser") != std::string::npos)
    {
        ExtractLaser(expressionStringRef);
    }
    if (expressionStringRef.find(L"<JumpShooter") != std::string::npos)
    {
        ExtractArrowShooter(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Arrow") != std::string::npos)
    {
        ExtractArrow(expressionStringRef);
    }
    if (expressionStringRef.find(L"<EnemyShooter") != std::string::npos)
    {
        ExtractEnemyShooter(expressionStringRef);
    }
    if (expressionStringRef.find(L"<CheckPoint") != std::string::npos)
    {
        ExtractCheckPoint(expressionStringRef);
    }
    if (expressionStringRef.find(L"<EnemyRotater") != std::string::npos)
    {
        ExtractEnemyRotater(expressionStringRef);
    }
    if (expressionStringRef.find(L"<EnemyHorizontal") != std::string::npos)
    {
        ExtractEnemyHorizontal(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Coin") != std::string::npos)
    {
        ExtractCoin(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Teleport") != std::string::npos)
    {
        ExtractTeleport(expressionStringRef);
    }
    if (expressionStringRef.find(L"<EnemyRocketLauncher") != std::string::npos)
    {
        ExtractEnemyRocketLauncher(expressionStringRef);
    }
    if (expressionStringRef.find(L"<MetalFan") != std::string::npos)
    {
        ExtractMetalFan(expressionStringRef);
    }
    if (expressionStringRef.find(L"<StickyWall") != std::string::npos)
    {
        ExtractStickyWall(expressionStringRef);
    }
    if (expressionStringRef.find(L"<Slicer") != std::string::npos)
    {
        ExtractSlicer(expressionStringRef);
    }
    if (expressionStringRef.find(L"<NpcHinter")!= std::string::npos)
    {
        ExtractNpcHinter(expressionStringRef);
    }
}
/*  Methods for extracting every object
*   
*/
void FileManager::ExtractBgMusic(const std::wstring& expressionStringRef)
{
    String filePath = String(GetValue(L"soundPath", expressionStringRef).c_str());
    if (m_SndBgMusicPtr == nullptr)
    {
        m_SndBgMusicPtr = SoundManager::GetSingleton()->LoadMusic(filePath);
        m_SndBgMusicPtr->Stop();
    }
    else
    {
        if (m_SndBgMusicPtr->GetPath() != filePath)
        {
            m_SndBgMusicPtr->Stop();
            m_SndBgMusicPtr = SoundManager::GetSingleton()->LoadMusic(filePath);
        }
    }
}
void FileManager::ExtractAvatar(const std::wstring& expressionStringRef)
{
    DOUBLE2 respawnPosition = StringToDouble2(GetValue(L"respawnPosition", expressionStringRef));
    //double coins = m_AvatarPtr->GetAmountOfCoins();
    if (m_AvatarPtr != nullptr)
    {
        DOUBLE2 oldRespawn = m_AvatarPtr->GetRespawnPosition();
        //respawnPosition = oldRespawn;

    }
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    String bitmapEpicMode = String(GetValue(L"bitmapEpicMode", expressionStringRef).c_str());
    double gravityScale = String(GetValue(L"gravityScale", expressionStringRef).c_str()).ToDouble();
    double jumpHeight = String(GetValue(L"jumpheight", expressionStringRef).c_str()).ToDouble();

    if (m_AvatarPtr == nullptr)
    {
        m_AvatarPtr = new Avatar(respawnPosition, BITMAP_MANAGER->LoadBitmapFile(bitmapName), BITMAP_MANAGER->LoadBitmapFile(bitmapEpicMode));
    }
    //m_AvatarPtr->AddAmountOfCoins(coins);
    m_AvatarPtr->SetPosition(respawnPosition);
    m_AvatarPtr->SetSpawnPosition(respawnPosition);
    m_AvatarPtr->SetGravityScale(gravityScale);
    m_AvatarPtr->SetLevel(m_LevelPtr);
    m_AvatarPtr->SetJumpHeight(jumpHeight);

    GAME_ENGINE->ConsolePrintString(String("Avatar succesfully created!"));
}
void FileManager::ExtractLevel(const std::wstring& expressionStringRef)
{
    if (m_LevelPtr != nullptr && m_LevelPtr->GetBgBmpPtr())
    {
        BITMAP_MANAGER->RemoveBitmapFile(m_LevelPtr->GetBgBmpPtr()->GetFileName());
    }

    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    String svgName = String(GetValue(L"svg", expressionStringRef).c_str());
    String svgNameBounds = String(GetValue(L"svgBounds", expressionStringRef).c_str());
    if (m_LevelPtr != nullptr)
    {
        delete m_LevelPtr;
        m_LevelPtr = nullptr;
    }

    if (svgNameBounds == String("NULL"))
    {
        m_LevelPtr = new Level(BITMAP_MANAGER->LoadBitmapFile(bitmapName), svgName);
    }
    else
    {
        m_LevelPtr = new Level(BITMAP_MANAGER->LoadBitmapFile(bitmapName), svgName, svgNameBounds);
    }
    GAME_ENGINE->ConsolePrintString(String("Level succesfully created!"));
}
void FileManager::ExtractLevelEnd(const std::wstring& expressionStringRef)
{
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));

    LevelEnd* tmpLevelEnd = new LevelEnd(position, String("Resources/test.txt"));

    if (m_LevelEndPtr != nullptr)
    {
        delete m_LevelEndPtr;
        m_LevelEndPtr = nullptr;
    }
    //Reading amount of levers
    int i = 0;

    while (expressionStringRef.find((String("Lever") + String(i) + String("Position")).C_str()) != std::string::npos)
    {
        String value = String("Lever") + String(i) + String("Position");
        i++;
        DOUBLE2 leverPosition = StringToDouble2(GetValue(value.C_str(), expressionStringRef));
        Lever* tmpLeverPtr = nullptr;
        int color = rand() & 3;
        switch (color)
        {
        case 0:
            tmpLeverPtr = new Lever(leverPosition, BITMAP_MANAGER->LoadBitmapFile(String("Resources/Interactions/buttonBlue.png")));
            break;
        case 1:
            tmpLeverPtr = new Lever(leverPosition, BITMAP_MANAGER->LoadBitmapFile(String("Resources/Interactions/buttonYellow.png")));
            break;
        case 2:
            tmpLeverPtr = new Lever(leverPosition, BITMAP_MANAGER->LoadBitmapFile(String("Resources/Interactions/buttonGreen.png")));
            break;
        case 3:
            tmpLeverPtr = new Lever(leverPosition, BITMAP_MANAGER->LoadBitmapFile(String("Resources/Interactions/buttonOrange.png")));
            break;
        default:
            break;
        }

        tmpLeverPtr->SetAvatar(m_AvatarPtr);
        tmpLevelEnd->Add(tmpLeverPtr);
    }
    m_LevelEndPtr = tmpLevelEnd;

    GAME_ENGINE->ConsolePrintString(String("LevelEnd succesfully created!"));
}
void FileManager::ExtractCamera(const std::wstring& expressionStringRef)
{
    if (expressionStringRef.find(L"<CameraTriggerRotate") != std::string::npos)
    {
        DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));

        int width = String(GetValue(L"width", expressionStringRef).c_str()).ToInteger();
        int height = String(GetValue(L"height", expressionStringRef).c_str()).ToInteger();
        double lowerAngle = String(GetValue(L"lowerAngle", expressionStringRef).c_str()).ToDouble();
        double upperAngle = String(GetValue(L"upperAngle", expressionStringRef).c_str()).ToDouble();
        double angularVelocity = String(GetValue(L"angularVelocity", expressionStringRef).c_str()).ToDouble();
        CameraTriggerRotate* tmpCameraTrigger = new CameraTriggerRotate(position, width, height);
        tmpCameraTrigger->SetAngleLimit(lowerAngle, upperAngle);
        tmpCameraTrigger->SetCamera(m_CameraPtr);
        tmpCameraTrigger->SetAngularVelocity(angularVelocity);
        m_TriggerListPtr->Add(tmpCameraTrigger);

    }
    else if (expressionStringRef.find(L"<Camera") != std::string::npos)
    {
        DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
        DOUBLE2 startPosition = StringToDouble2(GetValue(L"startPosition", expressionStringRef));
        double speed = String(GetValue(L"speed", expressionStringRef).c_str()).ToDouble();
        double scale = 1;
        if (expressionStringRef.find(L"scale") != std::string::npos)
        {
            scale = String(GetValue(L"scale", expressionStringRef).c_str()).ToDouble();
            std::cout << "The scale has been set to " << scale << std::endl;
        }
        if (m_CameraPtr != nullptr)
        {
            delete m_CameraPtr;
            m_CameraPtr = nullptr;
        }

        String controlState = String(GetValue(L"mode", expressionStringRef).c_str());
        Camera* tmpCamera = new Camera(m_LevelPtr, m_AvatarPtr, scale);
        if (controlState == String("manual"))
        {
            tmpCamera->SetCameraMode(Camera::controlState::MANUAL);
            tmpCamera->SetDefaultCameraMode(Camera::controlState::MANUAL);
        }
        else if (controlState == String("automatic"))
        {
            tmpCamera->SetCameraMode(Camera::controlState::AUTOMATIC);
            tmpCamera->SetDefaultCameraMode(Camera::controlState::AUTOMATIC);
        }
        else if (controlState == String("followAvatar"))
        {
            tmpCamera->SetCameraMode(Camera::controlState::FOLLOWAVATAR);
            tmpCamera->SetDefaultCameraMode(Camera::controlState::FOLLOWAVATAR);
        }

        tmpCamera->setCameraPosition(position);
        tmpCamera->SetCameraStartPosition(startPosition);
        tmpCamera->setCameraSpeed(speed);
        tmpCamera->SetScale(scale);
        m_CameraPtr = tmpCamera;
        GAME_ENGINE->ConsolePrintString(String("Camera succesfully created!"));
    }
}
void FileManager::ExtractBlockSlide(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    int width = String(GetValue(L"width", expressionStringRef).c_str()).ToInteger();
    int height = String(GetValue(L"height", expressionStringRef).c_str()).ToInteger();
    BlockSlide* tmpBlockSlide = new BlockSlide(position, width, height);
    tmpBlockSlide->SetAvatar(m_AvatarPtr);
    tmpBlockSlide->SetName(name);
    m_EntityListPtr->Add(tmpBlockSlide);
    GAME_ENGINE->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractGate(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 triggerPosition = StringToDouble2(GetValue(L"triggerPosition", expressionStringRef));
    DOUBLE2 GatePosition = StringToDouble2(GetValue(L"gatePosition", expressionStringRef));

    Gate* tmpGate = new Gate(GatePosition, triggerPosition);
    tmpGate->SetAvatar(m_AvatarPtr);
    tmpGate->SetName(name);
    m_EntityListPtr->Add(tmpGate);
    GAME_ENGINE->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractLaser(const std::wstring& expressionStringRef)
{
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    double angularVelocity = String(GetValue(L"angularVelocity", expressionStringRef).c_str()).ToDouble();
    EnemyLaser* tmpLaser = new EnemyLaser(position);
    tmpLaser->setName(name);
    tmpLaser->SetAngularVelocity(angularVelocity);
    tmpLaser->SetLevel(m_LevelPtr);
    tmpLaser->SetAvatar(m_AvatarPtr);
    m_EnemyListPtr->Add(tmpLaser);
}
void FileManager::ExtractArrow(const std::wstring& expressionStringRef)
{
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    double pushpower = String(GetValue(L"pushpower", expressionStringRef).c_str()).ToDouble();
    Arrow* tmpArrow = new Arrow(position, BITMAP_MANAGER->LoadBitmapFile(bitmapName));
    tmpArrow->SetPushPower(pushpower);
    tmpArrow->SetName(name);

    m_EntityListPtr->Add(tmpArrow);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractArrowShooter(const std::wstring& expressionStringRef)
{
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    DOUBLE2 direction = StringToDouble2(GetValue(L"direction", expressionStringRef));
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    int pushpower = String(GetValue(L"pushpower", expressionStringRef).c_str()).ToInteger();
    double intervalTime = String(GetValue(L"intervalTime", expressionStringRef).c_str()).ToDouble();
    ArrowShooter* tmpArrowShooterPtr = new ArrowShooter(position, direction, intervalTime);
    tmpArrowShooterPtr->SetName(name);
    tmpArrowShooterPtr->SetPushPower(pushpower);
    m_EntityListPtr->Add(tmpArrowShooterPtr);
    GAME_ENGINE->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractEnemyShooter(const std::wstring& expressionStringRef)
{
    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    double angle = String(GetValue(L"angle", expressionStringRef).c_str()).ToDouble();
    bool mirror = StringToBool(GetValue(L"mirror", expressionStringRef.c_str()));
    double gravityScale = String(GetValue(L"gravityScale", expressionStringRef).c_str()).ToDouble();
    EnemyShooter* tmpEnemyPtr = new EnemyShooter(position, BITMAP_MANAGER->LoadBitmapFile(bitmapName), angle);
    tmpEnemyPtr->SetMirror(mirror);
    tmpEnemyPtr->SetLevel(m_LevelPtr);
    tmpEnemyPtr->SetAvatar(m_AvatarPtr);
    tmpEnemyPtr->setName(name);
    m_EnemyListPtr->Add(tmpEnemyPtr);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractCheckPoint(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    double angle = 0;
    if (expressionStringRef.find(L"cameraAngle") != std::string::npos)
    {
        angle = String(GetValue(L"cameraAngle", expressionStringRef).c_str()).ToDouble();
    }
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    DOUBLE2 cameraPosition = position;
    if (expressionStringRef.find(L"cameraPosition") != std::string::npos)
    {
        cameraPosition = StringToDouble2(GetValue(L"cameraPosition", expressionStringRef));
    }
    CheckPoint* tmpCheckPoint = new CheckPoint(position, BITMAP_MANAGER->LoadBitmapFile(bitmapName));
    tmpCheckPoint->SetAvatar(m_AvatarPtr);
    tmpCheckPoint->SetName(name);
    tmpCheckPoint->SetCameraPosition(cameraPosition);
    tmpCheckPoint->SetCameraAngle(angle);
    m_EntityListPtr->Add(tmpCheckPoint);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyRotater(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    EnemyRotater* tmpEnemyRotater = new EnemyRotater(position, BITMAP_MANAGER->LoadBitmapFile(bitmapName));
    tmpEnemyRotater->SetLevel(m_LevelPtr);
    tmpEnemyRotater->SetAvatar(m_AvatarPtr);
    tmpEnemyRotater->setName(name);
    m_EnemyListPtr->Add(tmpEnemyRotater);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyHorizontal(const std::wstring& expressionStringRef)
{
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    String bitmapname = String(GetValue(L"bitmap", expressionStringRef).c_str());
    DOUBLE2 velocity = StringToDouble2(GetValue(L"velocity", expressionStringRef));
    DOUBLE2 offset = StringToDouble2(GetValue(L"offset", expressionStringRef));
    int lifes = 1;
    if (expressionStringRef.find(L"lifes") != std::string::npos)
    {
        lifes = String(GetValue(L"lifes", expressionStringRef).c_str()).ToInteger();
    }
    EnemyHorizontal* tmpEnemyPtr = new EnemyHorizontal(position, BITMAP_MANAGER->LoadBitmapFile(bitmapname), m_AvatarPtr);
    tmpEnemyPtr->SetLevel(m_LevelPtr);
    tmpEnemyPtr->SetVelocity(velocity);
    tmpEnemyPtr->SetOffSet(offset);
    tmpEnemyPtr->setName(name);
    tmpEnemyPtr->SetLifes(lifes);
    m_EnemyListPtr->Add(tmpEnemyPtr);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractCoin(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String bitmapname = String(GetValue(L"bitmap", expressionStringRef).c_str());
    int coinValue = String(GetValue(L"value", expressionStringRef).c_str()).ToInteger();
    Coin* tmpCoin = new Coin(position, BITMAP_MANAGER->LoadBitmapFile(bitmapname));
    tmpCoin->SetCoinValue(coinValue);
    tmpCoin->SetName(name);
    m_CoinListPtr->Add(tmpCoin);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractTeleport(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 entrancePos = StringToDouble2(GetValue(L"entrancePos", expressionStringRef));
    DOUBLE2 exitPos = StringToDouble2(GetValue(L"exitPos", expressionStringRef));
    String bitmapname = String(GetValue(L"bitmap", expressionStringRef).c_str());
    Teleport* tmpTeleport = new Teleport(entrancePos, exitPos, BITMAP_MANAGER->LoadBitmapFile(bitmapname));
    tmpTeleport->SetAvatar(m_AvatarPtr);
    tmpTeleport->SetLevel(m_LevelPtr);
    tmpTeleport->SetName(name);
    m_EntityListPtr->Add(tmpTeleport);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyRocketLauncher(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    double angle = String(GetValue(L"angle", expressionStringRef).c_str()).ToDouble();
    EnemyRocketLauncher* tmpEnemyRocketLauncher = new EnemyRocketLauncher(position, angle);
    tmpEnemyRocketLauncher->SetAvatar(m_AvatarPtr);
    tmpEnemyRocketLauncher->setName(name);
    m_EnemyListPtr->Add(tmpEnemyRocketLauncher);
    GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractMetalFan(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    double angle = String(GetValue(L"angle", expressionStringRef).c_str()).ToDouble();
    double repulsionForce = String(GetValue(L"repulsionForce", expressionStringRef).c_str()).ToDouble();
    double frictionForce = String(GetValue(L"frictionForce", expressionStringRef).c_str()).ToDouble();
    MetalFans* tmpMetalFan = new MetalFans(position, angle);
    tmpMetalFan->SetRepulsionForce(repulsionForce);
    tmpMetalFan->SetFrictionForce(frictionForce);
    tmpMetalFan->SetAvatar(m_AvatarPtr);
    m_EntityListPtr->Add(tmpMetalFan);
}
void FileManager::ExtractStickyWall(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    int width = String(GetValue(L"width", expressionStringRef).c_str()).ToInteger();
    int height = String(GetValue(L"height", expressionStringRef).c_str()).ToInteger();
    StickyWall* tmpStickyWall = new StickyWall(position, width, height);
    tmpStickyWall->SetName(name);
    tmpStickyWall->SetAvatar(m_AvatarPtr);
    m_EntityListPtr->Add(tmpStickyWall);
}
void FileManager::ExtractSlicer(const std::wstring& expressionStringRef)
{
    Slicer* tmpSlicerPtr = nullptr;
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    DOUBLE2 barPosition = StringToDouble2(GetValue(L"barPosition", expressionStringRef));
    int radius = String(GetValue(L"radius", expressionStringRef).c_str()).ToInteger();
    tmpSlicerPtr = new Slicer(position, barPosition,radius);
    tmpSlicerPtr->SetAvatar(m_AvatarPtr);
    tmpSlicerPtr->SetLevel(m_LevelPtr);
    m_EnemyListPtr->Add(tmpSlicerPtr);
}
void FileManager::ExtractNpcHinter(const std::wstring& expressionStringRef)
{
    String name = String(GetValue(L"name", expressionStringRef).c_str());
    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    String tipText = String(GetValue(L"tipText", expressionStringRef).c_str());
    String facing = String(GetValue(L"facing", expressionStringRef).c_str());
    NpcHinter* tmpNpcHinterPtr = new NpcHinter(position, tipText);
    tmpNpcHinterPtr->SetAvatar(m_AvatarPtr);
    tmpNpcHinterPtr->SetName(name);
    tmpNpcHinterPtr->SetFacingDirection(facing);
    m_EntityListPtr->Add(tmpNpcHinterPtr);
}

//! Loads the background music
void FileManager::LoadGameMusic(const String& filePath)
{
    std::wifstream inputFile;
    inputFile.open(filePath.C_str());
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully opened GameInit.txt."));
    }
    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {

        std::getline(inputFile, extractedLine);
        expressionStream << extractedLine;
        if (extractedLine.find(L"//") != std::string::npos)
        {
            expressionStream.str(L"");
        }
        else if (extractedLine.find(L"</") != std::string::npos)
        {
            std::wstring expressionStringRef = expressionStream.str();
            if (expressionStringRef.find(L"<MusicList") != std::string::npos)
            {
                int i = 0;
                std::wstringstream currentString;
                currentString << L"music_";
                currentString << i;
                while (expressionStringRef.find(currentString.str()) != std::string::npos)
                {
                    String filePath = String(GetValue(currentString.str(), expressionStringRef).c_str());
                    SoundManager::GetSingleton()->LoadMusic(filePath);
                    i++;
                    currentString.str(L"");
                    currentString << L"music_";
                    currentString << i;
                }
            }
            
            
            expressionStream.str(L"");

        }
    }
    inputFile.close();
}
//! creates a specific object
void FileManager::CreateSpecificObject(const std::wstring& expressionStringRef, const String& objectName)
{
    if (objectName == String("Arrow") && expressionStringRef.find(L"<Arrow") != std::string::npos)
    {
        DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
        String name = String(GetValue(L"name", expressionStringRef).c_str());
        String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
        double pushpower = String(GetValue(L"pushpower", expressionStringRef).c_str()).ToDouble();
        Arrow* tmpArrow = new Arrow(position, BITMAP_MANAGER->LoadBitmapFile(bitmapName));
        tmpArrow->SetPushPower(pushpower);
        tmpArrow->SetName(name);
        m_EntityListPtr->Add(tmpArrow);
        GAME_ENGINE->ConsolePrintString(name + String(" succesfully created!"));
    }
}
//! Gets the value of a property
std::wstring FileManager::GetValue(const std::wstring& nameRef, const std::wstring& objectRef)
{
    int idStart = objectRef.find(nameRef) + nameRef.size() + 2;
    int idEnd = objectRef.find('\"', idStart + 1);
    return objectRef.substr(idStart, idEnd - idStart);
}
//! Converts a string to double2
DOUBLE2 FileManager::StringToDouble2(const std::wstring& valueRef)
{
    int idSeperator = valueRef.find(L",");
    int length = valueRef.size();
    String firstNumber = String(valueRef.substr(0, idSeperator).c_str());
    String secondNumber = String(valueRef.substr(idSeperator + 1, 1 + length - idSeperator).c_str());
    return DOUBLE2(firstNumber.ToInteger(), secondNumber.ToInteger());
}
//! Converts a string to bool
bool FileManager::StringToBool(const std::wstring& valueRef)
{
    if (valueRef == L"true")
    {
        return true;
    }
    else
    {
        return false;
    }
}
//! Returns the triggerList
TriggerList* FileManager::GetTriggers()
{
    return m_TriggerListPtr;
}
//! Returns the levelEnd
LevelEnd* FileManager::GetLevelEnd()
{
    return m_LevelEndPtr;
}
//! Returns the coinList
CoinList* FileManager::GetCoinList()
{
    return m_CoinListPtr;
}
//! Returns the EntityList
EntityList* FileManager::GetEntityList()
{
    return m_EntityListPtr;
}
//! Returns the EnemyList
EnemyList* FileManager::GetEnemyList()
{
    return m_EnemyListPtr;
}
//! Returns the background Music
Sound* FileManager::GetBgMusic()
{
    return m_SndBgMusicPtr;
}
//! Removes everything
void FileManager::RemoveAll()
{
    delete m_TriggerListPtr;
    m_TriggerListPtr = nullptr;

    delete m_LevelEndPtr;
    m_LevelEndPtr = nullptr;
    delete m_CoinListPtr;
    m_CoinListPtr = nullptr;

    delete m_AnimationListPtr;
    m_AnimationListPtr = nullptr;

    delete m_EntityListPtr;
    m_EntityListPtr = nullptr;

    delete m_EnemyListPtr;
    m_EnemyListPtr = nullptr;


    delete m_LevelPtr;
    m_LevelPtr = nullptr;

    if (m_CameraPtr != nullptr)
    {
        delete m_CameraPtr;
        m_CameraPtr = nullptr;
    }


    delete m_AvatarPtr;
    m_AvatarPtr = nullptr;

}
//! Returns the avatar
Avatar* FileManager::GetAvatar()
{
    return m_AvatarPtr;
}
//! Returns the level
Level* FileManager::GetLevel()
{
    return m_LevelPtr;
}
//! Returns the camera
Camera* FileManager::GetCamera()
{
    return m_CameraPtr;
}
//! Removes all lists and clears them
void FileManager::ClearLists()
{
    if (m_TriggerListPtr != nullptr)
    {
        m_TriggerListPtr->RemoveAll();
    }
    
    if (m_LevelEndPtr != nullptr)
    {
        delete m_LevelEndPtr;
        m_LevelEndPtr = nullptr;
    }
    if (m_CoinListPtr != nullptr)
    {
        m_CoinListPtr->RemoveAll();
    }
    if (m_AnimationListPtr != nullptr)
    {
        m_AnimationListPtr->RemoveAll();
    }
    if (m_EntityListPtr != nullptr)
    {
        m_EntityListPtr->RemoveAll();
    }
    if (m_EnemyListPtr != nullptr)
    {
        m_EnemyListPtr->RemoveAll();
    }

}
//! Returns the animationList
AnimationList* FileManager::GetAnimationList()
{
    return m_AnimationListPtr;
}

//! Loads all the avatar keybindings from the filePath
//! Make sure the config file layout is correct.
std::vector<std::pair<String, TCHAR>> FileManager::LoadAvatarKeybinds(const String& filePath)
{
    std::vector<std::pair<String, TCHAR>> tmpKeyBindsArr;
    std::wifstream inputFile;
    inputFile.open(filePath.C_str());
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully opened ") + filePath);
    }
    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {
        std::getline(inputFile, extractedLine);
        expressionStream << extractedLine;
        if (extractedLine.find(L"//") != std::string::npos)
        {
            expressionStream.str(L"");
        }
        else if (extractedLine.find(L"</") != std::string::npos)
        {
            
            std::wstring expressionStringRef = expressionStream.str();
            if (expressionStringRef.find(L"<AvatarControls") != std::string::npos)
            {
                int i = 0;
                if (expressionStringRef.find(L"jump") != std::string::npos)
                {
                    std::pair<String, TCHAR> tmpPair;
                    tmpPair.first = String("jump");
                    tmpPair.second = String(GetValue(L"jump", expressionStringRef).c_str()).ToInteger();
                    tmpKeyBindsArr.push_back(tmpPair);
                }
                if (expressionStringRef.find(L"attack") != std::string::npos)
                {
                    std::pair<String, TCHAR> tmpPair;
                    tmpPair.first = String("attack");
                    tmpPair.second = String(GetValue(L"attack", expressionStringRef).c_str()).ToInteger();
                    tmpKeyBindsArr.push_back(tmpPair);
                }
                if (expressionStringRef.find(L"left") != std::string::npos)
                {
                    std::pair<String, TCHAR> tmpPair;
                    tmpPair.first = String("left");
                    tmpPair.second = String(GetValue(L"left", expressionStringRef).c_str()).ToInteger();
                    tmpKeyBindsArr.push_back(tmpPair);
                }
                if (expressionStringRef.find(L"right") != std::string::npos)
                {
                    std::pair<String, TCHAR> tmpPair;
                    tmpPair.first = String("right");
                    tmpPair.second = String(GetValue(L"right", expressionStringRef).c_str()).ToInteger();
                    tmpKeyBindsArr.push_back(tmpPair);
                }
                if (expressionStringRef.find(L"god") != std::string::npos)
                {
                    std::pair<String, TCHAR> tmpPair;
                    tmpPair.first = String("god");
                    tmpPair.second = String(GetValue(L"god", expressionStringRef).c_str()).ToInteger();
                    tmpKeyBindsArr.push_back(tmpPair);
                }

            }
            expressionStream.str(L"");

        }
    }
    inputFile.close();
    m_KeyBindsArr = tmpKeyBindsArr;
    return tmpKeyBindsArr;
}
//! Sets the keysbinds for the avatar
void FileManager::SetKeyBinds(std::vector < std::pair<String, TCHAR>> tmpKeyBinds, String filePath)
{

    std::wifstream inputConfigFile;
    std::wstringstream filebuffer;

    // Opening input file
    inputConfigFile.open(filePath.C_str());
    if (inputConfigFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
        return;
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully opened ") + filePath);
    }


    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputConfigFile.eof()))
    {

        std::getline(inputConfigFile, extractedLine);
        expressionStream << extractedLine << std::endl;
        
        if (extractedLine.find(L"</") != std::string::npos)
        {
            
            std::wstring expressionStringRef = expressionStream.str();
            if (expressionStringRef.find(L"<AvatarControls>") != std::string::npos)
            {
                expressionStream.str(L"");
                //Write the new keybinds
                filebuffer << L"<AvatarControls>" << std::endl;
                
                for (size_t i = 0; i < tmpKeyBinds.size(); i++)
                {
                    std::stringstream tmpExpression;
                    filebuffer << "\t";
                    filebuffer << tmpKeyBinds[i].first.C_str() << "=" << "\"" << (int)(tmpKeyBinds[i].second) << "\"" << std::endl;
                }
                filebuffer << L"</AvatarControls>" << std::endl;
            }
            else
            {
                filebuffer << expressionStringRef;
            }
            expressionStream.str(L"");
        }
    }
    inputConfigFile.close();

    std::wofstream outputConfigFile;
    outputConfigFile.open("Resources/cfg/config.txt");
    if (outputConfigFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open the config file and update keybinds"));
        return;
    }
    outputConfigFile << filebuffer.str();
    outputConfigFile.close();
    

    //Open config file and write changes

    m_KeyBindsArr = tmpKeyBinds;
}
//! Returns the keybinds as a vector
std::vector<std::pair<String, TCHAR>> FileManager::GetKeyBinds()
{
    return m_KeyBindsArr;
}
//! Loads a list of levels from the filePath
LevelList* FileManager::LoadLevels(const String& filePath)
{
    LevelList* tmpLevelList = new LevelList();
    std::wifstream configFile;
    configFile.open(filePath.C_str());
    if (configFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to load levels!"));
        return nullptr;
    }
    std::wifstream inputFile;
    inputFile.open(filePath.C_str());
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String("Failed to open ") + filePath);
    }
    else
    {
        GAME_ENGINE->ConsolePrintString(String("Succesfully opened GameInit.txt."));
    }
    std::wstringstream expressionStream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {

        std::getline(inputFile, extractedLine);
        expressionStream << extractedLine;
        if (extractedLine.find(L"//") != std::string::npos)
        {
            expressionStream.str(L"");
        }
        else if (extractedLine.find(L"</") != std::string::npos)
        {
            std::wstring expressionStringRef = expressionStream.str();
            if (expressionStringRef.find(L"<LevelList") != std::string::npos)
            {
                int i = 0;
                std::wstringstream currentString;
                currentString << L"level_";
                currentString << i;
                while (expressionStringRef.find(currentString.str()) != std::string::npos)
                {
                    String filePath = String(GetValue(currentString.str(), expressionStringRef).c_str());
                    tmpLevelList->Add(filePath);
                    GAME_ENGINE->ConsolePrintString(String("Added ") + String(currentString.str().c_str()) + String(" to the levelList."));
                    i++;
                    currentString.str(L"");
                    currentString << L"level_";
                    currentString << i;
                }
            }
            expressionStream.str(L"");

        }
    }
    inputFile.close();
    return tmpLevelList;
}

//! Saves the game result in the file GameResults.txt
void FileManager::SaveGameResults(tm beginTime, tm endTime, Game* gamePtr)
{
    std::wofstream fileStream;

    fileStream.open(L"./Resources/GameResults.txt", std::ios_base::app);
    if (fileStream.fail() == true)
    {
        GAME_ENGINE->MessageBox(String("Failed to save in ./Resources/GameResults.txt"));

    }

    fileStream << "<Session>" << std::endl;
    // Writing Start part
    fileStream << "\t<Start>" << std::endl;
    fileStream << "\t\t<Date>";
    fileStream << 1900 + beginTime.tm_year << ":" << std::setw(2) << std::setfill(L'0') << beginTime.tm_mon + 1 << ":" << std::setw(2) << std::setfill(L'0') << beginTime.tm_mday;
    fileStream << "</Date>" << std::endl;
    fileStream << "\t\t<Time>";
    fileStream << std::setw(2) << std::setfill(L'0') << beginTime.tm_hour << ":" << std::setw(2) << std::setfill(L'0') << beginTime.tm_min << ":" << std::setw(2) << std::setfill(L'0') << beginTime.tm_sec;
    fileStream << "</Time>" << std::endl;
    fileStream << "\t</Start>" << std::endl;

    // Writing end part
    fileStream << "\t<End>" << std::endl;
    fileStream << "\t\t<Date>";
    fileStream << 1900 + endTime.tm_year << ":" << std::setw(2) << std::setfill(L'0') << endTime.tm_mon + 1 << ":" << std::setw(2) << std::setfill(L'0') << endTime.tm_mday;
    fileStream << "</Date>" << std::endl;
    fileStream << "\t\t<Time>";
    fileStream << std::setw(2) << std::setfill(L'0') << endTime.tm_hour << ":" << std::setw(2) << std::setfill(L'0') << endTime.tm_min << ":" << std::setw(2) << std::setfill(L'0') << endTime.tm_sec;
    fileStream << "</Time>" << std::endl;
    fileStream << "\t\t<Deaths>";
    fileStream << gamePtr->GetTotalDeaths();
    fileStream << "</Deaths>" << std::endl;
    fileStream << "\t\t<Money>";
    if (gamePtr->GetAvatar() != nullptr)
    {
        fileStream << gamePtr->GetAvatar()->GetAmountOfCoins();
    }
    fileStream << "</Money>" << std::endl;
    fileStream << "\t\t<LifeTime>";
    fileStream << std::setprecision(2) << gamePtr->GetTotalTime();
    fileStream << "</LifeTime>" << std::endl;
    fileStream << "\t\t<LastLevel>";
    fileStream << m_LastLevel << std::endl;
    fileStream << "</LastLevel>" << std::endl;
    fileStream << "\t</End>" << std::endl;
    fileStream << "</Session>" << std::endl;

    fileStream.close();
}
//! Reads the file GameResults.txt
void FileManager::ReadGameResults(std::vector<sessionStats*> &tmpSessionStatsPtrArr)
{
    std::wifstream inputFile;
    inputFile.open("./Resources/GameResults.txt");
    if (inputFile.fail())
    {
        GAME_ENGINE->MessageBox(String(" Could not read GameResults.txt"));
        return;
    }
    std::wstringstream expressionstream;
    std::wstring extractedLine;
    while (!(inputFile.eof()))
    {
        std::getline(inputFile, extractedLine);
        expressionstream << extractedLine;
        if (extractedLine.find(L"</Session>") != std::string::npos)
        {
            sessionStats* tmpSessionStat = new sessionStats();
            std::wstring tmpstring = expressionstream.str();
            //Extract the start block
            int startId = tmpstring.find(L"<Start>");
            int endId = tmpstring.find(L"</Start>");
            std::wstring startBlock = tmpstring.substr(startId, (endId + String("</Start>").Length())- startId);
           
            // Extract the end block
            startId = tmpstring.find(L"<End>");
            endId = tmpstring.find(L"</End>");
            std::wstring endBlock = tmpstring.substr(startId, (endId + String("</End>").Length()) - startId);
            ReadGameResultsStart(startBlock, tmpSessionStat);
            ReadGameResultsEnd(endBlock, tmpSessionStat);
            
            tmpSessionStatsPtrArr.push_back(tmpSessionStat);

            std::wcout << L"Session: " << tmpSessionStat->beginTime.tm_year << "/" << tmpSessionStat->beginTime.tm_mon << "/" << tmpSessionStat->beginTime.tm_mday << L". Read." << std::endl;
            //push back session into the vector
            expressionstream.str(L"");
        }
        
        
    }
    inputFile.close();
}
//! Reads a start block
void FileManager::ReadGameResultsStart(std::wstring &expressionStringRef, sessionStats* tmpSessionStat)
{

    //Read date
    std::wstring date = GetValueOfBlock(L"Date", expressionStringRef);
    tm beginTime;
    
    int idStart = 0;
    int idEnd = date.find(L":");
    beginTime.tm_year = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    beginTime.tm_mon = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    beginTime.tm_mday = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();

    //Read time
    date = GetValueOfBlock(L"Time", expressionStringRef);
    idStart = 0;
    idEnd = date.find(L":");
    beginTime.tm_hour = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    beginTime.tm_min = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    beginTime.tm_sec = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    tmpSessionStat->beginTime = beginTime;
}
//! Read an end block
void FileManager::ReadGameResultsEnd(std::wstring &expressionStringRef, sessionStats* tmpSessionStat)
{
    int numberofDeaths = String(GetValueOfBlock(L"Deaths", expressionStringRef).c_str()).ToInteger();
    int money = String(GetValueOfBlock(L"Money", expressionStringRef).c_str()).ToInteger();
    double lifeTime = String(GetValueOfBlock(L"LifeTime", expressionStringRef).c_str()).ToDouble();
    
    //Read End date
    tm endTime;
   
    std::wstring date = GetValueOfBlock(L"Date", expressionStringRef);
    
    int idStart = 0;
    int idEnd = date.find(L":");
    endTime.tm_year = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    endTime.tm_mon = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    endTime.tm_mday = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();

    //Read time
    date = GetValueOfBlock(L"Time", expressionStringRef);
    idStart = 0;
    idEnd = date.find(L":");
    endTime.tm_hour = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    endTime.tm_min = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();
    idStart = idEnd + 1;
    idEnd = date.find(L":", idStart);
    endTime.tm_sec = String(date.substr(idStart, idEnd - idStart).c_str()).ToInteger();

    //Get last level
    int lastLevel = String(GetValueOfBlock(L"LastLevel",expressionStringRef).c_str()).ToInteger();
    // Putting everything into sessionStats
    tmpSessionStat->lastLevel = lastLevel;
    tmpSessionStat->amountOfDeaths = numberofDeaths;
    tmpSessionStat->money = money;
    tmpSessionStat->lifeTime = lifeTime;
    tmpSessionStat->endTime = endTime;
}
//! Gets the value of a block
std::wstring FileManager::GetValueOfBlock(const std::wstring& nameRef, const std::wstring& objectRef)
{
    int idStart = objectRef.find(L"<" + nameRef + L">") + nameRef.length() + 2;
    int idEnd = objectRef.find(L"</" + nameRef + L">");

    std::wstring value = objectRef.substr(idStart, idEnd - idStart);
    return value;

}

//! Updates the m_LastLevel variable for use when saving and reading results
void FileManager::UpdateLastLevel(int lastLevel)
{
    m_LastLevel = lastLevel;
}