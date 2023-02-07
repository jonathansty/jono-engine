#pragma once
#include "ResourceLoader.h"

#include "Graphics/GraphicsResourceHandle.h"

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

	void Load(std::string const& path);
	void LoadFromMemory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data, const char* debug_name = nullptr);

	// Gets the raw SRV for this texture resource
	GraphicsResourceHandle GetSRV() const { return m_SRV; }

	u32 GetWidth() const { return m_Width; };
	u32 GetHeight() const { return m_Height; };

private:
	GraphicsResourceHandle m_Resource;

	// Should we use texture resource for textures create from code?
	// ComPtr<ID3D11RenderTargetView> _rtv;
	GraphicsResourceHandle m_SRV;
	u32 m_Width;
	u32 m_Height;
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
