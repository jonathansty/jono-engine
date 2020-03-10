#include "stdafx.h"
#include "TextureResource.h"

#include <DirectXTK/WICTextureLoader.h>

TextureResource::TextureResource(FromFileResourceParameters params) : TCachedResource(params)
{

}

void TextureResource::load()
{
	std::string const& path = get_init_parameters().path;
	auto device = GameEngine::Instance()->GetD3DDevice();
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	std::wstring wpath = std::wstring(path.begin(), path.end());
	SUCCEEDED(DirectX::CreateWICTextureFromFile(device, wpath.c_str(), _resource.GetAddressOf(), _srv.GetAddressOf()));
}
