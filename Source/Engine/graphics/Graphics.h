#pragma once

struct DepthStencilState {
	enum Value {
		Default,
		GreaterEqual = Default,
		Equal,
		LessEqual,
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

	void deinit();


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

class ConstantBuffer {

public:
	enum class BufferUsage {
		Default,
		Dynamic,
		Staging,
		Immutable
	};

	static std::shared_ptr<ConstantBuffer> create(ID3D11Device* device, u32 size, bool cpu_write = false, BufferUsage usage = BufferUsage::Default, void* initialData = nullptr);

	ID3D11Buffer* Get() const { return _buffer.Get(); }

	void* map(ID3D11DeviceContext* ctx) {
		D3D11_MAPPED_SUBRESOURCE resource{};
		ctx->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		return resource.pData;

	}
	void unmap(ID3D11DeviceContext* ctx) {
		ctx->Unmap(_buffer.Get(), 0);
	}

	ConstantBuffer() {}
	~ConstantBuffer(){}

private:

	ComPtr<ID3D11Buffer> _buffer;
	u32 _size;
	bool _cpu_writeable;
	BufferUsage _usage;
};
using ConstantBufferRef = shared_ptr<ConstantBuffer>;

// --------------------------------------------------------------------------------- 
// Debug 
// --------------------------------------------------------------------------------- 

#ifdef _DEBUG
class scoped_gpu_event final {
public:
	scoped_gpu_event(ID3DUserDefinedAnnotation* annotation, std::wstring name)
			: _name(name), _annotation(annotation) {
		annotation->BeginEvent(name.c_str());
	}
	scoped_gpu_event(ID3DUserDefinedAnnotation* annotation, std::string name)
			: _annotation(annotation) {
		_name = std::wstring(name.begin(), name.end());
	}
	~scoped_gpu_event() {
		_annotation->EndEvent();
	}

private:
	std::wstring _name;
	ID3DUserDefinedAnnotation* _annotation;
};

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)
#define GPU_SCOPED_EVENT(ctx, name) scoped_gpu_event COMBINE(perfEvent, __LINE__) = scoped_gpu_event(ctx, name)
#define GPU_MARKER(ctx, name) ctx->SetMarker(name);
#else
#define GPU_SCOPED_EVENT(ctx, name)
#define GPU_MARKER(ctx, name)
#endif
