#pragma once
#include "debug_overlays/OverlayManager.h"

struct ID3D11DeviceContext;

#ifdef _DEBUG
namespace Graphics
{

class Renderer;

class RendererDebugTool : public DebugOverlay
{
public:
	RendererDebugTool(Renderer* owner);

	void render_overlay() override;

	void render_3d(ID3D11DeviceContext* ctx) override;

private:
	void render_shader_tool();
	void render_debug_tool();
	Renderer* _renderer;

	bool _show_shadow_debug;

	std::shared_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> _batch;
};


}
#endif
