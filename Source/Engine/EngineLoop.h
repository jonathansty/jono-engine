#pragma once

#include "Types/TypeManager.h"

struct IEngineLoop
{
	virtual int Run() = 0;

	virtual void Startup() = 0;
	virtual void Update(f64 dt) = 0;
	virtual void Shutdown() = 0;
};

class EngineLoop : public IEngineLoop
{
	CLASS(EngineLoop, IEngineLoop)

	public:
		EngineLoop(const char* gameType = nullptr);

		int Run() override;

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