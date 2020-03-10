#include "stdafx.h"

#include "MaterialResource.h"
#include "ModelResource.h"
#include "TextureResource.h"

namespace Shaders
{
#include "Shaders/generated/simple_px.h"
#include "Shaders/generated/simple_vx.h"
#include "Shaders/generated/debug_px.h"
}

extern int g_DebugMode;

void MaterialResource::apply(ID3D11DeviceContext* ctx)
{
	ctx->VSSetShader(_vert_shader.Get(), nullptr, 0);
	ctx->PSSetShader(_pixel_shader.Get(), nullptr, 0);
	if (g_DebugMode)
	{
		ctx->PSSetShader(_debug_shader.Get(), nullptr, 0);

	}
	ctx->IASetInputLayout(_input_layout.Get());

	// Bind material parameters
	std::array<ID3D11ShaderResourceView const*, 3> views{};
	for(int i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			views[i] = _textures[i]->get_srv();
		}
	}
	ctx->PSSetShaderResources(0, views.size(), (ID3D11ShaderResourceView**)views.data());
}

void MaterialResource::load()
{
	auto device = GameEngine::Instance()->GetD3DDevice();

	// Initiate texture loads
	for(std::string const& path : get_init_parameters().m_texture_paths)
	{
		if (path.empty())
		{
			_textures.push_back(TextureResource::invalid());
		}
		else
		{
			_textures.push_back(ResourceLoader::Instance()->load<TextureResource>({ path }, true));
		}
	}

	using VertexType = ModelVertex;

	SUCCEEDED(device->CreateVertexShader(Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), nullptr, _vert_shader.GetAddressOf()));
	SUCCEEDED(device->CreateInputLayout(VertexType::InputElements, VertexType::InputElementCount, Shaders::cso_simple_vx, std::size(Shaders::cso_simple_vx), _input_layout.GetAddressOf()));

	SUCCEEDED(device->CreatePixelShader(Shaders::cso_simple_px, std::size(Shaders::cso_simple_px), nullptr, _pixel_shader.GetAddressOf()));
	SUCCEEDED(device->CreatePixelShader(Shaders::cso_debug_px, std::size(Shaders::cso_debug_px), nullptr, _debug_shader.GetAddressOf()));



}
