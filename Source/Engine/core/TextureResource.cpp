#include "engine.pch.h"
#include "TextureResource.h"

#include "GameEngine.h"

#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//#include <DirectXTK/WICTextureLoader.h>

TextureResource::TextureResource(FromFileResourceParameters params) : TCachedResource(params)
{

}

std::shared_ptr<TextureResource> TextureResource::black() {
	return invalid();
}

std::shared_ptr<TextureResource> TextureResource::white() {
	static std::shared_ptr<TextureResource> s_white;
	if(!s_white) {
		s_white = std::make_shared<TextureResource>();
		std::array<u32, 4*4> data{};
		std::fill(data.begin(), data.end(), 0xFFFFFFFF);
		s_white->create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_white;
}

std::shared_ptr<TextureResource> TextureResource::default_normal() {
	static std::shared_ptr<TextureResource> s_default_normal;
	if (!s_default_normal) {
		s_default_normal = std::make_shared<TextureResource>();
		std::array<u32, 16> data{};
		std::fill(data.begin(), data.end(), 0xFFFF7D7D);
		s_default_normal->create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_default_normal;
}

std::shared_ptr<TextureResource> TextureResource::default_roughness() {
	static std::shared_ptr<TextureResource> s_default_normal;
	if (!s_default_normal) {
		s_default_normal = std::make_shared<TextureResource>();
		std::array<u32, 16> data{};
		std::fill(data.begin(), data.end(), 0xFF007DFF);
		s_default_normal->create_from_memory(4, 4, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, data.data());
	}
	return s_default_normal;
}

void TextureResource::load()
{
	std::string const& path = get_init_parameters().path;
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	int x, y, comp;
	stbi_uc* data = stbi_load(path.c_str(), &x, &y, &comp, 4); 
	ASSERTMSG(data, "Failed to  load image from {}", path);

	this->create_from_memory(x, y, DXGI_FORMAT_R8G8B8A8_UNORM, TextureType::Tex2D, (void*)data);
	stbi_image_free(data);
	//SUCCEEDED(DirectX::CreateWICTextureFromFile(device, wpath.c_str(), _resource.GetAddressOf(), _srv.GetAddressOf()));
}

void TextureResource::create_from_memory(uint32_t width, uint32_t height, DXGI_FORMAT format, TextureType type, void* data) {

	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();


	auto desc = CD3D11_TEXTURE2D_DESC(format, width, height);
	desc.MipLevels = desc.ArraySize = 1;

	ComPtr<ID3D11Texture2D> texture;
	D3D11_SUBRESOURCE_DATA tex_data{};
	tex_data.pSysMem = data;
	tex_data.SysMemPitch = sizeof(u32) * width;
	SUCCEEDED(device->CreateTexture2D(&desc, &tex_data, texture.GetAddressOf()));

	auto view_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, format);
	SUCCEEDED(texture.As(&_resource));
	device->CreateShaderResourceView(_resource.Get(), &view_desc, _srv.GetAddressOf());


}
