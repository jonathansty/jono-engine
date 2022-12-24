#pragma once

#include "PrecisionTimer.h"
#include "ShaderCompiler.h"
#include "singleton.h"

namespace Helpers
{

inline void SetDebugObjectName(IDXGIObject* obj, std::string const& name)
{
	ENSURE_HR(obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.c_str()));
}

inline  void SetDebugObjectName(ID3D11DeviceChild* res, std::string const& name)
{
	ENSURE_HR(res->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(name.size()), name.c_str()));
}

} // namespace Helpers

enum class DepthStencilState : u32
{
	Default,
	GreaterEqual = Default,
	Equal,
	LessEqual,
	NoDepth,
	Num
};
ENUM_UNDERLYING_TYPE(DepthStencilState);
const char* DepthStencilStateToString(DepthStencilState state);

enum class BlendState : u32
{
	Default,
	AlphaBlend,
	Num
};
ENUM_UNDERLYING_TYPE(BlendState);
const char* BlendStateToString(BlendState state);


enum class RasterizerState : u32
{
	Default,
	CullNone = Default,
	CullFront,
	CullBack,
	Num
};
ENUM_UNDERLYING_TYPE(RasterizerState);
const char* RasterizerStateToString(RasterizerState state);



enum class SamplerState : u32
{
	MinMagMip_Linear,
	MinMagMip_Point,
	MinMagMip_LinearClamp,
	MinMagMip_PointClamp,
	Num
};
ENUM_UNDERLYING_TYPE(SamplerState);
const char* SamplerStateToString(SamplerState state);


namespace Graphics
{
	struct ShaderCreateParams;

extern bool s_EnableShadowRendering;
extern bool s_EnableCSM0;
extern bool s_EnableCSM1;
extern bool s_EnableCSM2;
extern bool s_EnableCSM3;


struct DeviceContext;

// Entry point for the graphics. Initializes default D3D11 objects for usage later
void init(DeviceContext const& ctx);

// On shutdown the application should call this to release all handles to the device, context and the common states.
void deinit();

// Public API to retrieve the currently initialized graphics data and common states
ENGINE_API ComPtr<ID3D11Device> get_device();
ENGINE_API ComPtr<ID3D11DeviceContext> get_ctx();
ENGINE_API ComPtr<ID3D11BlendState> GetBlendState(BlendState blendState);
ENGINE_API ComPtr<ID3D11RasterizerState> GetRasterizerState(RasterizerState rasterizerState);
ENGINE_API ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DepthStencilState blendState);
ENGINE_API ComPtr<ID3D11SamplerState> GetSamplerState(SamplerState blendState);

ENGINE_API std::shared_ptr<class Shader> get_error_shader_px();
ENGINE_API std::shared_ptr<class Shader> get_error_shader_vx();

template <typename T>
HRESULT set_debug_name(T* obj, std::string const& n)
{
	return obj->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(n.size()), n.data());
}



} // namespace Graphics

