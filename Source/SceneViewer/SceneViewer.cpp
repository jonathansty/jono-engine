#include "sceneviewer.pch.h"
#include <hlsl++.h>
#include "SceneViewer.h"

#include "Engine/Core/ModelResource.h"
#include "Engine/Core/TextureResource.h"
#include "Engine/Core/MaterialResource.h"

#include "Engine/Graphics/ShaderCache.h"

#include "Framework/framework.h"
#include "Components.h"
#include "Overlays.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Core/Material.h"
#include "InputManager.h"
#include "Graphics/Graphics.h"

#include "Serialization.h"

using framework::Entity;
using framework::Component;

using hlslpp::float4x4;
using hlslpp::float4;

struct DebugVisualizeMode
{
	enum Enum
	{
		Default = 0,
		Albedo = 1,
		Roughness = 2,
		Metalness = 3,
		TangentNormals = 4,
		AO = 5,
		WorldNormals = 6
	};
};


extern int g_DebugMode;

void SceneViewer::configure_engine(EngineSettings& engineSettings)
{
	engineSettings.d2d_use = false;

	engineSettings.d3d_use = true;
	engineSettings.d3d_msaa_mode = MSAAMode::Off;

	engineSettings.max_frame_time = 1.0 / 72.0;
}

void SceneViewer::initialize(GameSettings& gameSettings)
{
	gameSettings.m_FullscreenMode = GameSettings::FullScreenMode::Windowed;
	gameSettings.m_WindowWidth = 800;
	gameSettings.m_WindowHeight = 800;
}

std::string show_file_dialog(HWND owner)
{
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = "Model Files (*.glb;*.gltf)\0*.glb;*.gltf\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = nullptr;

	if (GetOpenFileNameA(&ofn))
	{
		return ofn.lpstrFile;
	}

	return {};
}

void SceneViewer::start()
{

	auto ge = GameEngine::instance();

	ge->set_build_menu_callback([ge, this](GameEngine::BuildMenuOrder order)
	{
		if (order == GameEngine::BuildMenuOrder::First)
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					open_file();
				}

				if (ImGui::MenuItem("Rebuild Shaders"))
				{
					LOG_INFO(Graphics, "Rebuilding all shaders.");

					this->rebuild_shaders();
				}
				ImGui::EndMenu();
			}
		}
	});

	auto device = Graphics::get_device();
	auto ctx = Graphics::get_ctx();

	// Capture the mouse in the window
	::SetCapture(GameEngine::instance()->get_window());

	// Setup the game world
	using namespace framework;
	_world = GameEngine::instance()->get_world();
	
	auto render_world = GameEngine::instance()->get_render_world();

	ImVec2 size = GameEngine::instance()->get_viewport_size();
	const float aspect = (float)size.x / (float)size.y;
	const float near_plane = 0.1f;
	const float far_plane = 500.0f;

	// Create Cam 1
	auto rw_cam = render_world->create_camera();
	RenderWorldCamera::CameraSettings settings{};
	settings.aspect = aspect;
	settings.far_clip = far_plane;
	settings.near_clip = near_plane;
	settings.fov = hlslpp::radians(float1(60.0f));
	settings.projection_type = RenderWorldCamera::Projection::Perspective;
	rw_cam->set_settings(settings);
	rw_cam->set_position({ 0.0f, 1.0f, -1.0f });

	auto l = render_world->create_light(RenderWorldLight::LightType::Directional);
	settings.aspect = 1.0f;
	settings.near_clip = 0.0f;
	settings.far_clip = 25.0f;
	settings.width = 10.0f;
	settings.height = 20.0f;
	settings.projection_type = RenderWorldCamera::Projection::Ortographic;
	l->set_settings(settings);

	constexpr f32 c_scale = 2.0f;
	l->set_colour({ 1.0f * c_scale, 1.0f * c_scale, 1.0f * c_scale });
	l->set_casts_shadow(true);
	l->set_position({ 10.0, 10.0f, 0.0f });
	l->look_at(float3{0.0f, 0.0f, 0.0f });
	_light = l;
	//framework::EntityDebugOverlay* overlay = new framework::EntityDebugOverlay(_world.get());
	//GameEngine::instance()->get_overlay_manager()->register_overlay(overlay);


	for(size_t y = 0; y < 10; ++y)
	{
		for(size_t x = 0; x < 10; ++x)
		{
			float3 pos = float3(2.0f + x * 2.0f, 0.0f, y * 2.0f);
			RenderWorldInstanceRef inst = GameEngine::instance()->get_render_world()->create_instance(float4x4::translation(pos), "res:/sphere.glb");
			Model const* model = inst->_model->get();

			f32 x_dt = (float)x / 10.0f;
			f32 y_dt = (float)y / 10.0f;

			auto mat_inst = std::make_unique<MaterialInstance>();
			mat_inst->set_param_float("Roughness", hlslpp::lerp(float1(0.01f), 1.0f, x_dt));
			mat_inst->set_param_float("Metalness", hlslpp::lerp(float1(0.01f), 1.0f, y_dt));
			inst->set_dynamic_material(0,std::move(mat_inst));
		}
	}


}

void SceneViewer::end()
{
}

#if FEATURE_D2D
void SceneViewer::paint(Graphics::D2DRenderContext& ctx)
{
}
#endif

int* s_data = nullptr;

void SceneViewer::tick(double deltaTime)
{

	f32 f32_dt = (f32)deltaTime;
	auto& input_manager = GameEngine::instance()->get_input();
	if (input_manager->is_key_pressed(KeyCode::Right)) {
		_timer -= 0.5f;
	}
	if (input_manager->is_key_pressed(KeyCode::Left)) {
		_timer += 0.5f;
	}

	if (input_manager->is_key_pressed(KeyCode::Up)) {
		_light_tick += 0.5f;
	}
	if (input_manager->is_key_pressed(KeyCode::Down)) {
		_light_tick -= 0.5f;
	}
	if(input_manager->is_key_pressed(KeyCode::R) && input_manager->is_key_down(KeyCode::Control))
	{
		rebuild_shaders();
	}

	if (input_manager->is_key_pressed(KeyCode::O) && input_manager->is_key_down(KeyCode::Control))
	{
		open_file();
	}


	if(input_manager->is_key_pressed(KeyCode::P))
	{
		if (s_data)
		{
			delete[] s_data;
			s_data = nullptr;
		}
		else
		{
			s_data = new int[1000000];
		}
	}



	auto imgui = ImGui::GetIO();

	RenderWorldRef world = GameEngine::instance()->get_render_world();
	auto camera = world->get_view_camera();
	bool has_viewport_focus = GameEngine::instance()->is_viewport_focused();
	if (has_viewport_focus)
	{
		f32 scroll_delta = input_manager->get_scroll_delta();
		f32 zoom_factor = _zoom * 0.05f;
		if (scroll_delta != 0)
		{
			_zoom -= scroll_delta * (zoom_factor)*100.0f * f32_dt;
		}

		// Rotate camera
		float2 mouse_delta = float2(input_manager->get_mouse_delta());
		if (input_manager->is_mouse_button_down(0))
		{
			_timer -= mouse_delta.x * f32_dt * 0.5f;
			_up_timer += mouse_delta.y * f32_dt * 0.5f;
		}
		_up_timer = std::clamp<f32>(_up_timer, -static_cast<f32>(M_PI) + std::numeric_limits<f32>::epsilon(), -std::numeric_limits<f32>::epsilon());

		// rotate light
		if (input_manager->is_mouse_button_down(2))
		{
			_light_tick -= mouse_delta.x * f32_dt * 0.5f;
		}

		// pan
		float3 pos = camera->get_position();

		float3 localPan = float3(0.0f, 0.0f, 0.0f);
		if (input_manager->is_mouse_button_down(1))
		{
			float4x4 view = camera->get_view();

			float4 right = hlslpp::mul(view, float4(1.0, 0.0, 0.0, 0.0)) * -(f32)mouse_delta.x * deltaTime;
			float4 up = hlslpp::mul(view, float4(0.0, 1.0, 0.0, 0.0)) * (f32)mouse_delta.y * deltaTime;

			localPan = (right + up).xyz * zoom_factor;
		}
		_center += localPan;

		if (input_manager->is_key_pressed(KeyCode::Z))
		{
			LOG_INFO(Input, "Resetting view.");
			_timer = 0.0f;
			_up_timer = 0.0f;
			_center = float3(0.0, 0.0, 0.0);
		}
	}


	float radius = 10.0f;
	float3 newUnitPos = float3{ cos(_timer) * sin(_up_timer), cos(_up_timer), sin(_timer) * sin(_up_timer) };
	camera->set_position(_center + _zoom*newUnitPos);
	camera->look_at(_center);

	if(_light)
	{
		_light->set_position(float3{ radius * sin(_light_tick), 5.0f, radius * cos(_light_tick) });
		_light->look_at(float3{ 0.0f, 0.0f, 0.0f });
	}

	if(_world)
	{
		_world->update((float)deltaTime);
	}

}

void SceneViewer::debug_ui()
{
	static bool s_open = true;
	ImGui::Begin("Game", &s_open);

	ImGui::Text("Light: %.2f", _light_tick);
	ImGui::Text("Camera: %.2f", _timer);

	const char* items[] = {
		"Default",
		"(Input) Base Color",
		"(Input) Roughness",
		"(Input) Metalness",
		"(Input) Normals",
		"(Input) AO",
		"(Input) Normals (World)",
		"(Input) Vertex Colours",
		"(Input) UV",
		"Lighting",
	};

	ImGui::Combo("Debug Mode", &g_DebugMode, items, static_cast<int>(std::size(items)));

	//std::shared_ptr<ModelResource> res = _model->_model;
	//Model const* model = res->get();
	//for(u32 i = 0; i < model->get_material_count(); ++i)
	//{
	//	model->get_material(i);
	//}

	//ImGui::PushID("#SceneName");
	//static char buff[512] = "test.scene";
	//ImGui::InputText("", buff, 512);
	//ImGui::PopID();
	//ImGui::SameLine();

	//if (ImGui::Button("Save")) {
	//	std::string p = "Scenes/";
	//	p += buff;
	//	save_world(p.c_str());	
	//}

	//ImGui::SameLine();
	//if (ImGui::Button("Load")) {
	//	std::string p = "Scenes/";
	//	p += buff;

	//	_world->clear();
	//	load_world(p.c_str());
	//}


	ImGui::End();

}

static const char* s_world_path = "Scenes/test_world.scene";

void SceneViewer::open_file()
{
	LOG_VERBOSE(UI, "Opening file...");
	std::string file = show_file_dialog(GameEngine::instance()->get_window());
	if (!file.empty())
	{
		this->swap_model(file.c_str());
	}
}

void SceneViewer::rebuild_shaders()
{
	using namespace Graphics;
	auto world = GameEngine::instance()->get_render_world();

	ShaderCache::instance()->reload_all();

	for(MaterialResource* res : MaterialResource::s_resources)
	{
		res->_status = ResourceStatus::Loading;
		res->load(nullptr);
		res->_status = ResourceStatus::Loaded;
	}
}

bool SceneViewer::load_world(const char* path) {
	auto io = IO::get();
	if (io->exists(path)) {
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
			rttr::instance obj = rttr::instance(ent.get());

			serialization::serialize_instance<IO::Mode::Read>(file, obj);

			// Write components manually
			u32 n_components = serialization::read<u32>(file);
			for (u32 j =0; j < n_components; ++j) {
				u64 pos = file->tell();
				u64 hash = serialization::read<u64>(file);
				file->seek(pos, IO::SeekMode::FromBeginning);

				auto t = Helpers::get_type_by_id(hash);
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


void SceneViewer::save_world(const char* path)
{
	using namespace serialization;

	auto io = GameEngine::instance()->io();
	std::shared_ptr<IO::IFile> file = io->open(path, IO::Mode::Write, true);
	assert(file);

	std::vector<Entity*> all_entities = _world->get_entities();

	u32 number_of_entities = static_cast<u32>(_world->get_number_of_entities());
	serialization::write<u32>(file, number_of_entities);

	// Skip the root entity
	for (u32 i = 1; i < all_entities.size(); ++i)
	{
		if (!all_entities[i])
		{
			continue;
		}

		rttr::instance obj = all_entities[i];

		Entity* ent = all_entities[i];

		serialization::write(file, ent->get_id());

		// Serialize parent id
		Identifier64 parent_id;
		if (ent->get_parent() && ent->get_parent() != _world->get_root())
		{
			parent_id = ent->get_parent()->get_id();
		}
		serialization::write<Identifier64>(file, parent_id);

		serialization::serialize_instance<IO::Mode::Write>(file, obj);

		// Write components manually
		u32 n_components = static_cast<u32>(all_entities[i]->get_components().size());
		write<u32>(file, n_components);
		for (Component* comp : all_entities[i]->get_components())
		{
			rttr::instance inst = comp;
			serialization::serialize_instance<IO::Mode::Write>(file, inst);
		}
	}
}

void SceneViewer::swap_model(const char* path)
{
	GameEngine::instance()->get_render_world()->remove_instance(_model);

	_model = GameEngine::instance()->get_render_world()->create_instance(float4x4::identity(), path);
}
