#pragma once
#include "ResourceLoader.h"

class TextureResource : public TCachedResource<FromFileResourceParameters>
{
public:
	TextureResource() = default;
	TextureResource(FromFileResourceParameters params);

	static std::shared_ptr<TextureResource> invalid() {
		static std::shared_ptr<TextureResource> s_invalid = std::make_shared<TextureResource>();
		return s_invalid;
	}

	virtual void load() override;

	enum class TextureType
	{
		Tex1D,
		Tex2D,
		Tex3D
	};

	ID3D11ShaderResourceView const* get_srv() const { return _srv.Get(); }
private:
	ComPtr<ID3D11Resource> _resource;

	// Should we use texture resource for textures create from code?
	// ComPtr<ID3D11RenderTargetView> _rtv;
	ComPtr<ID3D11ShaderResourceView> _srv;

};