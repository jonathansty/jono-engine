#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"

class Hello3D : public AbstractGame
{
public:
	void initialize(GameSettings& gameSettings);
	void start() override;
	void end() override;
	void paint(RECT rect) override;
	void tick(double deltaTime) override;
	void debug_ui() override;
	void render_3d() override;

private:
	std::shared_ptr<framework::World> _world;

	ComPtr<ID3D11Buffer> _cb_MVP;
	ComPtr<ID3D11Buffer> _cb_Debug;
	ComPtr<ID3D11DepthStencilState> _depth_state;
	ComPtr<ID3D11BlendState> _blend_state;
	ComPtr<ID3D11RasterizerState> _raster_state;

	enum class Samplers
	{
		AllLinear,
		AllPoint,
		Count
	};
	std::array<ComPtr<ID3D11SamplerState>, uint32_t(Samplers::Count)> m_Samplers;

	float _timer;
};


