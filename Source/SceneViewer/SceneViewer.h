#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"
#include "Graphics/Graphics.h"

class SceneViewer final : public AbstractGame
{
public:
	SceneViewer(std::string const& path)
			: _scene_path(path)
			, _timer(0.0f)
			, _light_tick(0.0f)
			, _zoom(2.0f)
		, _up_timer(0.0f)
		,_center()
	{}
	virtual ~SceneViewer() {}

	virtual void configure_engine(EngineSettings &engineSettings) override;
	virtual void initialize(GameSettings& gameSettings);
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

	framework::EntityDebugOverlay* _overlay;

	float3 _center;
	float _zoom;
	float _timer;
	float _up_timer;
	float _light_tick;
};


