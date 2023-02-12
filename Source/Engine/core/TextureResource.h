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
    ~Texture();

    void Load(std::string const& path);
    void LoadFromMemory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data, const char* debug_name = nullptr);
    void LoadFromRaw(GraphicsResourceHandle resource, bool createSRV = true, bool createRTV = true, bool createUAV = true, std::string_view debug_name = "");

	void Create(D3D11_TEXTURE2D_DESC desc, bool createSRV = true, bool createRTV = true, bool createUAV = true, std::string_view debug_name = "");

    u32 GetWidth() const { return m_Desc.Width; };
    u32 GetHeight() const { return m_Desc.Height; };

	// #TODO: Abstract texture descriptors to be cross platform
	D3D11_TEXTURE2D_DESC GetDesc() const { return m_Desc; }

    GraphicsResourceHandle const& GetRTV() const { return m_RTV; }
    GraphicsResourceHandle const& GetSRV() const { return m_SRV; }
    GraphicsResourceHandle const& GetUAV() const { return m_UAV; }
    GraphicsResourceHandle const& GetResource() const { return m_Resource; }


private:
    GraphicsResourceHandle m_Resource;
    GraphicsResourceHandle m_RTV;
    GraphicsResourceHandle m_SRV;
    GraphicsResourceHandle m_UAV;

    D3D11_TEXTURE2D_DESC m_Desc;
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
