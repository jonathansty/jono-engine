#pragma once

struct DepthStencilState {
	enum Value {
		Default,
		GreaterEqual = Default,
		Num
	};
};

struct BlendState {
	enum Value {
		Default,
		Num
	};
};

struct RasterizerState {
	enum Value {
		Default,
		CullNone = Default,
		CullFront,
		CullBack,
		Num
	};
};

struct SamplerState {
	enum Value {
		MinMagMip_Linear,
		Num
	};

};

namespace Graphics {

	// Entry point for the graphics. Initializes default D3D11 objects for usage later
	void init(ComPtr<ID3D11Device> device);


	ComPtr<ID3D11Device> GetDevice();

	// Returns the requested blend state
	ComPtr<ID3D11BlendState> GetBlendState(BlendState::Value blendState);

	// Returns the requested rasterizer state
	ComPtr<ID3D11RasterizerState> GetRasterizerState(RasterizerState::Value rasterizerState);

	// Returns the requested depth stencil state
	ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DepthStencilState::Value blendState);

	// Returns the requested sampler state 
	ComPtr<ID3D11SamplerState> GetSamplerState(SamplerState::Value blendState);

}