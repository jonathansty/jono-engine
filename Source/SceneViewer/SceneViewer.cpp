#include "sceneviewer.pch.h"

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
#include "Parsing/Yaml.h"

#include "OrbitCamera.h"
#include "FreeCam.h"

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


//extern int g_DebugMode;

REGISTER_TYPE("/Types/Games/SceneViewer", SceneViewer);
SERIALIZE_FN(SceneViewer) {}

 SceneViewer::SceneViewer(std::string const& path)
	:m_ScenePath(path)
	,m_LightT(0.0f)
	,m_OrbitCamera(nullptr)
	,m_FreeCamera(nullptr)
	,m_EntityOverlay(nullptr)
{

}

SceneViewer::SceneViewer()
	: m_ScenePath("")
	,m_LightT(0.0f)
	,m_OrbitCamera(nullptr)
	,m_FreeCamera(nullptr)
	,m_EntityOverlay(nullptr)
{
}

SceneViewer::~SceneViewer()
{
}

void SceneViewer::ConfigureEngine(EngineCfg& engineSettings)
{
	Super::ConfigureEngine(engineSettings);
}

void SceneViewer::ConfigureGame(GameCfg& gameSettings)
{
	Super::ConfigureGame(gameSettings);

	gameSettings.m_FullscreenMode = GameCfg::FullScreenMode::Windowed;
	gameSettings.m_WindowWidth = 1920;
	gameSettings.m_WindowHeight = 1080;
}

std::string ShowFileDialog(SDL_Window* owner)
{
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = nullptr;
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

void SceneViewer::OnStartup()
{
	Super::OnStartup();

	auto ge = GameEngine::instance();

	ge->set_build_menu_callback([ge, this](GameEngine::BuildMenuOrder order)
	{
		if (order == GameEngine::BuildMenuOrder::First)
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					OpenFile();
				}

				if (ImGui::MenuItem("Rebuild Shaders"))
				{
					LOG_INFO(Graphics, "Rebuilding all shaders.");

					this->RebuildAllShaders();
				}
				ImGui::EndMenu();
			}
		} 
	});

	auto device = Graphics::get_device();
	auto ctx = Graphics::get_ctx();

	// Capture the mouse in the window
	GetGlobalContext()->m_InputManager->set_cursor_visible(false);

	// Setup the game world
	using namespace framework;
	m_World = GameEngine::instance()->get_world();

	const char* path = "res:/Scenes/default.yml";
	std::string resolvedPath = IO::get()->ResolvePath(path);
	yaml::Document doc = yaml::Document(resolvedPath.c_str());

	auto render_world = GameEngine::instance()->get_render_world();

	Yaml::Node& node = doc.GetRoot();
	auto data = node["Models"][0]["name"].As<std::string>();
	for (auto it = node["Models"].Begin(); it != node["Models"].End(); it++)
	{
		//LOG_INFO(Unknown, "{}: {}", (*it).first, (*it).second.As<string>()); 
		std::string name = (*it).second["name"].As<std::string>();
		std::string type = (*it).second["type"].As<std::string>();
		std::string position = (*it).second["position"].As<std::string>();
		std::string scaleT = (*it).second["scale"].As<std::string>();
		std::string rotation = (*it).second["rotation"].As<std::string>();
		std::string mesh = (*it).second["mesh"].As<std::string>();

		float pos[3];
		float scale[3];
		float rot[3];

		std::vector<std::string_view> result;
		Helpers::split_string(position, ",", result);

		pos[0] = (float)atof(result[0].data());
		pos[1] = (float)atof(result[1].data());
		pos[2] = (float)atof(result[2].data());

		Helpers::split_string(scaleT, ",", result);
		scale[0] = (float)atof(result[0].data());
		scale[1] = (float)atof(result[1].data());
		scale[2] = (float)atof(result[2].data());

		Helpers::split_string(rotation, ",", result);
		rot[0] = (float)atof(result[0].data());
		rot[1] = (float)atof(result[1].data());
		rot[2] = (float)atof(result[2].data());

		float4x4 transform = float4x4::translation(float3(pos[0], pos[1], pos[2]));
		render_world->create_instance(transform, mesh);
	}



	ImVec2 size = GameEngine::instance()->GetViewportSize();
	const float aspect = (float)size.x / (float)size.y;
	const float near_plane = 0.5f;
	const float far_plane = 250.0f;

	// Create Cam 1
	auto rw_cam = render_world->create_camera();
	RenderWorldCamera::CameraSettings settings{};
	settings.aspect = aspect;
	settings.far_clip = far_plane;
	settings.near_clip = near_plane;
	settings.fov = hlslpp::radians(float1(45.0f)); // 45 deg fov
	settings.projection_type = RenderWorldCamera::Projection::Perspective;
	rw_cam->set_settings(settings);
	rw_cam->set_view(float4x4::translation(0.0f, 0.0f, -2.0f));

	// Copy our cam into debug cam
	auto new_cam = render_world->create_camera();
	settings.far_clip = 500.0f;
	new_cam->set_settings(settings);

	m_CameraType = 0;
	m_OrbitCamera  = JONO_NEW(OrbitCamera, render_world);
	m_FreeCamera = JONO_NEW(FreeCam, render_world);

	auto l = render_world->create_light(RenderWorldLight::LightType::Directional);
	settings.aspect = 1.0f;
	settings.near_clip = 0.0f;
	settings.far_clip = 25.0f;
	settings.width = 10.0f;
	settings.height = 20.0f;
	settings.projection_type = RenderWorldCamera::Projection::Ortographic;
	l->set_settings(settings);

	constexpr f32 c_scale = 0.5f;
	l->set_colour({ 1.0f * c_scale, 1.0f * c_scale, 1.0f * c_scale });
	l->set_casts_shadow(true);
	l->set_view(float4x4::look_at({ 10.0, 10.0f, 0.0f }, float3(0.0f,0.0f,0.0f), float3(0.0f,1.0f,0.0f)));
	m_SunLight = l;

	// Disabled for now as this app is not using the entity system
	//framework::EntityDebugOverlay* overlay = new framework::EntityDebugOverlay(_world.get());
	//GameEngine::instance()->get_overlay_manager()->register_overlay(overlay);

	#if 0 
	constexpr u32 c_grid_size = 25;
	constexpr f32 c_grid_spacing = 3.0f;
	for(size_t y = 0; y < c_grid_size; ++y)
	{
		for(size_t x = 0; x < c_grid_size; ++x)
		{
			float3 pos = float3(10.0f + x * c_grid_spacing, 1.0f, y * c_grid_spacing);
			RenderWorldInstanceRef inst = render_world->create_instance(float4x4::translation(pos), "res:/Models/sphere.glb");
			Model const* model = inst->_model->get();

			f32 x_dt = (float)x / c_grid_size;
			f32 y_dt = (float)y / c_grid_size;

			auto mat_inst = std::make_unique<MaterialInstance>();
			mat_inst->set_param_float("Roughness", hlslpp::lerp(float1(0.01f), 1.0f, 1.0f - x_dt));
			mat_inst->set_param_float("Metalness", hlslpp::lerp(float1(0.01f), 1.0f, y_dt));
			inst->set_dynamic_material(0,std::move(mat_inst));

	
		}
	}

	f32 world_size = c_grid_size * c_grid_spacing;
	u32 nLightsY = 8;
	u32 nLightsX = 8; 
	for(u32 y = 0; y < nLightsX; ++y)
	{
		for(u32 x = 0; x < nLightsY; ++x)
		{
			f32 spacing_x = world_size / nLightsY;
			f32 spacing_y = world_size / nLightsX;
			float3 pos = float3(10.0f + x * spacing_x, 1.0f, y * spacing_y);
			f32 x_dt = (float)x / nLightsY;
			f32 y_dt = (float)y / nLightsX;

			RenderWorldLightRef light = render_world->create_light(RenderWorldLight::LightType::Spot);
			RenderWorldCamera::CameraSettings light_proj_settings{};
			light_proj_settings.aspect = 1;
			light_proj_settings.far_clip = 50.0f; // Range
			light_proj_settings.near_clip = 0.01f;
			light_proj_settings.fov = hlslpp::radians(float1(60.0f)); // Cone angle?
			light_proj_settings.projection_type = RenderWorldCamera::Projection::Perspective;
			light->set_settings(settings);
			light->set_range(8.0f);
			light->set_cone_angle(hlslpp::radians(float1(33.5f)));
			light->set_outer_cone_angle(hlslpp::radians(float1(60.5f)));
			light->set_view(float4x4::look_at(float3(pos) + float3(0.0f, 3.0f, 0.125f), pos, Math::c_up));

			constexpr f32 c_intensity = 1.0f;
			float3 colour = float3(x_dt, y_dt, 1.0f) * c_intensity;
			light->set_colour(colour);
		}
	}
	#endif


	#if 0
	RenderWorldInstanceRef inst = render_world->create_instance(float4x4::translation(0.0f,0.0f,2.0f), "res:/Models/sphere.glb");
	_model = inst;

	Model const* model = inst->_model->get();
	auto mat_inst = std::make_unique<MaterialInstance>();
	mat_inst->set_param_float("Roughness", 0.25f);
	mat_inst->set_param_float("Metalness", 0.0f);
	mat_inst->set_param_float3("Albedo", float3{ 0.8f, 0.8f, 0.8f });
	inst->set_dynamic_material(0, std::move(mat_inst));


	//world->create_instance(float4x4::scale(2.0f) * float4x4::translation(float3(-5.0f,0.5f,0.0f)), "Resources/Models/gltf-models/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
	render_world->create_instance(float4x4::translation(float3(0.0f, 0.0f, 0.0f)), "Resources/Models/plane_big.glb");
	#endif

}

void SceneViewer::OnShutdown()
{
	Super::OnShutdown();

	delete m_OrbitCamera;
	m_OrbitCamera = nullptr;

	delete m_FreeCamera;
	m_FreeCamera = nullptr;
}

#if FEATURE_D2D
void SceneViewer::OnPaint2D(Graphics::D2DRenderContext& ctx)
{
	Super::OnPaint2D(ctx);
}
#endif

int* s_data = nullptr;

void SceneViewer::OnUpdate(double deltaTime)
{
	Super::OnUpdate(deltaTime);

	f32 f32_dt = (f32)deltaTime;
	auto& input_manager = GameEngine::instance()->get_input();
	float2 mouse_delta = float2(input_manager->get_mouse_delta());

	if (input_manager->is_key_pressed(KeyCode::Up))
	{
		m_LightT += 0.5f;
	}
	if (input_manager->is_key_pressed(KeyCode::Down))
	{
		m_LightT -= 0.5f;
	}

	if (input_manager->is_key_pressed(KeyCode::R) && input_manager->is_key_down(KeyCode::LControl))
	{
		RebuildAllShaders();
	}

	if (input_manager->is_key_pressed(KeyCode::O) && input_manager->is_key_down(KeyCode::LControl))
	{
		OpenFile();
	}

	if(m_CameraType == 0)
	{
		m_OrbitCamera->tick(deltaTime);
	}
	else
	{
		m_FreeCamera->tick(deltaTime);
	}

	bool has_viewport_focus = GameEngine::instance()->is_viewport_focused();
	if (has_viewport_focus)
	{
		// rotate light
		if (input_manager->is_mouse_button_down(2))
		{
			m_LightT -= mouse_delta.x * f32_dt * 0.5f;
		}
	}

	RenderWorldRef world = GameEngine::instance()->get_render_world();
	auto camera = world->get_view_camera();

	if (m_SunLight)
	{
		constexpr float radius = 10.0f;

		float4x4 m = float4x4::look_at(float3{ radius * sin(m_LightT), 5.0f, radius * cos(m_LightT) }, {0.0f,0.0f,0.0f}, Math::c_up);
		m_SunLight->set_view(m);
	}

	if (m_World)
	{
		m_World->update((float)deltaTime);
	}

}

void SceneViewer::OnDebugUI()
{
	static bool s_open = true;

	ImGuiID propertyDockID = GetGlobalContext()->m_Engine->GetPropertyDockID();
	if(propertyDockID)
	{
		ImGui::SetNextWindowDockID(propertyDockID, ImGuiCond_Once);
	}

	ImGui::Begin("Viewer", &s_open);

	const char* cameras[] = {
		"Orbit",
		"Free"
	};
	ImGui::Combo("Camera", &m_CameraType, cameras, int(std::size(cameras)));

	ImGui::Text("Light: %.2f", m_LightT);

	const char* items[] = {
		"Default",
		"(Input) Base Color",
		"(Input) Roughness",
		"(Input) Metalness",
		"(Input) Normals",         // 4
		"(Input) AO",              // 5
		"(Input) Normals (World)", // 6
		"(Input) Vertex Colours",  // 7
		"(Input) UV",              // 8
		"Lighting",                // 9
		"ForwardPlusDebug"         // 10
	};

	extern int g_DebugMode;
	ImGui::Combo("Debug Mode", &g_DebugMode, items, static_cast<int>(std::size(items)));


	if(ImGui::CollapsingHeader("Model Info"))
	{
		if(m_CurrentModel)
		{
			MaterialInstance* inst = m_CurrentModel->get_material_instance(0);
			if (inst)
			{
				static float col[3] = { 1.0f, 1.0f, 1.0f };
				if (ImGui::ColorPicker3("Color", col))
				{
					inst->set_param_float3("Albedo", { col[0], col[1], col[2] });
				}

				static f32 s_roughness = 0.25f;
				if(ImGui::SliderFloat("Roughness", &s_roughness, 0.001f, 1.0f))
				{
					inst->set_param_float("Roughness", s_roughness);
				}

				static f32 s_metalness = 0.25f;
				if (ImGui::SliderFloat("Metalness", &s_metalness, 0.001f, 1.0f))
				{
					inst->set_param_float("Metalness", s_metalness);
				}


			}
		}
	}


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

void SceneViewer::OpenFile()
{
	LOG_VERBOSE(UI, "Opening file...");
	std::string file = ShowFileDialog(GameEngine::instance()->GetWindow());
	if (!file.empty())
	{
		this->SwapModel(file.c_str());
	}
}

void SceneViewer::RebuildAllShaders()
{
	using namespace Graphics;
	auto world = GameEngine::instance()->get_render_world();

	ShaderCache::instance()->reload_all();

	for(auto const& res : MaterialHandle::s_Resources)
	{
		res->SetStatus(ResourceStatus::Loading);
		res->load(nullptr);
		res->SetStatus(ResourceStatus::Loaded);
	}
}

bool SceneViewer::LoadWorld(const char* path) {
	//auto io = IO::get();
	//if (io->exists(path)) {
	//	std::shared_ptr<IO::IFile> file = io->open(path, IO::Mode::Read, true);
	//	assert(file);

	//	u32 number_of_entities = serialization::read<u32>(file);

	//	// Phase 1: Read in the entire world
	//	struct ReadData {
	//		Identifier64 parent_id;
	//		framework::EntityHandle ent;
	//	};
	//	std::vector<ReadData> handles;
	//	handles.reserve(number_of_entities);

	//	for (u32 i = 1; i < number_of_entities; ++i) {
	//				
	//		Identifier64 id = serialization::read<Identifier64>(file);
	//		Identifier64 parent_id = serialization::read<Identifier64>(file);

	//		framework::EntityHandle ent = _world->create_entity(id);
	//		rttr::instance obj = rttr::instance(ent.get());

	//		serialization::serialize_instance<IO::Mode::Read>(file, obj);

	//		// Write components manually
	//		u32 n_components = serialization::read<u32>(file);
	//		for (u32 j =0; j < n_components; ++j) {
	//			u64 pos = file->tell();
	//			u64 hash = serialization::read<u64>(file);
	//			file->seek(pos, IO::SeekMode::FromBeginning);

	//			auto t = Helpers::get_type_by_id(hash);
	//			rttr::variant inst = t.create();
	//			serialization::serialize_instance<IO::Mode::Read>(file, inst);

	//			Component* comp = inst.get_value<Component*>();
	//			_world->attach_to(ent, inst.get_value<Component*>());
	//		}

	//		// Store for resolving later
	//		handles.push_back({ parent_id, ent });
	//	}

	//	// Phase 2: Fixup the parent IDs
	//	std::for_each(handles.begin(), handles.end(), [&](ReadData const& h) {

	//		framework::EntityHandle parent = _world->find_by_id(h.parent_id);
	//		if (!parent.is_valid()) {
	//			parent = _world->get_root();
	//		}
	//		_world->attach_to(parent, h.ent);
	//	});
	//}
	return true;
}


void SceneViewer::SaveWorld(const char* path)
{
	//using namespace serialization;

	//auto io = GetGlobalContext()->m_PlatformIO;
	//std::shared_ptr<IO::IFile> file = io->open(path, IO::Mode::Write, true);
	//assert(file);

	//std::vector<Entity*> all_entities = _world->get_entities();

	//u32 number_of_entities = static_cast<u32>(_world->get_number_of_entities());
	//serialization::write<u32>(file, number_of_entities);

	//// Skip the root entity
	//for (u32 i = 1; i < all_entities.size(); ++i)
	//{
	//	if (!all_entities[i])
	//	{
	//		continue;
	//	}

	//	rttr::instance obj = all_entities[i];

	//	Entity* ent = all_entities[i];

	//	serialization::write(file, ent->get_id());

	//	// Serialize parent id
	//	Identifier64 parent_id;
	//	if (ent->get_parent() && ent->get_parent() != _world->get_root())
	//	{
	//		parent_id = ent->get_parent()->get_id();
	//	}
	//	serialization::write<Identifier64>(file, parent_id);

	//	serialization::serialize_instance<IO::Mode::Write>(file, obj);

	//	// Write components manually
	//	u32 n_components = static_cast<u32>(all_entities[i]->get_components().size());
	//	write<u32>(file, n_components);
	//	for (Component* comp : all_entities[i]->get_components())
	//	{
	//		rttr::instance inst = comp;
	//		serialization::serialize_instance<IO::Mode::Write>(file, inst);
	//	}
	//}
}

void SceneViewer::SwapModel(const char* path)
{
	GameEngine::instance()->get_render_world()->remove_instance(m_CurrentModel);

	m_CurrentModel = GameEngine::instance()->get_render_world()->create_instance(float4x4::identity(), path);
	m_CurrentModel->set_dynamic_material(0, std::make_unique<MaterialInstance>());
}
