#pragma once
#include "Debug/OverlayManager.h"

struct ID3D11DeviceContext;

namespace Graphics
{

class Renderer;

class RendererDebugTool : public DebugOverlay
{
public:
	static bool s_force_all_visible;

	RendererDebugTool(Renderer* owner);
	virtual ~RendererDebugTool() {}

	void RenderOverlay() override;

	void Render3D(ID3D11DeviceContext* ctx) override;

private:
	void render_shader_tool();
	void render_debug_tool();
	Renderer* _renderer;

	bool _show_shadow_debug;

	std::shared_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> _batch;
};


}
