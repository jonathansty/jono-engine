#include "stdafx.h"		
	
#include "SoundManager.h"
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

using namespace tinyxml2;
//! Reads the level file
void FileManager::ReadGameInit(const std::string& filePath)
{
    tinyxml2::XMLDocument document{};
    document.LoadFile(filePath.c_str());

    // For each child
    for (XMLElement* current = document.FirstChildElement(); current; current = current->NextSiblingElement())
    {
        CreateObject(current);
    }
}
//! Reads the level file only for certain objects
void FileManager::ReadGameInitForObject(const std::string& filePath, const std::string& objectName)
{
    std::wifstream inputFile;
    inputFile.open(filePath);
    if (inputFile.fail())
    {
        game_engine::instance()->MessageBox(String("Failed to open ") + String(filePath.c_str()));
    }
    else
    {
        game_engine::instance()->ConsolePrintString(String("Successfully loaded GameInit.txt."));
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
            CreateSpecificObject(expressionStream.str(), String(objectName.c_str()));
            expressionStream.str(L"");
                
        }
    }
    inputFile.close();
}
//! Creates all the objects.
void FileManager::CreateObject(tinyxml2::XMLElement* element)
{
    std::string name = element->Name();
    if (name.compare("LevelSound") == 0)
    {
        ExtractBgMusic(element);
    }
    if (name.compare("Avatar") == 0)
    {
        ExtractAvatar(element);
    }
    if (name.compare("LevelMap") == 0)
    {
        ExtractLevel(element);
    }
    if (name.compare("LevelEnd") == 0)
    {
        ExtractLevelEnd(element);
    }
    if (name.compare("Camera") == 0)
    {
        ExtractCamera(element);
    }
    if (name.compare("BlockSlide") == 0)
    {
        ExtractBlockSlide(element);
    }
    if (name.compare("Gate") == 0)
    {
        ExtractGate(element);
    }
    if (name.compare("Laser") == 0)
    {
        ExtractLaser(element);
    }
    if (name.compare("JumpShooter") == 0)
    {
        ExtractArrowShooter(element);
    }
    if (name.compare("Arrow") == 0)
    {
        ExtractArrow(element);
    }
    if (name.compare("EnemyShooter") == 0)
    {
        ExtractEnemyShooter(element);
    }
    if (name.compare("CheckPoint") == 0)
    {
        ExtractCheckPoint(element);
    }
    if (name.compare("EnemyRotater") == 0)
    {
        ExtractEnemyRotater(element);
    }
    if (name.compare("EnemyHorizontal") == 0)
    {
        ExtractEnemyHorizontal(element);
    }
    if (name.compare("Coin") == 0)
    {
        ExtractCoin(element);
    }
    if (name.compare("Teleport") == 0)
    {
        ExtractTeleport(element);
    }
    if (name.compare("EnemyRocketLauncher") == 0)
    {
        ExtractEnemyRocketLauncher(element);
    }
    if (name.compare("MetalFan") == 0)
    {
        ExtractMetalFan(element);
    }
    if (name.compare("StickyWall") == 0)
    {
        ExtractStickyWall(element);
    }
    if (name.compare("Slicer") == 0)
    {
        ExtractSlicer(element);
    }
    if (name.compare("NpcHinter") == 0)
    {
        ExtractNpcHinter(element);
    }
}

/*  Methods for extracting every object
*   
*/
void FileManager::ExtractBgMusic(tinyxml2::XMLElement* element)
{
    const XMLAttribute* attribute = element->FindAttribute("soundPath");
    if (attribute)
    {
        String path = String(attribute->Value());
		if (m_SndBgMusicPtr == nullptr)
		{
			m_SndBgMusicPtr = sound_manager::instance()->LoadMusic(path);
			m_SndBgMusicPtr->Stop();
		}
		else
		{
			if (m_SndBgMusicPtr->GetPath() != path)
			{
				m_SndBgMusicPtr->Stop();
				m_SndBgMusicPtr = sound_manager::instance()->LoadMusic(path);
			}
		}

    }
}
void FileManager::ExtractAvatar(tinyxml2::XMLElement* element)
{

    XMLAttribute const*  respawnPosAttribute = element->FindAttribute("respawnPosition");
    assert(respawnPosAttribute);
    XMLAttribute const*  posAttribute = element->FindAttribute("position");
    assert(posAttribute);
    XMLAttribute const*  bitmapAttribute = element->FindAttribute("bitmap");
    assert(bitmapAttribute);
    XMLAttribute const*  bitmapEpicModeAttr = element->FindAttribute("bitmapEpicMode");
    assert(bitmapEpicModeAttr);
    XMLAttribute const*  gravityScaleAttr = element->FindAttribute("gravityScale");
    assert(gravityScaleAttr);
    XMLAttribute const*  jumpheightAttr = element->FindAttribute("jumpheight");
    assert(jumpheightAttr);

    DOUBLE2 respawnPosition = StringToDouble2(respawnPosAttribute->Value());
    //double coins = m_AvatarPtr->GetAmountOfCoins();
    if (m_AvatarPtr != nullptr)
    {
        DOUBLE2 oldRespawn = m_AvatarPtr->GetRespawnPosition();
        //respawnPosition = oldRespawn;
    }
    DOUBLE2 position = StringToDouble2(posAttribute->Value());
    String bitmapName = String(bitmapAttribute->Value());
    String bitmapEpicMode = String(bitmapEpicModeAttr->Value());
    double gravityScale = gravityScaleAttr->DoubleValue();
    double jumpHeight = jumpheightAttr->DoubleValue();

    if (m_AvatarPtr == nullptr)
    {
        m_AvatarPtr = new Avatar(respawnPosition, bitmap_manager::instance()->LoadBitmapFile(bitmapName), bitmap_manager::instance()->LoadBitmapFile(bitmapEpicMode));
    }
    //m_AvatarPtr->AddAmountOfCoins(coins);
    m_AvatarPtr->SetPosition(respawnPosition);
    m_AvatarPtr->SetSpawnPosition(respawnPosition);
    m_AvatarPtr->SetGravityScale(gravityScale);
    m_AvatarPtr->SetLevel(m_LevelPtr);
    m_AvatarPtr->SetJumpHeight(jumpHeight);

    game_engine::instance()->ConsolePrintString(String("Avatar succesfully created!"));
}
void FileManager::ExtractLevel(tinyxml2::XMLElement* element)
{
    if (m_LevelPtr != nullptr && m_LevelPtr->GetBgBmpPtr())
    {
        bitmap_manager::instance()->RemoveBitmapFile(m_LevelPtr->GetBgBmpPtr()->GetFileName());
    }

    // Attributes
    XMLAttribute const* bitmapAttr = element->FindAttribute("bitmap");
    assert(bitmapAttr);
    XMLAttribute const* svgAttr = element->FindAttribute("svg");
    assert(svgAttr);
    XMLAttribute const* svgBoundsAttr = element->FindAttribute("svgBounds");
    assert(svgBoundsAttr);

    String bitmapName = String(bitmapAttr->Value());
    String svgName = String(svgAttr->Value());
    String svgNameBounds = String(svgBoundsAttr->Value());
    if (m_LevelPtr != nullptr)
    {
        delete m_LevelPtr;
        m_LevelPtr = nullptr;
    }

    if (svgNameBounds == String("NULL"))
    {
        m_LevelPtr = new Level(bitmap_manager::instance()->LoadBitmapFile(bitmapName), svgName);
    }
    else
    {
        m_LevelPtr = new Level(bitmap_manager::instance()->LoadBitmapFile(bitmapName), svgName, svgNameBounds);
    }
    game_engine::instance()->ConsolePrintString(String("Level succesfully created!"));
}
void FileManager::ExtractLevelEnd(tinyxml2::XMLElement* element)
{
    XMLAttribute const* attribute = element->FindAttribute("position");
    assert(attribute);
    DOUBLE2 position = StringToDouble2(attribute->Value());

    LevelEnd* tmpLevelEnd = new LevelEnd(position, String("Resources/test.txt"));

    if (m_LevelEndPtr != nullptr)
    {
        delete m_LevelEndPtr;
        m_LevelEndPtr = nullptr;
    }

    //TODO: Fix levers to obey correct XML specs
    int i = 0;
    XMLAttribute const* attr = nullptr;
    std::string attrName = "";

    std::string paths[4] = {
        "Resources/Interactions/buttonBlue.png",
        "Resources/Interactions/buttonYellow.png",
        "Resources/Interactions/buttonGreen.png",
        "Resources/Interactions/buttonOrange.png",
    };
	do
	{
	    attrName = "Level" + std::to_string(i) + "Position";
        attr = element->FindAttribute(attrName.c_str());
        if (attr)
		{
            DOUBLE2 leverPosition = StringToDouble2(attr->Value());
			Lever* tmpLeverPtr = nullptr;
			int color = rand() & 3;
			tmpLeverPtr = new Lever(leverPosition, bitmap_manager::instance()->LoadBitmapFile(String(paths[color].c_str())));

			tmpLeverPtr->SetAvatar(m_AvatarPtr);
			tmpLevelEnd->Add(tmpLeverPtr);
		}
		i++;
    } while (attr);
    m_LevelEndPtr = tmpLevelEnd;

    game_engine::instance()->ConsolePrintString(String("LevelEnd succesfully created!"));
}
void FileManager::ExtractCamera(tinyxml2::XMLElement* element)
{
    std::string name = element->Name();
    if (name.compare("CameraTriggerRotate") == 0)
    {
        DOUBLE2 position = StringToDouble2(element->FindAttribute("position")->Value());

        int width = element->FindAttribute("width")->IntValue();
        int height = element->FindAttribute("height")->IntValue();
        double lowerAngle = element->FindAttribute("lowerAngle")->DoubleValue();
        double upperAngle = element->FindAttribute("upperAngle")->DoubleValue();
        double angularVelocity = element->FindAttribute("angularVelocity")->DoubleValue();
        CameraTriggerRotate* tmpCameraTrigger = new CameraTriggerRotate(position, width, height);
        tmpCameraTrigger->SetAngleLimit(lowerAngle, upperAngle);
        tmpCameraTrigger->SetCamera(m_CameraPtr);
        tmpCameraTrigger->SetAngularVelocity(angularVelocity);
        m_TriggerListPtr->Add(tmpCameraTrigger);

    }
    else if (name.compare("Camera") == 0)
    {
		DOUBLE2 position = StringToDouble2(element->FindAttribute("position")->Value());
		DOUBLE2 startPosition = StringToDouble2(element->FindAttribute("startPosition")->Value());
        double speed = element->FindAttribute("speed")->DoubleValue();
        double scale = 1;
        if (XMLAttribute const* attr = element->FindAttribute("scale"); attr)
        {
            attr->QueryDoubleValue(&scale);
        }

        if (m_CameraPtr != nullptr)
        {
            delete m_CameraPtr;
            m_CameraPtr = nullptr;
        }

        String controlState = String(element->FindAttribute("mode")->Value());
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

        tmpCamera->SetCameraPosition(position);
        tmpCamera->SetCameraStartPosition(startPosition);
        tmpCamera->SetCameraSpeed(speed);
        tmpCamera->SetScale(scale);
        m_CameraPtr = tmpCamera;
        game_engine::instance()->ConsolePrintString(String("Camera succesfully created!"));
    }
}
void FileManager::ExtractBlockSlide(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    int width = String(GetValue("width", element).c_str()).ToInteger();
    int height = String(GetValue("height", element).c_str()).ToInteger();
    BlockSlide* tmpBlockSlide = new BlockSlide(position, width, height);
    tmpBlockSlide->SetAvatar(m_AvatarPtr);
    tmpBlockSlide->SetName(name);
    m_EntityListPtr->Add(tmpBlockSlide);
    game_engine::instance()->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractGate(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 triggerPosition = StringToDouble2(GetValue("triggerPosition", element));
    DOUBLE2 GatePosition = StringToDouble2(GetValue("gatePosition", element));

    Gate* tmpGate = new Gate(GatePosition, triggerPosition);
    tmpGate->SetAvatar(m_AvatarPtr);
    tmpGate->SetName(name);
    m_EntityListPtr->Add(tmpGate);
    game_engine::instance()->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractLaser(tinyxml2::XMLElement* element)
{
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String name = String(GetValue("name", element).c_str());
    double angularVelocity = String(GetValue("angularVelocity", element).c_str()).ToDouble();
    EnemyLaser* tmpLaser = new EnemyLaser(position);
    tmpLaser->setName(name);
    tmpLaser->SetAngularVelocity(angularVelocity);
    tmpLaser->SetLevel(m_LevelPtr);
    tmpLaser->SetAvatar(m_AvatarPtr);
    m_EnemyListPtr->Add(tmpLaser);
}
void FileManager::ExtractArrow(tinyxml2::XMLElement* element)
{
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String name = String(GetValue("name", element).c_str());
    String bitmapName = String(GetValue("bitmap", element).c_str());
    double pushpower = String(GetValue("pushpower", element).c_str()).ToDouble();
    Arrow* tmpArrow = new Arrow(position, bitmap_manager::instance()->LoadBitmapFile(bitmapName));
    tmpArrow->SetPushPower(pushpower);
    tmpArrow->SetName(name);

    m_EntityListPtr->Add(tmpArrow);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractArrowShooter(tinyxml2::XMLElement* element)
{
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    DOUBLE2 direction = StringToDouble2(GetValue("direction", element));
    String name = String(GetValue("name", element).c_str());
    int pushpower = String(GetValue("pushpower", element).c_str()).ToInteger();
    double intervalTime = String(GetValue("intervalTime", element).c_str()).ToDouble();
    ArrowShooter* tmpArrowShooterPtr = new ArrowShooter(position, direction, intervalTime);
    tmpArrowShooterPtr->SetName(name);
    tmpArrowShooterPtr->SetPushPower(pushpower);
    m_EntityListPtr->Add(tmpArrowShooterPtr);
    game_engine::instance()->ConsolePrintString(name + String(" sucessfully created!"));
}
void FileManager::ExtractEnemyShooter(tinyxml2::XMLElement* element)
{
    String bitmapName = String(GetValue("bitmap", element).c_str());
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    double angle = String(GetValue("angle", element).c_str()).ToDouble();
    bool mirror = element->FindAttribute("mirror")->BoolValue();
    double gravityScale = String(GetValue("gravityScale", element).c_str()).ToDouble();
    EnemyShooter* tmpEnemyPtr = new EnemyShooter(position, bitmap_manager::instance()->LoadBitmapFile(bitmapName), angle);
    tmpEnemyPtr->SetMirror(mirror);
    tmpEnemyPtr->SetLevel(m_LevelPtr);
    tmpEnemyPtr->SetAvatar(m_AvatarPtr);
    tmpEnemyPtr->setName(name);
    m_EnemyListPtr->Add(tmpEnemyPtr);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractCheckPoint(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    String bitmapName = String(GetValue("bitmap", element).c_str());
    double angle = 0;
    if (auto it = element->FindAttribute("cameraAngle"); it) 
    {
        angle = String(GetValue("cameraAngle", element).c_str()).ToDouble();
    }
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    DOUBLE2 cameraPosition = position;
    if (element->FindAttribute("cameraPosition") )
    {
        cameraPosition = StringToDouble2(GetValue("cameraPosition", element));
    }
    CheckPoint* tmpCheckPoint = new CheckPoint(position, bitmap_manager::instance()->LoadBitmapFile(bitmapName));
    tmpCheckPoint->SetAvatar(m_AvatarPtr);
    tmpCheckPoint->SetName(name);
    tmpCheckPoint->SetCameraPosition(cameraPosition);
    tmpCheckPoint->SetCameraAngle(angle);
    m_EntityListPtr->Add(tmpCheckPoint);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyRotater(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String bitmapName = String(GetValue("bitmap", element).c_str());
    EnemyRotater* tmpEnemyRotater = new EnemyRotater(position, bitmap_manager::instance()->LoadBitmapFile(bitmapName));
    tmpEnemyRotater->SetLevel(m_LevelPtr);
    tmpEnemyRotater->SetAvatar(m_AvatarPtr);
    tmpEnemyRotater->setName(name);
    m_EnemyListPtr->Add(tmpEnemyRotater);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyHorizontal(tinyxml2::XMLElement* element)
{
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String name = String(GetValue("name", element).c_str());
    String bitmapname = String(GetValue("bitmap", element).c_str());
    DOUBLE2 velocity = StringToDouble2(GetValue("velocity", element));
    DOUBLE2 offset = StringToDouble2(GetValue("offset", element));
    int lifes = 1;
    if (element->FindAttribute("lifes"))
    {
        lifes = String(GetValue("lifes", element).c_str()).ToInteger();
    }
    EnemyHorizontal* tmpEnemyPtr = new EnemyHorizontal(position, bitmap_manager::instance()->LoadBitmapFile(bitmapname), m_AvatarPtr);
    tmpEnemyPtr->SetLevel(m_LevelPtr);
    tmpEnemyPtr->SetVelocity(velocity);
    tmpEnemyPtr->SetOffSet(offset);
    tmpEnemyPtr->setName(name);
    tmpEnemyPtr->SetLifes(lifes);
    m_EnemyListPtr->Add(tmpEnemyPtr);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractCoin(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String bitmapname = String(GetValue("bitmap", element).c_str());
    int coinValue = String(GetValue("value", element).c_str()).ToInteger();
    Coin* tmpCoin = new Coin(position, bitmap_manager::instance()->LoadBitmapFile(bitmapname));
    tmpCoin->SetCoinValue(coinValue);
    tmpCoin->SetName(name);
    m_CoinListPtr->Add(tmpCoin);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractTeleport(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 entrancePos = StringToDouble2(GetValue("entrancePos", element));
    DOUBLE2 exitPos = StringToDouble2(GetValue("exitPos", element));
    String bitmapname = String(GetValue("bitmap", element).c_str());
    Teleport* tmpTeleport = new Teleport(entrancePos, exitPos, bitmap_manager::instance()->LoadBitmapFile(bitmapname));
    tmpTeleport->SetAvatar(m_AvatarPtr);
    tmpTeleport->SetLevel(m_LevelPtr);
    tmpTeleport->SetName(name);
    m_EntityListPtr->Add(tmpTeleport);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractEnemyRocketLauncher(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    double angle = String(GetValue("angle", element).c_str()).ToDouble();
    EnemyRocketLauncher* tmpEnemyRocketLauncher = new EnemyRocketLauncher(position, angle);
    tmpEnemyRocketLauncher->SetAvatar(m_AvatarPtr);
    tmpEnemyRocketLauncher->setName(name);
    m_EnemyListPtr->Add(tmpEnemyRocketLauncher);
    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
}
void FileManager::ExtractMetalFan(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    double angle = String(GetValue("angle", element).c_str()).ToDouble();
    double repulsionForce = String(GetValue("repulsionForce", element).c_str()).ToDouble();
    double frictionForce = String(GetValue("frictionForce", element).c_str()).ToDouble();
    MetalFans* tmpMetalFan = new MetalFans(position, angle);
    tmpMetalFan->SetRepulsionForce(repulsionForce);
    tmpMetalFan->SetFrictionForce(frictionForce);
    tmpMetalFan->SetAvatar(m_AvatarPtr);
    m_EntityListPtr->Add(tmpMetalFan);
}
void FileManager::ExtractStickyWall(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    int width = String(GetValue("width", element).c_str()).ToInteger();
    int height = String(GetValue("height", element).c_str()).ToInteger();
    StickyWall* tmpStickyWall = new StickyWall(position, width, height);
    tmpStickyWall->SetName(name);
    tmpStickyWall->SetAvatar(m_AvatarPtr);
    m_EntityListPtr->Add(tmpStickyWall);
}
void FileManager::ExtractSlicer(tinyxml2::XMLElement* element)
{
    Slicer* tmpSlicerPtr = nullptr;
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    DOUBLE2 barPosition = StringToDouble2(GetValue("barPosition", element));
    int radius = String(GetValue("radius", element).c_str()).ToInteger();
    tmpSlicerPtr = new Slicer(position, barPosition,radius);
    tmpSlicerPtr->SetAvatar(m_AvatarPtr);
    tmpSlicerPtr->SetLevel(m_LevelPtr);
    m_EnemyListPtr->Add(tmpSlicerPtr);
}
void FileManager::ExtractNpcHinter(tinyxml2::XMLElement* element)
{
    String name = String(GetValue("name", element).c_str());
    DOUBLE2 position = StringToDouble2(GetValue("position", element));
    String tipText = String(GetValue("tipText", element).c_str());
    String facing = String(GetValue("facing", element).c_str());
    NpcHinter* tmpNpcHinterPtr = new NpcHinter(position, tipText);
    tmpNpcHinterPtr->SetAvatar(m_AvatarPtr);
    tmpNpcHinterPtr->SetName(name);
    tmpNpcHinterPtr->SetFacingDirection(facing);
    m_EntityListPtr->Add(tmpNpcHinterPtr);
}

//! Loads the background music
void FileManager::LoadGameMusic(const std::string& filePath)
{

    tinyxml2::XMLDocument musicDocument;
    if(musicDocument.LoadFile(filePath.c_str()) != XML_SUCCESS)
	{
		game_engine::instance()->MessageBox(String("Failed to open ") + String(filePath.c_str()));
	}
	else
	{
		game_engine::instance()->ConsolePrintString(String("Succesfully opened GameInit.txt."));
	}


    XMLElement* m = musicDocument.FirstChildElement("MusicList");
	if (m)
	{
        for (auto it = m->FirstChildElement("Music"); it; it = it->NextSiblingElement())
        {
            auto attr = it->FindAttribute("path");

            if (attr)
            {
				String filePath = String(attr->Value());
				sound_manager::instance()->LoadMusic(filePath);

            }
        }
	}
}
//! creates a specific object
void FileManager::CreateSpecificObject(const std::wstring& expressionStringRef, const String& objectName)
{
    assert("RELOADING not supported anymore");
    //if (objectName == String("Arrow") && expressionStringRef.find(L"<Arrow") != std::string::npos)
    //{
    //    DOUBLE2 position = StringToDouble2(GetValue(L"position", expressionStringRef));
    //    String name = String(GetValue(L"name", expressionStringRef).c_str());
    //    String bitmapName = String(GetValue(L"bitmap", expressionStringRef).c_str());
    //    double pushpower = String(GetValue(L"pushpower", expressionStringRef).c_str()).ToDouble();
    //    Arrow* tmpArrow = new Arrow(position, bitmap_manager::instance()->LoadBitmapFile(bitmapName));
    //    tmpArrow->SetPushPower(pushpower);
    //    tmpArrow->SetName(name);
    //    m_EntityListPtr->Add(tmpArrow);
    //    game_engine::instance()->ConsolePrintString(name + String(" succesfully created!"));
    //}
}

std::string FileManager::GetValue(const std::string& nameRef, XMLElement* el)
{
    if (auto attr = el->FindAttribute(nameRef.c_str()); attr)
    {
        return attr->Value();
    }
    return "";
}

std::wstring FileManager::GetValue(const std::wstring& nameRef, const std::wstring& objectRef)
{
	int idStart = int(objectRef.find(nameRef) + int(nameRef.size()) + 2);
	int idEnd = int(objectRef.find('\"', idStart + 1));
	return objectRef.substr(idStart, idEnd - idStart);
}

//! Converts a string to double2
DOUBLE2 FileManager::StringToDouble2(const std::wstring& valueRef)
{
    int idSeperator = int(valueRef.find(L","));
    int length = int(valueRef.size());
    String firstNumber = String(valueRef.substr(0, idSeperator).c_str());
    String secondNumber = String(valueRef.substr(idSeperator + 1, 1 + length - idSeperator).c_str());
    return DOUBLE2(firstNumber.ToInteger(), secondNumber.ToInteger());
}

DOUBLE2 FileManager::StringToDouble2(const std::string& valueRef)
{
	int idSeperator = int(valueRef.find(","));
	int length = int(valueRef.size());
	String firstNumber = String(valueRef.substr(0, idSeperator).c_str());
	String secondNumber = String(valueRef.substr(idSeperator + 1, 1 + length - idSeperator).c_str());
	return DOUBLE2(firstNumber.ToInteger(), secondNumber.ToInteger());
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
    safe_delete(m_TriggerListPtr);
    safe_delete(m_LevelEndPtr);
    safe_delete(m_CoinListPtr);
    safe_delete(m_AnimationListPtr);
    safe_delete(m_EntityListPtr);
    safe_delete(m_EnemyListPtr);
    safe_delete(m_EnemyListPtr);

    safe_delete(m_LevelPtr);
    safe_delete(m_CameraPtr);
    safe_delete(m_AvatarPtr);
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
FileManager::KeyMap FileManager::LoadAvatarKeybinds(const std::string& filePath)
{
	KeyMap tmpKeyBindsArr{};
	tinyxml2::XMLDocument document{};
	if (document.LoadFile(filePath.c_str()) != XML_SUCCESS)
	{
		game_engine::instance()->MessageBox(String("Failed to open ") + String(filePath.c_str()));
	}

	if (auto avatar_controls = document.FirstChildElement("AvatarControls"); avatar_controls != nullptr)
	{
        for (auto action = avatar_controls->FirstChildElement("action"); action; action = action->NextSiblingElement("action"))
        {
            std::string action_name = action->FindAttribute("name")->Value();
            UINT32 scan_code = action->FindAttribute("scan_code")->IntValue();
            tmpKeyBindsArr[action_name] = scan_code;
        }
	}
	m_KeyBindsArr = tmpKeyBindsArr;
	return tmpKeyBindsArr;
}
//! Sets the keysbinds for the avatar
void FileManager::SetKeyBinds(KeyMap& keyBinds, std::string filePath)
{
    std::wifstream inputConfigFile;
    std::wstringstream filebuffer;

    tinyxml2::XMLDocument document;
    if (document.LoadFile(filePath.c_str()) != XML_SUCCESS)
    {
        game_engine::instance()->ConsolePrintString(String("Failed to set keybinds!"));
        return;
    }

    XMLElement* root = document.RootElement();
    if (XMLElement* avatar_controls = root->FirstChildElement("AvatarControls"); avatar_controls)
    {
        for(XMLElement* action = avatar_controls->FirstChildElement("action"); action; action = action->NextSiblingElement("action"))
		{
            action->SetAttribute("scan_code", keyBinds[action->FindAttribute("name")->Value()]);
		}
	}

    if(document.SaveFile(filePath.c_str()) != XML_SUCCESS)
    {
        game_engine::instance()->MessageBox(String("Failed to open the config file and update keybinds"));
        return;
    }

    m_KeyBindsArr = keyBinds;
}
//! Returns the keybinds as a vector
FileManager::KeyMap const& FileManager::GetKeyBinds()
{
    return m_KeyBindsArr;
}
//! Loads a list of levels from the filePath
LevelList* FileManager::LoadLevels(const std::string& filePath)
{
    LevelList* tmpLevelList = new LevelList();
    tinyxml2::XMLDocument document{};
    if(document.LoadFile(filePath.c_str()) != XML_SUCCESS)
    {
        game_engine::instance()->MessageBox(String("Failed to open ") + String(filePath.c_str()));
        return nullptr;
    }
    else
    {
        game_engine::instance()->ConsolePrintString(String("Succesfully opened GameInit.txt."));
    }
    auto root = document.FirstChildElement("LevelList");
    if (root)
    {
		for (auto it = root->FirstChildElement("Level"); it; it = it->NextSiblingElement())
		{
			auto attr = it->FindAttribute("path");

            if (attr)
            {
				std::string filePath = attr->Value();
				tmpLevelList->Add(filePath);
				game_engine::instance()->ConsolePrintString(String("Added ") + String(filePath.c_str()) + String(" to the levelList."));

            }

		}
    }

    return tmpLevelList;
}

//! Saves the game result in the file GameResults.txt
void FileManager::SaveGameResults(tm beginTime, tm endTime, Game* gamePtr)
{
    std::wofstream fileStream;

    fileStream.open(L"./Resources/GameResults.txt", std::ios_base::app);
    if (fileStream.fail() == true)
    {
        game_engine::instance()->MessageBox(String("Failed to save in ./Resources/GameResults.txt"));

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
        game_engine::instance()->MessageBox(String(" Could not read GameResults.txt"));
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
            std::size_t startId = tmpstring.find(L"<Start>");
            std::size_t endId = tmpstring.find(L"</Start>");
            std::wstring startBlock = tmpstring.substr(startId, (endId + std::size_t(String("</Start>").Length()))- startId);
           
            // Extract the end block
            startId = tmpstring.find(L"<End>");
            endId = tmpstring.find(L"</End>");
            std::wstring endBlock = tmpstring.substr(startId, (endId + std::size_t(String("</End>").Length())) - startId);
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
    
    std::size_t idStart = 0;
    std::size_t idEnd = date.find(L":");
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
    
    std::size_t idStart = 0;
    std::size_t idEnd = date.find(L":");
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
    std::size_t idStart = objectRef.find(L"<" + nameRef + L">") + nameRef.length() + 2;
    std::size_t idEnd = objectRef.find(L"</" + nameRef + L">");

    std::wstring value = objectRef.substr(idStart, idEnd - idStart);
    return value;

}

//! Updates the m_LastLevel variable for use when saving and reading results
void FileManager::UpdateLastLevel(int lastLevel)
{
    m_LastLevel = lastLevel;
}