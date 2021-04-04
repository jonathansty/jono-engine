#pragma once

#include "AbstractGame.h"
#include "Framework/World.h"


using framework::World;
using framework::EntityHandle;

class GameTest2D : public AbstractGame
{
public:
	GameTest2D() = default;
	virtual ~GameTest2D() = default;

	void configure_engine(EngineSettings& engineSettings);
	void initialize(GameSettings& gameSettings) {}

	void start() override;

	void end() override;

	void paint(graphics::D2DRenderContext& ctx) override;

	void tick(double deltaTime) override;

	private:
		struct DrawElement {
			float3x3 matrix;
			unique_ptr<Bitmap> bmp;
		};
		vector<DrawElement> _bitmaps;
};


