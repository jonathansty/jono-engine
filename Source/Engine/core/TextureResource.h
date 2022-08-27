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

	void create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data, const char* debug_name = nullptr);

	// Gets the raw SRV for this texture resource
	ID3D11ShaderResourceView* get_srv() const { return _srv.Get(); }

	u32 get_width() const { return _width; };
	u32 get_height() const { return _height; };

private:
	ComPtr<ID3D11Resource> _resource;

	// Should we use texture resource for textures create from code?
	// ComPtr<ID3D11RenderTargetView> _rtv;
	ComPtr<ID3D11ShaderResourceView> _srv;
	u32 _width;
	u32 _height;
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

	static void init_default();
	static void deinit();

	// Basic texture resources for when we fail to load specific textures
	static std::shared_ptr<TextureResource> black();
	static std::shared_ptr<TextureResource> white();
	static std::shared_ptr<TextureResource> default_normal();
	static std::shared_ptr<TextureResource> default_roughness();

	virtual void load(enki::ITaskSet* parent) override;

	void create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data);


private:

	struct DefaultTexture
	{
		enum Enum : u32
		{
			Black,
			White,
			Normal,
			Roughness,
			Count
		};
	};
	static std::array<std::shared_ptr<TextureResource>, DefaultTexture::Count> s_default_textures;

};
