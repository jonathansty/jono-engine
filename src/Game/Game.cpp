//-----------------------------------------------------
// Name: Steyfkens
// First name: Jonathan
// Group: 1DAE01
//-----------------------------------------------------
#include "game.stdafx.h"		
	
#include "Game.h"
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
#include "LevelList.h"
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
#include "DataManager.h"
#include "Slicer.h"
#include "NpcHinter.h"
#include "SoundManager.h"

Game::Game(ElectronicJonaJoy* owner)
    :_owner(owner)
{
    // Reference all the lists from the file manager
    m_FileManagerPtr = _owner->GetFileManager();


    m_CheckPointBgPtr = new CheckPointBg(float2(-6000, 0),BitmapManager::instance()->load_image(String("Resources/Animations/CheckPointBg.png")));
    m_CheckPointRotLightPtr = new RotLight(float2(-6000, 0));
    m_CheckPointRotLightPtr->SetColor(COLOR(255, 255, 255));
    m_CheckPointRotLightPtr->SetRadius(150);
    m_AttackBeamListPtr = new AttackBeamList(); 
}

Game::~Game()
{
	safe_delete(m_AttackBeamListPtr);
	safe_delete(m_CheckPointBgPtr);
	safe_delete(m_CheckPointRotLightPtr);
    safe_delete(m_HudPtr);
}


void Game::on_activate()
{
    // Load the first level
    std::string level_path = _owner->get_level_names()->GetLevel(_owner->get_curr_level());
    LoadLevel(level_path);

    m_World = framework::World::create();

	// TODO: Populate world from file

    m_HudPtr = new HUD(this);
}

void Game::on_deactivate()
{

}

void Game::render_2d(graphics::D2DRenderContext& ctx)
{
	paint(ctx);
}

void Game::update(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

	if(m_GameState == GameState::Running)
	{
		m_World->update(dt);
	}

	tick(deltaTime);
}

void Game::tick(double deltaTime)
{
	m_HudPtr->SetTime(GetAccuTime());
	m_HudPtr->Tick(deltaTime);

    if (m_GameState == GameState::Running)
    {
        m_TotalTime += deltaTime;
        m_AccuTime += deltaTime;

        //Fade in and fade out of the soundv
        if (!(SoundManager::instance()->isMusicMuted())&& !(SoundManager::instance()->isSoundMuted()))
        {
            SoundManager::instance()->FadeIn(m_SndBgMusicPtr, deltaTime);
            if (m_CameraPtr->GetCameraShakeMode() == Camera::Shakemode::EPICEFFECT)
            {
                SoundManager::instance()->FadeOut(m_SndBgMusicPtr, deltaTime);
            }
        }
        UpdateDrawMode();
        UpdateObjects(deltaTime);
        UpdateGameChecks(deltaTime);
    }

    UpdateKeyChecks(deltaTime);
        
}
void Game::UpdateObjects(double deltaTime)
{

    int AmountOfCoinsToAdd = m_CoinListPtr->IsHit();
    int oldAmount = m_AvatarPtr->GetAmountOfCoins();
    if (oldAmount != m_AvatarPtr->GetAmountOfCoins() + AmountOfCoinsToAdd)
    {
        m_AvatarPtr->AddAmountOfCoins(AmountOfCoinsToAdd);
    }
    m_CameraPtr->Tick(deltaTime);

    m_CheckPointBgPtr->Tick(deltaTime);
    m_CheckPointRotLightPtr->Tick(deltaTime);
    m_AvatarPtr->Tick(deltaTime);
    m_AnimationListPtr->Tick(deltaTime);
    m_AttackBeamListPtr->Tick(deltaTime);
    m_CoinListPtr->Tick(deltaTime);
    m_EntityListPtr->Tick(deltaTime);
    m_LevelEndPtr->Tick(deltaTime);
    m_TriggerListPtr->Tick(deltaTime);
    
    //Checking which entity is hit.
    // In the if we check if it's a checkpoint and draw the animation if it's hit.
    Entity* tmpHitEntity = m_EntityListPtr->isHit();
    if (tmpHitEntity != nullptr)
    {
        if (tmpHitEntity->GetActor()->GetName() == String("CheckPoint"))
        {
            if (m_EntityLastHitCheckpointPtr != nullptr)
            {
                m_EntityLastHitCheckpointPtr = nullptr;
            }
            m_EntityLastHitCheckpointPtr = (CheckPoint*)tmpHitEntity;
            if (hlslpp::any(m_AvatarPtr->GetRespawnPosition() != tmpHitEntity->GetPosition()))
            {
                m_AvatarPtr->SetSpawnPosition(tmpHitEntity->GetPosition());
            }
            
            if (hlslpp::any(m_CheckPointBgPtr->GetPosition() != tmpHitEntity->GetPosition()))
            {
                m_CheckPointBgPtr->SetPosition(tmpHitEntity->GetPosition());
                m_CheckPointRotLightPtr->SetPosition(tmpHitEntity->GetPosition());
                m_CheckPointRotLightPtr->SetDrawState(RotLight::drawState::SPAWNING);
                
            }
        }
    }
    m_EnemyListPtr->Tick(deltaTime);
    // Checking which enemy is hit and displaying the animations according to it
    Enemy* tmpHitEnemy = m_EnemyListPtr->IsHit();
    if (tmpHitEnemy != nullptr)
    {
        if (tmpHitEnemy->GetActor()->GetName() != String("EnemyRocketLauncher") && tmpHitEnemy->GetActor()->GetName() != String("EnemyLaser") && tmpHitEnemy->GetActor()->GetName() != String("EnemyHorizontal"))
        {
            CombRotLightCpBg* tmpLightPtr = new CombRotLightCpBg(tmpHitEnemy->GetPosition(), 100, BitmapManager::instance()->load_image(String("Resources/Animations/CheckPointBg.png")), COLOR(255, 255, 255));
            m_AnimationListPtr->Add(tmpLightPtr);
            m_AnimationListPtr->Add(new EntityDestroy(tmpHitEnemy->GetPosition()));
            m_EnemyListPtr->Remove(tmpHitEnemy);
            tmpHitEnemy = nullptr;
        }
        if (tmpHitEnemy->GetActor()->GetName() == String("EnemyHorizontal"))
        {
            EnemyHorizontal* tmpEnemyHorizontal = reinterpret_cast<EnemyHorizontal*>(tmpHitEnemy);
            m_AnimationListPtr->Add(new EntityDestroy(tmpHitEnemy->GetPosition()));
            if (tmpEnemyHorizontal->GetLifes() <= 0)
            {
                CombRotLightCpBg* tmpLightPtr = new CombRotLightCpBg(tmpHitEnemy->GetPosition(), 100, BitmapManager::instance()->load_image(String("Resources/Animations/CheckPointBg.png")), COLOR(255, 255, 255));
                m_AnimationListPtr->Add(tmpLightPtr);
                m_EnemyListPtr->Remove(tmpEnemyHorizontal);
                tmpHitEnemy = nullptr;
            }     
        }
    }
}
void Game::UpdateGameChecks(double deltaTime)
{
    if (m_LevelEndPtr->isHit())
    {
        _owner->LoadNextLevel(this);
    }
    if (m_AvatarPtr->GetLifes() <= 0)
    {
        if (m_GameOver == false)
        {
            m_CameraPtr->SetCameraMode(Camera::controlState::MANUAL);
            if (m_CameraPtr->GetCameraShakeMode() != Camera::Shakemode::EPICEFFECT)
            {
                m_CameraPtr->SetCameraShakeMode(Camera::Shakemode::ATTACKSHAKE);
            }
            
        }
        m_GameOver = true;
       
        
        
    }

    float2 dimension = m_CameraPtr->GetCameraDimension();
    float2 position = m_CameraPtr->GetCameraPosition();
    float2 direction = m_CameraPtr->GetCameraDirection();
    if (m_GameOver == true && m_AvatarPtr->GetPosition().y < position.y - dimension.y/2)
    {
        Restart();
    }
}
void Game::UpdateKeyChecks(double deltaTime)
{
	auto engine = GameEngine::instance();
    // Process input for when the game is running
    if (m_GameState == GameState::Running)
    {
        if (engine->is_key_pressed('R') && !(engine->is_key_down(VK_CONTROL)))
        {
            Restart();
        }
        if (engine->is_key_pressed('X') && m_AvatarPtr->GetMoveState() == Avatar::moveState::ATTACK)
        {
            AttackBeam* tmpBeam = new AttackBeam(m_AvatarPtr->GetPosition());
            tmpBeam->SetLevel(m_LevelPtr);
            tmpBeam->SetGroundBitmap(String("Resources/Animations/AttackBeamGround.png"));
            m_AttackBeamListPtr->Add(tmpBeam);
        }
        if (engine->is_key_pressed('L') && engine->is_key_pressed('K'))
        {
            Camera::Shakemode tmpShakeMode = m_CameraPtr->GetCameraShakeMode();
            switch (tmpShakeMode)
            {
            case Camera::Shakemode::NOSHAKE:
                m_SndEpicModePtr->play();
                m_CameraPtr->SetCameraShakeMode(Camera::Shakemode::EPICEFFECT);
                break;
            case Camera::Shakemode::EPICEFFECT:
                m_SndEpicModePtr->stop();
                m_SndBgMusicPtr->play();
                m_CameraPtr->SetCameraShakeMode(Camera::Shakemode::NOSHAKE);
                break;
            default:
                break;
            }

        }
        if (engine->is_key_pressed(VK_F5))
        {
            float2 avatarPosition = m_AvatarPtr->GetPosition();
            float2 avatarRespawnPosition = m_AvatarPtr->GetRespawnPosition();
            LoadLevel(m_Level);
            m_AvatarPtr->SetPosition(avatarPosition);
            m_AvatarPtr->SetSpawnPosition(avatarRespawnPosition);
            if (m_CameraPtr->GetCameraMode() == Camera::controlState::FOLLOWAVATAR)
            {
                m_CameraPtr->SetCameraPosition(m_AvatarPtr->GetPosition());
            }
            
        }
        if (engine->is_key_pressed(VK_F11))
        {
            m_TimeMultiplier += 1;
            engine->print_string(String("the game runs ") + String(m_TimeMultiplier) + String(" times faster."));
        }
        if (engine->is_key_pressed(VK_F10))
        {
            m_TimeMultiplier -= 1;
            engine->print_string(String("the game runs ") + String(m_TimeMultiplier) + String(" times slower."));
        }
    }

	if (engine->is_key_pressed(VK_ESCAPE))
	{
		switch (m_GameState)
		{
		case Game::GameState::Running:
			Pause();
			m_GameState = GameState::Paused;
			break;
		case Game::GameState::Paused:
			UnPause();
			m_GameState = GameState::Running;
			break;
		default:
			break;
		}
	}
	if (engine->is_key_pressed(VK_F6))
	{
		auto cameraMatrix = hlslpp::inverse(m_CameraPtr->GetViewMatrix());

        // Transform mouse position into world coordinates
		float2 mousePosition = engine->get_mouse_pos_in_viewport();
		mousePosition = hlslpp::mul(cameraMatrix,float3(mousePosition.x, mousePosition.y, 1.0)).xy;

        engine->print_string(String("[") +
			String(mousePosition.x) +
			String(", ") +
			String(mousePosition.y) + String("]"));

		if (m_EntityLastHitCheckpointPtr != nullptr)
		{
            engine->print_string(String("The camera position is: "));
            //engine->print_string(m_CameraPtr->GetCameraPosition().ToString());
		}

	}
}
void Game::UpdateDrawMode()
{
    auto engine = GameEngine::instance();
    if (engine->is_key_pressed('P'))
    {
        if(m_DrawMode == DrawMode::Physics)
        {
            m_DrawMode = DrawMode::Bitmap;
            engine->enable_physics_debug_rendering(false);
            engine->print_string(String("Draw mode is now changed to PhysicsActors and bitmaps."));
        }
        else if(m_DrawMode == (DrawMode::Physics | DrawMode::Bitmap))
        {
            m_DrawMode = DrawMode::Physics;
            engine->enable_physics_debug_rendering(true);
            engine->print_string(String("Draw mode is now changed to Bitmaps."));
        }
        else if(m_DrawMode == DrawMode::Bitmap)
        {
            m_DrawMode = DrawMode::Physics | DrawMode::Bitmap;
            engine->enable_physics_debug_rendering(true);
            engine->print_string(String("Draw mode is now changed to PhysicsActors only."));
        }
    }
}
void Game::paint(graphics::D2DRenderContext& ctx)
{
	auto engine = GameEngine::instance();

    assert(m_CameraPtr);
    float3x3 matView = m_CameraPtr->GetViewMatrix();
   
    //TODO: Refactor these lists into an actual systems (entity system or actors)
    if (m_DrawMode & DrawMode::Bitmap)
    {
        ctx.set_view_matrix(float3x3::identity());
        drawBackgroundGradient(ctx,15);
        ctx.set_view_matrix(matView);

        m_CheckPointRotLightPtr->Paint(ctx);
		m_CheckPointBgPtr->Paint(ctx);
        
        m_AnimationListPtr->Paint(ctx);
        m_AttackBeamListPtr->Paint(ctx);
        
        m_AvatarPtr->Paint(ctx);
        m_LevelEndPtr->Paint(ctx);
        m_EntityListPtr->Paint(ctx);
        m_EnemyListPtr->Paint(ctx);
        m_CoinListPtr->Paint(ctx);
        m_LevelPtr->Paint(ctx);
        m_EnemyListPtr->PaintRockets(ctx);
        ctx.set_world_matrix(hlslpp::inverse(matView));
        m_CameraPtr->Paint(ctx);
        ctx.set_color(COLOR(0, 0, 0));
    }

    if(m_DrawMode & DrawMode::Physics)
    {
        m_EnemyListPtr->PaintDebug(ctx);
        m_CoinListPtr->PaintDebug(ctx);
        m_EntityListPtr->PaintDebug(ctx);
        m_AvatarPtr->PaintDebug(ctx);
        ctx.set_view_matrix(float3x3::identity());
        ctx.set_view_matrix(matView);
        ctx.set_world_matrix(hlslpp::inverse(matView));
    }

    m_HudPtr->Paint(ctx);
}

void Game::Initializeall(const std::string& fileName)
{
    m_CheckPointBgPtr->SetPosition(float2(-650, -300));
    m_CheckPointRotLightPtr->SetPosition(float2(-650, -300));
    m_FileManagerPtr->ClearLists();


    m_AttackBeamListPtr->Remove();

    m_EntityLastHitCheckpointPtr = nullptr;
    m_FileManagerPtr->ReadGameInit(fileName);
    
    m_SndEpicModePtr = SoundManager::instance()->LoadMusic(String("Resources/Sound/BgMusic/Exhilarate.mp3"));
    // ----- Set The Items ---- //
    m_AvatarPtr = m_FileManagerPtr->GetAvatar();
    m_LevelPtr = m_FileManagerPtr->GetLevel();
    m_CameraPtr = m_FileManagerPtr->GetCamera();
    m_TriggerListPtr = m_FileManagerPtr->GetTriggers();
    m_LevelEndPtr = m_FileManagerPtr->GetLevelEnd();
    m_LevelEndPtr->SetAvatar(m_AvatarPtr);
    m_CoinListPtr = m_FileManagerPtr->GetCoinList();
    m_AnimationListPtr = m_FileManagerPtr->GetAnimationList();
    m_EntityListPtr = m_FileManagerPtr->GetEntityList();
    m_EnemyListPtr = m_FileManagerPtr->GetEnemyList();
    m_EnemyListPtr->SetAvatar(m_AvatarPtr);
    m_SndBgMusicPtr = m_FileManagerPtr->GetBgMusic();
    m_SndBgMusicPtr->set_repeat(true);
    m_SndBgMusicPtr->set_volume(0);
    
    if (m_HudPtr != nullptr)
    {
        m_HudPtr->LinkLevers(m_LevelEndPtr->GetLeversArray());
    }
}

void Game::LoadLevel(const std::string& filePath)
{
    m_GameState = GameState::Running;
    if (m_SndEpicModePtr != nullptr)
    {
        m_SndEpicModePtr->stop();
    }
    
    m_AccuTime = 0;
    m_Level = filePath;
    m_LoadNextLevel = false;
    m_GameOver = false;

    if (m_CheckPointBgPtr == nullptr)
    {
        m_CheckPointBgPtr = new CheckPointBg(float2(-300, 0),BitmapManager::instance()->load_image(String("Resources/Animations/CheckPointBg.png")));
    }
    Initializeall(filePath);
    m_AvatarPtr->SetKeyBinds(m_FileManagerPtr->GetKeyBinds());
    if (m_AvatarPtr != nullptr)
    {
        m_AvatarPtr->SetLifes(1);
        m_AvatarPtr->SetDeaths(0);
        
    }
    UnPause();
    GameEngine::instance()->print_string(String("Loaded level ") + String(filePath.c_str()));
    m_SndBgMusicPtr->set_volume(0);
    m_SndBgMusicPtr->play();
}

void Game::Unload()
{
    m_SndBgMusicPtr->stop();
    m_CheckPointBgPtr->SetPosition(float2(-650, -300));
    m_AttackBeamListPtr->Remove();
    m_SndEpicModePtr->stop();
    m_FileManagerPtr->ClearLists();

}
/*
* Private Methods used in different methods.
* update methods are used in the Tick.
* draw Methods are used in the Paint;
*/

void Game::drawBackgroundGradient(graphics::D2DRenderContext& ctx, int levels)
{
    if (levels > 255)
    {
        GameEngine::instance()->message_box(String("Please input a valid amount of levels(under 255)"));
        GameEngine::instance()->quit_game();
    }
    for (int i = 0; i < 255; i++)
    {
        COLOR tmpColor = COLOR(255 - (255 / levels)*i, 0, 255 - (255 / levels)*i);
        ctx.set_color(tmpColor);
        ctx.fill_rect(0, - 1 + i*GameEngine::instance()->get_height() / levels, GameEngine::instance()->get_width(), 1 + GameEngine::instance()->get_height() / levels + i * GameEngine::instance()->get_height() / levels);
        ctx.set_color(COLOR(0, 0, 0));
    }
}

void Game::Pause()
{
    GameEngine::instance()->set_physics_step(false);
}
void Game::UnPause()
{
    GameEngine::instance()->set_physics_step(true);
    
}
/**
* ReadGameInit reads the GameInit.txt file
* and creates the objects
*/
/*
* Methods for Restarting the Game and reloading
*/
void Game::Reset(const std::string& fileName)
{
    m_GameState = GameState::Running;
    m_GameOver = false;
    Initializeall(fileName);
    //m_CameraPtr->Reset(float2(m_AvatarPtr->GetPosition().x, m_CameraPtr->GetCameraPosition().y));
    
}
void Game::Restart()
{
    m_TotalDeaths++;
    m_GameState = GameState::Running;
    m_GameOver = false;
    m_AvatarPtr->Reset();
   
    m_EntityListPtr->Reset();
    m_EnemyListPtr->Reset();
    m_TriggerListPtr->Reset();
    //TODO: Fix resetting the level state for checkpoints to the state of when we reached the checkpoint
    m_FileManagerPtr->ReadGameInitForObject(m_Level, "EnemyRocketLauncher");
    m_FileManagerPtr->ReadGameInitForObject(m_Level, "Arrow");
    GameEngine::instance()->print_string(String("Respawned the avatar!"));
    if (m_EntityLastHitCheckpointPtr != nullptr)
    {
        m_CameraPtr->Reset(m_EntityLastHitCheckpointPtr->GetCameraPosition());
        m_CameraPtr->SetAngle(m_EntityLastHitCheckpointPtr->GetCameraAngle());
        
        if (m_CameraPtr->GetAngle() > M_PI || m_CameraPtr->GetAngle() < -M_PI)
        {
            m_CameraPtr->SetAngle(0);
        }
        
        //m_CameraPtr->SetCameraStartPosition(m_EntityLastHitCheckpointPtr->GetCameraPosition());
        GameEngine::instance()->print_string(String("Camera angle set to: ") + String(m_EntityLastHitCheckpointPtr->GetCameraAngle()));
    }
    else
    {
        float2 startPosition = m_CameraPtr->GetCameraStartPosition();
        m_CameraPtr->Reset(m_CameraPtr->GetCameraStartPosition());
    }
    
    
}


/*
* Getters and Setters
*/
bool Game::GetGameOver()
{
    return m_GameOver;
}
bool Game::GetLevelEnd()
{
    return m_LoadNextLevel;
}
Avatar* Game::GetAvatar()
{
    return m_AvatarPtr;
}
Level* Game::GetLevel()
{
    return m_LevelPtr;
}
double Game::GetAccuTime()
{
    return m_AccuTime;
}
int Game::GetTotalDeaths()
{
    return m_TotalDeaths;
}
double Game::GetTotalTime()
{
    return m_TotalTime;
}
void Game::SetFileManager(DataManager* tmpFileManager)
{
    m_FileManagerPtr = tmpFileManager;
}
void Game::LinkHUD(HUD* hudPtr)
{
    m_HudPtr = hudPtr;
    m_HudPtr->LinkLevers(m_LevelEndPtr->GetLeversArray());
}
