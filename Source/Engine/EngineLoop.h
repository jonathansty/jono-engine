#pragma once

#include "CLI/CommandLine.h"

struct ENGINE_API IEngineLoop
{
    virtual int Run(cli::CommandLine cmdLine = {}) = 0;

	virtual void Startup() = 0;
	virtual void Update(f64 dt) = 0;
	virtual void Shutdown() = 0;
};

class ENGINE_API EngineLoop : public IEngineLoop
{
	CLASS(EngineLoop, IEngineLoop)

	public:
		EngineLoop();
		EngineLoop(const char* gameType);

		int Run(cli::CommandLine cmdLine = {}) override;

		void Startup() override;
		void Update(f64 dt) override;
		void Shutdown() override;

		EngineLoop& operator=(EngineLoop const&) = delete;
		EngineLoop(EngineLoop const&) = delete;

	private:
		class GameEngine* m_Engine;
		std::string m_GameType;

		bool m_IsRunning = false;

};


class EditorLoop : public EngineLoop
{
	CLASS(EditorLoop, EngineLoop)

	public:
		void Update(f64 dt) override;

};