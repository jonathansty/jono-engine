#pragma once
#include "ResourceLoader.h"

enum class TextureType
{
	Tex1D,
	Tex2D,
	Tex3D
};

class Texture
{
public:

	Texture() = default;

	void load(std::string const& path);

	void create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data);

	// Gets the raw SRV for this texture resource
	ID3D11ShaderResourceView const* get_srv() const { return _srv.Get(); }


private:
	ComPtr<ID3D11Resource> _resource;

	// Should we use texture resource for textures create from code?
	// ComPtr<ID3D11RenderTargetView> _rtv;
	ComPtr<ID3D11ShaderResourceView> _srv;
};

class TextureResource : public TCachedResource<Texture, FromFileResourceParameters>
{
public:
	TextureResource() = default;
	TextureResource(FromFileResourceParameters params);

	static std::shared_ptr<TextureResource> invalid()
	{
		static std::shared_ptr<TextureResource> s_invalid = std::make_shared<TextureResource>();
		return s_invalid;
	}

	static void initialise_default();

	// Basic texture resources for when we fail to load specific textures
	static std::shared_ptr<TextureResource> black();
	static std::shared_ptr<TextureResource> white();
	static std::shared_ptr<TextureResource> default_normal();
	static std::shared_ptr<TextureResource> default_roughness();

	virtual void load() override;

	void create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data);


private:

};