#include "engine.pch.h"
#include "Material.h"

#include "GameEngine.h"
#include "ModelResource.h"
#include "TextureResource.h"


std::unique_ptr<Material> Material::create(std::shared_ptr<Graphics::Shader> vertex_shader, std::shared_ptr<Graphics::Shader> pixel_shader)
{
	auto obj = std::make_unique<Material>();
	obj->_vertex_shader = vertex_shader;
	obj->_pixel_shader = pixel_shader;
	return obj;
}

extern int g_DebugMode;

void Material::apply()
{
	auto ctx = Graphics::get_ctx();
	ctx->VSSetShader(_vertex_shader->as<ID3D11VertexShader>().Get(), nullptr, 0);
	ctx->IASetInputLayout(_vertex_shader->get_input_layout().Get());

	ctx->PSSetShader(_pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);

	if (g_DebugMode)
	{
		ctx->PSSetShader(_debug_pixel_shader->as<ID3D11PixelShader>().Get(), nullptr, 0);
	}

	// Bind material parameters
	std::array<ID3D11ShaderResourceView const*, 3> views{};
	for (std::size_t i = 0; i < _textures.size(); ++i)
	{
		if (_textures[i])
		{
			Texture const* texture = _textures[i]->get();
			views[i] = texture->get_srv();
		}
	}
	ctx->PSSetShaderResources(0, (UINT)views.size(), (ID3D11ShaderResourceView**)views.data());
}
