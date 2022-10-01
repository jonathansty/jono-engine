#pragma once
#include "Graphics/ShaderTypes.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Core/TextureResource.h"

namespace Graphics
{
	using ShaderRef = std::shared_ptr<class Shader>;
}

namespace Graphics
{

class Shader;
class Renderer;
struct ViewParams;

}

struct ParameterInfo
{
	size_t offset; // Offset into buffer data
	size_t size; // Size in float slots (not bytes)
};

struct IMaterialObject
{
	virtual ~IMaterialObject(){};

	virtual void apply(Graphics::Renderer* renderer, Graphics::ViewParams const& params) const = 0;

	virtual u32 get_texture_count() const = 0;
	virtual u32  get_slot(Identifier64 const& slot_id) const = 0;
	virtual void get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views) const = 0;
	virtual ConstantBufferRef const& get_cb() const = 0;
	virtual std::vector<u8> const& get_param_data() const = 0;
	virtual bool is_double_sided() const = 0;

	virtual Graphics::ShaderRef const& get_vertex_shader() const = 0;
	virtual Graphics::ShaderRef const& get_pixel_shader() const = 0;
	virtual Graphics::ShaderRef const& get_debug_pixel_shader() const = 0;

	virtual ParameterInfo const* find_parameter(Identifier64 const& id) const = 0;

};

class Material  : public IMaterialObject
{
public:
	Material();

	virtual ~Material() {}

	__declspec(align(16)) struct ConstantBufferData
	{
		Shaders::float3 albedo;
		f32 roughness;
		f32 metalness;
		f32 padding[3];
	};

	static std::unique_ptr<Material> create(Graphics::ShaderRef const& vertex_shader, Graphics::ShaderRef const& pixel_shader, u32 intialDataSize, void* initialData = nullptr);

	static std::unique_ptr<Material> load(std::string const& path);

	bool is_double_sided() const override { return _double_sided; }

	ComPtr<ID3D11InputLayout> get_input_layout() const;

	Graphics::ShaderRef const& get_vertex_shader() const override { return _vertex_shader; }
	Graphics::ShaderRef const& get_pixel_shader() const override { return _pixel_shader; }
	Graphics::ShaderRef const& get_debug_pixel_shader() const override { return _debug_pixel_shader; }

	void get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views) const;

	ConstantBufferRef const& get_cb() const { return _material_cb; }

	u32 get_slot(Identifier64 const& slot_id) const { return _texture_slot_mapping.at(slot_id); }

	void set_texture(u32 slot, TextureHandle const& resource) {  _textures[slot] = resource; }

	u32 get_texture_count() const override;

	std::vector<u8> const& get_param_data() const override;

	ParameterInfo const* find_parameter(Identifier64 const& id) const override;

	void apply(Graphics::Renderer* renderer, Graphics::ViewParams const& params) const override;


private:
	bool _double_sided;

	ConstantBufferRef _material_cb;

	Graphics::ShaderRef _vertex_shader;
	Graphics::ShaderRef _pixel_shader;
	Graphics::ShaderRef _debug_pixel_shader;

	std::vector<u8> _param_data;

	std::vector<TextureHandle> _textures;
	std::unordered_map<Identifier64, u32> _texture_slot_mapping;
	std::unordered_map<Identifier64, ParameterInfo> _parameters;

	friend class MaterialHandle;
	friend class MaterialInstance;
};

class MaterialInstance final : public IMaterialObject
{
	public:
		MaterialInstance(std::shared_ptr<MaterialHandle const>const& baseMaterial);
		MaterialInstance();

		~MaterialInstance();

		void bind(IMaterialObject const* obj);

		virtual void apply(Graphics::Renderer* renderer, Graphics::ViewParams const& params) const;


		u32 get_slot(Identifier64 const& slot_id) const;

		void set_texture(u32 slot, std::shared_ptr<class TextureHandle> const& resource);
		void set_texture(Identifier64 const& slot_id, std::shared_ptr<class TextureHandle> const& tex);

		void set_param_float(Identifier64 const& parameter_id, float value);
		void set_param_float3(Identifier64 const& parameter_id, float3 value);

		void update();

		void get_texture_views(std::vector<ID3D11ShaderResourceView const*>& views) const;

		Graphics::ShaderRef const& get_vertex_shader() const { return get_material_obj()->get_vertex_shader(); }
		Graphics::ShaderRef const& get_pixel_shader() const { return get_material_obj()->get_pixel_shader(); }
		Graphics::ShaderRef const& get_debug_pixel_shader() const { return get_material_obj()->get_debug_pixel_shader(); }

		ConstantBufferRef const& get_cb() const;

		Material const* get_material() const;

		bool is_double_sided() const override;
		u32 get_texture_count() const override;
		std::vector<u8> const& get_param_data() const override;
		ParameterInfo const* find_parameter(Identifier64 const& id) const override;

		template<typename T>
		struct TParameter
		{
			Identifier64 hash;
			T value;
		};
		using FloatParameter = TParameter<float>;
		using Float2Parameter = TParameter<float2>;
		using Float3Parameter = TParameter<float3>;
		using Float4Parameter = TParameter<float4>;

	private:
		IMaterialObject const* get_material_obj()  const;


		IMaterialObject const* _obj;
		std::shared_ptr<MaterialHandle const> _resource;

		template<typename ParamType> 
		using ParameterCollection = std::vector<TParameter<ParamType>>;
		ParameterCollection<float> _float_params;
		ParameterCollection<float3> _float3_params;

		bool _has_overrides;
		bool _needs_flush;
		std::vector<u8> _param_data;

		std::vector<std::shared_ptr<class TextureHandle>> _textures;
		ConstantBufferRef _instance_cb;
};
