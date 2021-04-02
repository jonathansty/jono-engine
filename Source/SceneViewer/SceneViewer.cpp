#include "sceneviewer.pch.h"
#include <hlsl++.h>
#include "SceneViewer.h"

#include "Engine/Core/ModelResource.h"
#include "Engine/Core/TextureResource.h"
#include "Engine/Core/MaterialResource.h"

#include "Framework/framework.h"
#include "Components.h"
#include "Overlays.h"

#include "Serialization.h"

using framework::Entity;
using framework::Component;

namespace Shaders {
#include "shaders/simple_px.h"
#include "shaders/simple_vx.h"
#include "shaders/debug_px.h"
}

using hlslpp::float4x4;
using hlslpp::float4;

__declspec(align(16))
struct MVPConstantBuffer
{
	float4x4 world;
	float4x4 world_view;
	float4x4 proj;
	float4x4 wvp;

	float4x4 view;
	float4x4 inv_view;

	float4 view_direction;
	float4 light_direction;
	float4 light_color;
};

struct DebugVisualizeMode
{
	enum Enum {
		Default = 0,
		Albedo = 1,
		Roughness = 2,
		Metalness = 3,
		TangentNormals = 4,
		AO = 5,
		WorldNormals = 6
	};
};
__declspec(align(16))
struct DebugCB
{
	unsigned int m_VisualizeMode;
	uint8_t pad[12];
};

int g_DebugMode = 0;

void SceneViewer::configure_engine(EngineSettings &engineSettings) {
	engineSettings.d2d_use = false;

	engineSettings.d3d_use = true;
	engineSettings.d3d_msaa_mode = MSAAMode::MSAA_4x;
}

void SceneViewer::initialize(GameSettings& gameSettings)
{
	gameSettings.m_FullscreenMode = GameSettings::FullScreenMode::Windowed;
	gameSettings.m_WindowWidth = 1800;
	gameSettings.m_WindowHeight = 900;
}


void SceneViewer::start()
{
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	TextureResource::black();
	TextureResource::white();
	TextureResource::default_normal();

	::SetCapture(GameEngine::instance()->get_window());

	using namespace framework;
	_world = std::make_shared<World>();
	_world->init();


	framework::EntityDebugOverlay *overlay = new framework::EntityDebugOverlay(_world.get());
	GameEngine::instance()->get_overlay_manager()->register_overlay(overlay);

	// Create the world camera
	{
		World::EntityId camera = _world->create_entity();
		camera->set_name("MainCamera");
		auto comp = camera->create_component<CameraComponent>();
		camera->set_local_position(float3(0.0f, 0.0f, -2.0f));
		_world->attach_to_root(camera);
	}

	{
		namespace fs = std::filesystem;
		auto add_model = [&](float3 pos, float3 scale, fs::path const& model_path) {
			World::EntityId model = _world->create_entity();

			model->set_name(model_path.string());
			model->set_local_position(pos);
			model->set_local_scale(scale);
			auto comp = model->create_component<SimpleMeshComponent>();
			comp->set_model_path((fs::path{ "Resources/Models/" } / model_path).string());

			_world->attach_to_root(model);


			return model;
		};

		add_model(float3(0.0), float3(1.0), "plane/planes.gltf");

		for (int i = 0; i < 1; ++i) {
			for (int j = 0; j < 1; ++j) {
				World::EntityId ent = add_model(float3(2.0f * (float)i, 3.0f, 2.0f * (float)j), float3(1.0), "m-96_mattock/scene.gltf");
				auto c = ent->create_component<SimpleMovement3D>();
				c->set_speed(10.0f);
			}
		}

		World::EntityId ent = _world->create_entity();
		ent->set_name("Sun");
		ent->set_rotation(hlslpp::euler(float3(-0.33f, -0.33f, 0.0f)));
		ent->set_local_position(float3(0.0, 10.0, 0.0));
		auto comp = ent->create_component<LightComponent>();
		auto mesh_comp = ent->create_component<SimpleMeshComponent>();
		mesh_comp->set_model_path((fs::path{ "Resources/Models/axes/axes.gltf" }).string());
		_world->attach_to_root(ent);

		add_model(float3(50.0, 0.0, 0.0f), float3(1.0), "Tower/scene.gltf");
	}


	// Initialize our rendering buffer
	{
		D3D11_BUFFER_DESC buff{};
		buff.ByteWidth = sizeof(MVPConstantBuffer);
		buff.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buff.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buff.Usage = D3D11_USAGE_DYNAMIC;
		buff.StructureByteStride = 0;
		buff.MiscFlags = 0;

		MVPConstantBuffer mvp_data{};
		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = &mvp_data;
		SUCCEEDED(device->CreateBuffer(&buff, &data, _cb_MVP.GetAddressOf()));

		DebugCB debugData{};
		data.pSysMem = &debugData;
		buff.ByteWidth = sizeof(DebugCB);
		SUCCEEDED(device->CreateBuffer(&buff, &data, _cb_Debug.GetAddressOf()));

	}

	CD3D11_DEPTH_STENCIL_DESC ds_desc{ CD3D11_DEFAULT() };
	ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	SUCCEEDED(device->CreateDepthStencilState(&ds_desc, _depth_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_depth_state.Get(), L"Default DepthStencilState");

	CD3D11_BLEND_DESC bs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateBlendState(&bs_desc, _blend_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_blend_state.Get(), L"Default BlendState");

	CD3D11_RASTERIZER_DESC rs_desc{ CD3D11_DEFAULT() };
	SUCCEEDED(device->CreateRasterizerState(&rs_desc, _raster_state.GetAddressOf()));
	DirectX::SetDebugObjectName(_raster_state.Get(), L"Default RasterizerState");




	CD3D11_SAMPLER_DESC sampler{ CD3D11_DEFAULT() };
	sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SUCCEEDED(device->CreateSamplerState(&sampler, m_Samplers[uint32_t(Samplers::AllLinear)].GetAddressOf()));
}

void SceneViewer::end()
{

}

void SceneViewer::paint(graphics::D2DRenderContext& ctx)
{
}

void SceneViewer::tick(double deltaTime)
{
	_world->update((float)deltaTime);

	_timer += (float)deltaTime;
}

void SceneViewer::debug_ui()
{
	static bool s_open = true;
	ImGui::Begin("Game", &s_open);
	const char* items[] = {
		"Default",
		"Base Color",
		"Roughness",
		"Metalness",
		"Normals",
		"AO",
		"Normals (World)",
		"Vertex Colours"
	};

	ImGui::Combo("Debug Mode", &g_DebugMode, items, std::size(items));

	ImGui::PushID("#SceneName");
	static char buff[512] = "test.scene";
	ImGui::InputText("", buff, 512);
	ImGui::PopID();
	ImGui::SameLine();

	if (ImGui::Button("Save")) {
		std::string p = "Scenes/";
		p += buff;
		save_world(p.c_str());	
	}

	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		std::string p = "Scenes/";
		p += buff;

		_world->clear();
		load_world(p.c_str());
	}
	ImGui::End();

}

void SceneViewer::render_3d()
{
	auto device = GameEngine::instance()->GetD3DDevice();
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	// Use RH system so that we can directly export from blender
	//_timer = 0.0f;
	using namespace hlslpp;
	float4 view_direction = float4(0.0f, 0.0f, -2.0f, 0.0f);
	float4 light_direction = float4(0.0, -1.0, -1.0f,0.0f);
	float3 light_color = float3(1.0, 1.0, 1.0);
	LightComponent* comp = (LightComponent*)_world->find_first_component<LightComponent>();
	if (comp)
	{
		float4x4 worldTransform = comp->get_entity()->get_world_transform();
		float3 fwd = float3(0.0f, 0.0f, 1.0f);
		light_direction = hlslpp::mul(worldTransform, float4(fwd, 0.0));
		light_color = comp->get_color();
	}

	float4x4 View = float4x4::identity();

	ImVec2 size = GameEngine::instance()->get_viewport_size();
	const float aspect = (float)size.x / (float)size.y;
	const float near_plane = 0.01f;
	const float far_plane = 100.0f;

	hlslpp::projection proj(frustum::field_of_view_x(XMConvertToRadians(45.0f),aspect, far_plane, near_plane), zclip::zero);
	float4x4 Projection = float4x4::perspective(proj);


	ctx->OMSetDepthStencilState(_depth_state.Get(), 0);
	ctx->OMSetBlendState(_blend_state.Get(), NULL, 0xffffffff);
	ctx->RSSetState(_raster_state.Get());

	ID3D11SamplerState const* samplers[1] = {
		m_Samplers[uint32_t(Samplers::AllLinear)].Get()
	};
	ctx->PSSetSamplers(0, 1, (ID3D11SamplerState**)samplers);

	// Find our camera and use the matrices
	CameraComponent* camera = _world->find_first_component<CameraComponent>();
	View = hlslpp::float4x4::look_at(view_direction.xyz, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
	if (camera)
	{
		float4x4 cameraTransform = camera->get_entity()->get_world_transform();
		View = inverse(cameraTransform);

		hlslpp::projection proj(frustum::field_of_view_x(camera->get_fov(), aspect, camera->get_far_plane(), camera->get_near_plane()), zclip::zero);
		Projection = float4x4::perspective(proj);

		// +Y is forward
		float4 world_fwd{ 0.0f, 0.0f, 1.0f, 0.0f };
		view_direction = mul(cameraTransform, world_fwd);
	}
	float4x4 invView = inverse(View);


	// Update debug mode
	if (g_DebugMode != DebugVisualizeMode::Default)
	{
		D3D11_MAPPED_SUBRESOURCE resource{};
		ctx->Map(_cb_Debug.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		DebugCB* buffer = (DebugCB*)resource.pData;
		buffer->m_VisualizeMode = g_DebugMode;
		ctx->Unmap(_cb_Debug.Get(), 0);

		ctx->PSSetConstantBuffers(1, 1, _cb_Debug.GetAddressOf());
	}


	for (auto& ent : _world->get_entities())
	{
		if (ent == nullptr)
			continue;

		// If the entity has a mesh component and is done loading
		if (SimpleMeshComponent* mesh_comp = ent->get_component<SimpleMeshComponent>(); mesh_comp && mesh_comp->is_loaded() && mesh_comp->is_active())
		{
			float4x4 world = ent->get_world_transform();
			float4x4 MVP = mul(world, mul(View, Projection));

			D3D11_MAPPED_SUBRESOURCE resource{};
			ctx->Map(_cb_MVP.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			MVPConstantBuffer* buffer = (MVPConstantBuffer*)resource.pData;
			buffer->world = world;
			buffer->wvp = MVP;
			buffer->world_view = mul(world, View);
			buffer->proj = Projection;
			buffer->inv_view = invView;
			buffer->view = View;
			buffer->view_direction = view_direction;
			buffer->light_direction = light_direction;
			buffer->light_color = float4(light_color, 1.0);
			ctx->Unmap(_cb_MVP.Get(), 0);

			// TODO: Implement materials supplying buffers?
			ctx->VSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());
			ctx->PSSetConstantBuffers(0, 1, _cb_MVP.GetAddressOf());

			mesh_comp->render();
		}
	}
}

static const char* s_world_path = "Scenes/test_world.scene";
bool SceneViewer::load_world(const char* path) {
	auto io = GameEngine::instance()->io();
	if (io->exists(path)) {
		auto io = GameEngine::instance()->io();
		std::shared_ptr<IO::IFile> file = io->open(path, IO::Mode::Read, true);
		assert(file);

		u32 number_of_entities = serialization::read<u32>(file);

		// Phase 1: Read in the entire world

		struct ReadData {
			Identifier64 parent_id;
			framework::EntityHandle ent;
		};
		std::vector<ReadData> handles;
		handles.reserve(number_of_entities);

		for (u32 i = 1; i < number_of_entities; ++i) {
					
			Identifier64 id = serialization::read<Identifier64>(file);
			Identifier64 parent_id = serialization::read<Identifier64>(file);

			framework::EntityHandle ent = _world->create_entity(id);
			rttr::instance obj = ent.get();

			serialization::serialize_instance<IO::Mode::Read>(file, obj);

			// Write components manually
			u32 n_components = serialization::read<u32>(file);
			for (int i =0; i < n_components; ++i) {
				u64 pos = file->tell();
				u64 hash = serialization::read<u64>(file);
				file->seek(pos, IO::SeekMode::FromBeginning);

				auto t = helpers::get_type_by_id(hash);
				rttr::variant inst = t.create();
				serialization::serialize_instance<IO::Mode::Read>(file, inst);

				Component* comp = inst.get_value<Component*>();
				_world->attach_to(ent, inst.get_value<Component*>());
			}

			// Store for resolving later
			handles.push_back({ parent_id, ent });
		}

		// Phase 2: Fixup the parent IDs
		std::for_each(handles.begin(), handles.end(), [&](ReadData const& h) {

			framework::EntityHandle parent = _world->find_by_id(h.parent_id);
			if (!parent.is_valid()) {
				parent = _world->get_root();
			}
			_world->attach_to(parent, h.ent);
		});
	}
	return true;
}


void SceneViewer::save_world(const char* path) {
	using namespace serialization;

	auto io = GameEngine::instance()->io();
	std::shared_ptr<IO::IFile> file = io->open(path, IO::Mode::Write, true);
	assert(file);

	std::vector<Entity*> all_entities = _world->get_entities();

	u32 number_of_entities = _world->get_number_of_entities();
	serialization::write<u32>(file, number_of_entities);

	// Skip the root entity
	for (u32 i = 1; i < all_entities.size(); ++i) {

		if (!all_entities[i]) {
			continue;
		}

		rttr::instance obj = all_entities[i];

		Entity* ent = all_entities[i];

		serialization::write(file, ent->get_id());

		// Serialize parent id
		Identifier64 parent_id;
		if(ent->_parent && ent->_parent != _world->get_root()) {
			parent_id = ent->_parent->get_id();
		}
		serialization::write<Identifier64>(file, parent_id);

		serialization::serialize_instance<IO::Mode::Write>(file, obj);



		// Write components manually
		u32 n_components = all_entities[i]->get_components().size();
		write<u32>(file, n_components);
		for (Component* comp : all_entities[i]->get_components()) {
			rttr::instance inst = comp;
			serialization::serialize_instance<IO::Mode::Write>(file, inst);
		}
	}
}

