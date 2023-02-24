#include "engine.pch.h"
#include "EngineLoop.h"
#include "GameEngine.h"
#include "AbstractGame.h"

REGISTER_TYPE("/Types/EngineLoop", EngineLoop);
REGISTER_TYPE("/Types/EditorLoop", EditorLoop);

SERIALIZE_FN(EngineLoop) {}
SERIALIZE_FN(EditorLoop) {}

int EngineLoop::Run(cli::CommandLine cmdLine)
{
    cli::set(cmdLine);

    MemoryTracker::init();

    // Create global singletons
    GameEngine::create();

    m_Engine = GameEngine::instance();

    // Bind singletons to our global context
    GlobalContext* ctx = GetGlobalContext();
    ctx->m_TypeManager = TypeManager::instance();

    // Construct the game from type 
    AbstractGame* rawGame = ctx->m_TypeManager->CreateObject<AbstractGame>(m_GameType);
    m_Engine->m_Game = std::unique_ptr<AbstractGame>(rawGame);
    m_Engine->m_CommandLine = cmdLine;

    Startup();

    PrecisionTimer frameTimer{};
    f64 timeElapsed = 0.0;
    f64 timePrevious = 0.0;
    f64 timeLag = 0.0;

    while (m_Engine->m_IsRunning)
    {
        f64 delta = frameTimer.get_delta_time();
        frameTimer.start();

        Update(delta);

        // Update CPU only timings
        {
            JONO_EVENT("FrameLimiter");

            EngineCfg const& cfg = m_Engine->m_EngineCfg;
            if (cfg.m_MaxFrametime > 0.0)
            {
                f64 targetTimeMs = cfg.m_MaxFrametime;

                // Get the current frame time
                f64 framet = frameTimer.get_delta_time();
                f64 time_to_sleep = targetTimeMs - framet;

                Perf::PreciseSleep(time_to_sleep);
            }
        }

        frameTimer.stop();
        timeElapsed += frameTimer.get_delta_time();
    }

    Shutdown();
    return 0;
}

void EngineLoop::Startup()
{
    // #TODO: Should we have some kind of base game? Or drive the game modes in a different way with engine features?
    m_Engine->Startup();
}

void EngineLoop::Update(f64 dt)
{
    m_Engine->Update(dt);
}

void EngineLoop::Shutdown()
{
    m_Engine->Shutdown();
}

EngineLoop::EngineLoop(const char* gameType)
  : m_GameType(gameType)
  , m_Engine(nullptr)
  , m_IsRunning(false)
{
}

EngineLoop::EngineLoop()
  : m_GameType()
  , m_Engine(nullptr)
  , m_IsRunning(false)
{
}

void EditorLoop::Update(f64 dt)
{
    // #TODO: Implement pre-update

    Super::Update(dt);

    // #TODO: Implement post-update
}
