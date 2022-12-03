#include "sceneviewer.pch.h"

#if FEATURE_D2D
#include "GameTest2D.h"

#include "Components.h"
#include "Overlays.h"

void GameTest2D::configure_engine(EngineCfg& engineSettings)
{
	engineSettings.m_UseD2D = true;
	engineSettings.m_UseD2DAA = true;
}

void GameTest2D::start()
{
	_bitmaps.reserve(100);
	_bitmaps.push_back({ float4x4::identity(), Bitmap::load("Tests/Bitmaps/coinBronze.png") });
	_bitmaps.push_back({ float4x4::identity(), Bitmap::load("Tests/Bitmaps/coinSilver.png") });
	_bitmaps.push_back({ float4x4::identity(), Bitmap::load("Tests/Bitmaps/coinGold.png") });
	_bitmaps.push_back({ float4x4::identity(), Bitmap::load("Tests/Bitmaps/character.png") });

	float width_offset = 0;
	float height_offset = 0;
	float row_height = 0;
	for (u32 i = 0; i < _bitmaps.size(); ++i)
	{
		auto const& bmp = _bitmaps[i].bmp;
		float4x4 c = float4x4::translation({ width_offset, height_offset, 0.0f });

		float new_width_offset = width_offset + bmp->get_width();
		if (new_width_offset > 200)
		{
			height_offset += row_height;
			row_height = 0;
			width_offset = 0;

			c = float4x4::translation({ width_offset, height_offset, 0.0f });
		}

		_bitmaps[i].matrix = c;

		width_offset = new_width_offset;
		row_height = std::max<float>(row_height, static_cast<float>(bmp->get_height()));
	}
}

void GameTest2D::end()
{
}

void GameTest2D::paint(Graphics::D2DRenderContext& ctx)
{
	ctx.draw_background(MK_COLOR(153, 217, 234, 255));

	std::for_each(_bitmaps.begin(), _bitmaps.end(), [&ctx](auto const& el)
			{
		ctx.set_world_matrix(el.matrix);
		ctx.draw_bitmap(el.bmp.get()); });
}

void GameTest2D::tick(double deltaTime)
{
}
#endif
