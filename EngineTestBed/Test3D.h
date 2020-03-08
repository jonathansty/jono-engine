#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"

class Hello3D : public AbstractGame
{
public:
	void GameInitialize(GameSettings& gameSettings);
	void GameStart() override;
	void GameEnd() override;
	void GamePaint(RECT rect) override;
	void GameTick(double deltaTime) override;
	void DebugUI() override;
	void Render3D() override;

private:
	std::unique_ptr<framework::World> _world;

	ComPtr<ID3D11Buffer> _cb_MVP;
	ComPtr<ID3D11DepthStencilState> _depth_state;
	ComPtr<ID3D11BlendState> _blend_state;
	ComPtr<ID3D11RasterizerState> _raster_state;

	struct {
		std::shared_ptr<class TextureResource> albedo;
		std::shared_ptr<class TextureResource> roughness;
		std::shared_ptr<class TextureResource> metalness;
		std::shared_ptr<class TextureResource> normal;
	} g_Materials;

	enum class Samplers
	{
		AllLinear,
		AllPoint,
		Count
	};
	std::array<ComPtr<ID3D11SamplerState>, uint32_t(Samplers::Count)> m_Samplers;

	float _timer;
};


