#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"

class SceneViewer final : public AbstractGame
{
public:
	SceneViewer(std::string const& path) : _scene_path(path)
	{}
	virtual ~SceneViewer() {}

	virtual void configure_engine(EngineSettings &engineSettings) override;
	virtual void initialize(GameSettings& gameSettings);
	virtual void start() override;
	virtual void end() override;
	virtual void paint(graphics::D2DRenderContext& ctx) override;
	virtual void tick(double deltaTime) override;
	virtual void debug_ui() override;
	virtual void render_3d() override;

private:

	bool load_world(const char* path);
	void save_world(const char* path);

	// Initial scene path to view 
	std::string _scene_path;

	// World to store our scene data in
	std::shared_ptr<framework::World> _world;

	ComPtr<ID3D11Buffer> _cb_MVP;
	ComPtr<ID3D11Buffer> _cb_Debug;
	ComPtr<ID3D11DepthStencilState> _depth_state;
	ComPtr<ID3D11BlendState> _blend_state;
	ComPtr<ID3D11RasterizerState> _raster_state;

	framework::EntityDebugOverlay* _overlay;

	float _timer;
};


