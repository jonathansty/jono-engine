#pragma once
#if  FEATURE_D2D

#include "AbstractGame.h"
#include "Framework/World.h"


using framework::World;
using framework::EntityHandle;

class GameTest2D : public AbstractGame
{
public:
	GameTest2D() = default;
	virtual ~GameTest2D() = default;

	void ConfigureEngine(EngineCfg& engineSettings);
	void ConfigureGame(GameCfg& gameSettings) {}

	void OnStartup() override;

	void OnShutdown() override;

	void OnPaint2D(Graphics::D2DRenderContext& ctx) override;

	void OnUpdate(double deltaTime) override;

	private:
		struct DrawElement {
			float4x4 matrix;
			unique_ptr<Bitmap> bmp;
		};
		vector<DrawElement> _bitmaps;
};

#endif
