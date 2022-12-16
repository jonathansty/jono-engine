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

void TextureHandle::load(enki::ITaskSet* parent)
{
	std::string const& path = get_init_parameters().path;
	_resource = std::make_shared<Texture>();
	_resource->load(path);
}

void TextureHandle::create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data)
{
	_resource = std::make_shared<Texture>();
	_resource->create_from_memory(width, height, format, type, data);
	
	SetStatus(ResourceStatus::Loaded);
}


void Texture::create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data, const char* debug_name)
{
	auto device = Graphics::get_device();
	auto ctx = Graphics::get_ctx();

	auto desc = CD3D11_TEXTURE2D_DESC(format, width, height);
	desc.MipLevels = desc.ArraySize = 1;

	ComPtr<ID3D11Texture2D> texture;
	D3D11_SUBRESOURCE_DATA tex_data{};
	tex_data.pSysMem = data;
	tex_data.SysMemPitch = sizeof(u32) * width;
	SUCCEEDED(device->CreateTexture2D(&desc, &tex_data, texture.GetAddressOf()));
	Helpers::SetDebugObjectName(texture.Get(), debug_name ? debug_name : "Texture::CreateFromMemory");

	auto view_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, format);
	SUCCEEDED(texture.As(&_resource));
	ENSURE_HR(device->CreateShaderResourceView(_resource.Get(), &view_desc, _srv.GetAddressOf()));
	Helpers::SetDebugObjectName(_srv.Get(), debug_name ? debug_name : "Texture::CreateFromMemory");
}

void Texture::load(std::string const& path)
{
	ComPtr<ID3D11Device> device = Graphics::get_device();
	ComPtr<ID3D11DeviceContext> ctx = Graphics::get_ctx();

	int x, y, comp;
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &comp, 4);
	ASSERTMSG(data, "Failed to  load image from {}", path);

	this->_width = u32(x);
	this->_height = u32(y);

	this->create_from_memory(x, y, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, (void*)data, path.c_str());
	stbi_image_free(data);
	// SUCCEEDED(DirectX::CreateWICTextureFromFile(device, wpath.c_str(), _resource.GetAddressOf(), _srv.GetAddressOf()));
}
