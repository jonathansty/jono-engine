#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"
#include "Graphics/Graphics.h"


class OrbitCamera;
class FreeCam;	

class SceneViewer final : public AbstractGame
{
public:
	using Super = AbstractGame;

	SceneViewer(std::string const& path);
	virtual ~SceneViewer();

	virtual void configure_engine(EngineCfg &engineSettings) override;
	virtual void initialize(GameCfg& gameSettings);
	virtual void start() override;
	virtual void end() override;

	#if FEATURE_D2D
	virtual void paint(Graphics::D2DRenderContext& ctx) override;
	#endif
	virtual void tick(double deltaTime) override;
	virtual void debug_ui() override;

private:

	void open_file();
	void rebuild_shaders();

	bool load_world(const char* path);
	void save_world(const char* path);

	void swap_model(const char* path);

	// Initial scene path to view 
	std::string _scene_path;

	// World to store our scene data in
	std::shared_ptr<framework::World> _world;
	shared_ptr<RenderWorldLight> _light;
	shared_ptr<RenderWorldInstance> _model;

	// Orbit camera behaviour
	s32 _camera_type = 0;
	OrbitCamera* _camera;
	FreeCam* _freecam;

	framework::EntityDebugOverlay* _overlay;

	float _light_tick;
};


