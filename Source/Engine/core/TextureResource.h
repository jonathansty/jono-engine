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

class TextureHandle : public TCachedResource<Texture, FromFileResourceParameters>
{
public:
	TextureHandle() = default;
	TextureHandle(FromFileResourceParameters params);

	virtual ~TextureHandle() {}

	static TextureHandle const& invalid()
	{
		static TextureHandle s_invalid = {};
		return s_invalid;
	}

	bool IsValid() const { return _resource.get(); }


	bool operator==(TextureHandle const& rhs)
	{
		return _resource == rhs._resource;
	}

	static void init_default();
	static void deinit();

	// Basic texture resources for when we fail to load specific textures
	static TextureHandle black();
	static TextureHandle white();
	static TextureHandle default_normal();
	static TextureHandle default_roughness();

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
	static std::array<TextureHandle, DefaultTexture::Count> s_default_textures;

};
