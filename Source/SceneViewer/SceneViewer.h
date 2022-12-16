#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"
#include "Graphics/Graphics.h"


class OrbitCamera;
class FreeCam;	

class SceneViewer final : public AbstractGame
{
	CLASS(SceneViewer, AbstractGame);

public:
	using Super = AbstractGame;

	SceneViewer();
	SceneViewer(std::string const& path);
	virtual ~SceneViewer();

	virtual void ConfigureEngine(EngineCfg &engineSettings) override;
	virtual void ConfigureGame(GameCfg& gameSettings);
	virtual void OnStartup() override;
	virtual void OnShutdown() override;

	#if FEATURE_D2D
	virtual void OnPaint2D(Graphics::D2DRenderContext& ctx) override;
	#endif
	virtual void OnUpdate(double deltaTime) override;
	virtual void OnDebugUI() override;

private:

	void OpenFile();
	void RebuildAllShaders();

	bool LoadWorld(const char* path);
	void SaveWorld(const char* path);

	void SwapModel(const char* path);

	// Initial scene path to view 
	std::string m_ScenePath;

	// World to store our scene data in
	std::shared_ptr<framework::World> m_World;
	shared_ptr<RenderWorldLight> m_SunLight;
	shared_ptr<RenderWorldInstance> m_CurrentModel;

	// Orbit camera behaviour
	s32 m_CameraType = 0;
	OrbitCamera* m_OrbitCamera;
	FreeCam* m_FreeCamera;

	framework::EntityDebugOverlay* m_EntityOverlay;

	float m_LightT;
};


