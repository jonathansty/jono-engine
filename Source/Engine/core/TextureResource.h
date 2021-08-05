#pragma once
#include "ResourceLoader.h"

class Texture {};
class TextureResource : public TCachedResource<Texture, FromFileResourceParameters>
{
public:
	TextureResource() = default;
	TextureResource(FromFileResourceParameters params);

	enum class TextureType {
		Tex1D,
		Tex2D,
		Tex3D
	};


	static std::shared_ptr<TextureResource> invalid() {
		static std::shared_ptr<TextureResource> s_invalid = std::make_shared<TextureResource>();
		return s_invalid;
	}

	static void initialise_default();
	static std::shared_ptr<TextureResource> black();

	static std::shared_ptr<TextureResource> white();

	static std::shared_ptr<TextureResource> default_normal();
	static std::shared_ptr<TextureResource> default_roughness();


	virtual void load() override;


	void create_from_memory(uint32_t width, uint32_t height,DXGI_FORMAT format,  TextureType type, void* data);

	ID3D11ShaderResourceView const* get_srv() const { return _srv.Get(); }
private:
	ComPtr<ID3D11Resource> _resource;

	// Should we use texture resource for textures create from code?
	// ComPtr<ID3D11RenderTargetView> _rtv;
	ComPtr<ID3D11ShaderResourceView> _srv;

};