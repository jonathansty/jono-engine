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
	{}
	virtual ~SceneViewer() {}

	virtual void configure_engine(EngineSettings &engineSettings) override;
	virtual void initialize(GameSettings& gameSettings);
	virtual void start() override;
	virtual void end() override;
	virtual void paint(graphics::D2DRenderContext& ctx) override;
	virtual void tick(double deltaTime) override;
	virtual void debug_ui() override;

private:

	bool load_world(const char* path);
	void save_world(const char* path);

	void swap_model(const char* path);

	// Initial scene path to view 
	std::string _scene_path;

	// World to store our scene data in
	std::shared_ptr<framework::World> _world;
	shared_ptr<RenderWorld> _render_world;
	shared_ptr<RenderWorldCamera> _camera;
	shared_ptr<RenderWorldLight> _light;
	shared_ptr<RenderWorldInstance> _model;

	framework::EntityDebugOverlay* _overlay;

	float _timer;
	float _light_tick;
};


