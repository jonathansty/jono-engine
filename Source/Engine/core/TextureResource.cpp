#include "engine.pch.h"

#include "TextureResource.h"

#include "GameEngine.h"

#include <algorithm>


std::array<TextureHandle, TextureHandle::DefaultTexture::Count> TextureHandle::s_default_textures;

TextureHandle::TextureHandle(FromFileResourceParameters params)
		: TCachedResource(params)
{
}

void TextureHandle::init_default()
{
	TextureHandle::black();
	TextureHandle::white();
	TextureHandle::default_normal();
	TextureHandle::default_roughness();
}

void TextureHandle::deinit()
{
	for (u32 i = 0; i < DefaultTexture::Count; ++i)
	{
		s_default_textures[i] = TextureHandle::invalid();
	}
}

TextureHandle TextureHandle::black()
{
	auto& s_tex = s_default_textures[DefaultTexture::Black];
	if (!s_tex.IsValid())
	{
		s_tex = TextureHandle();
		std::array<u32, 4 * 4> data{};
		std::fill(data.begin(), data.end(), 0);
		s_tex.create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}

	return s_tex;
}

TextureHandle TextureHandle::white()
{
	auto& s_white = s_default_textures[DefaultTexture::White];
	if (!s_white.IsValid())
	{
		s_white = TextureHandle();
		std::array<u32, 4 * 4> data{};
		std::fill(data.begin(), data.end(), 0xFFFFFFFF);
		s_white.create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_white;
}

TextureHandle TextureHandle::default_normal()
{
	auto& s_default_normal = s_default_textures[DefaultTexture::Normal];
	if (!s_default_normal.IsValid())
	{
		s_default_normal = TextureHandle();
		std::array<u32, 16> data{};
		std::fill(data.begin(), data.end(), 0xFFFF7D7D);
		s_default_normal.create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_default_normal;
}

TextureHandle TextureHandle::default_roughness()
{
	auto& s_tex = s_default_textures[DefaultTexture::Roughness];
	if (!s_tex.IsValid())
	{
		s_tex = TextureHandle();
		std::array<u32, 16> data{};
		std::fill(data.begin(), data.end(), 0xFF007DFF);
		s_tex.create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_tex;
}

bool TextureHandle::load(enki::ITaskSet* parent)
{
	std::string const& path = get_init_parameters().path;
	_resource = std::make_shared<Texture>();
	return _resource->Load(path);
}

void TextureHandle::create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data)
{
	_resource = std::make_shared<Texture>();
	_resource->LoadFromMemory(width, height, format, type, data);
	
	SetStatus(ResourceStatus::Loaded);
}


void Texture::LoadFromMemory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data, const char* debug_name)
{
	RenderInterface* ri = GetRI();

	m_Desc = CD3D11_TEXTURE2D_DESC(format, width, height);
    m_Desc.MipLevels = m_Desc.ArraySize = 1;

	D3D11_SUBRESOURCE_DATA tex_data{};
	tex_data.pSysMem = data;
	tex_data.SysMemPitch = sizeof(u32) * width;

    m_Resource = ri->CreateTexture(m_Desc, data, debug_name ? debug_name : "Texture::CreateFromMemory");
    ASSERT(m_Resource.IsValid());

	D3D11_SRV_DIMENSION dim = D3D11_SRV_DIMENSION_UNKNOWN;
	switch(type)
    {
        case TextureType::Tex1D:
            dim = D3D11_SRV_DIMENSION_TEXTURE1D;
            break;
        case TextureType::Tex2D:
            dim = D3D11_SRV_DIMENSION_TEXTURE2D;
            break;
        case TextureType::Tex3D:
            dim = D3D11_SRV_DIMENSION_TEXTURE3D;
            break;
        default:
            break;
    
	}

    CD3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(dim, format);
    m_SRV = ri->CreateShaderResourceView(m_Resource, viewDesc, debug_name ? debug_name : "Texture::CreateFromMemory");
}

void Texture::LoadFromRaw(GraphicsResourceHandle resource, bool createSRV, bool createRTV, bool createUAV, std::string_view debugName)
{
    m_Resource = resource;
	m_Desc = GetRI()->GetTexture2DDesc(m_Resource);

	if(createSRV)
    {
        m_SRV = GetRI()->CreateShaderResourceView(m_Resource, debugName);
    }

	if(createRTV)
    {
        m_RTV = GetRI()->CreateRenderTargetView(m_Resource, debugName);
	}

	if(createUAV)
    {
        m_UAV = GetRI()->CreateUnorderedAccessView(m_Resource, debugName);
	}
}

void Texture::Create(D3D11_TEXTURE2D_DESC desc, bool createSRV /*= true*/, bool createRTV /*= true*/, bool createUAV /*= true*/, std::string_view debug_name /*= ""*/)
{
    m_Resource = GetRI()->CreateTexture(desc, nullptr, debug_name);
    m_Desc = GetRI()->GetTexture2DDesc(m_Resource);

    if (createSRV)
    {
        m_SRV = GetRI()->CreateShaderResourceView(m_Resource, debug_name);
    }

    if (createRTV)
    {
        m_RTV = GetRI()->CreateRenderTargetView(m_Resource, debug_name);
    }

    if (createUAV)
    {
        m_UAV = GetRI()->CreateUnorderedAccessView(m_Resource, debug_name);
    }
}

Texture::~Texture()
{
    GetRI()->ReleaseResource(m_Resource);
    GetRI()->ReleaseResource(m_SRV);
    GetRI()->ReleaseResource(m_RTV);
    GetRI()->ReleaseResource(m_UAV);
}

bool Texture::Load(std::string const& path)
{
	int x, y, comp;

	std::string p = GetGlobalContext()->m_PlatformIO->ResolvePath(path);
	stbi_uc* data = stbi_load(p.c_str(), &x, &y, &comp, 4);
	ASSERTMSG(data, "Failed to  load image from {}", path);

	this->LoadFromMemory(x, y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TextureType::Tex2D, (void*)data, path.c_str());
	stbi_image_free(data);

    return true;
}
