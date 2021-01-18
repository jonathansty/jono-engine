#include "stdafx.h"
#include "HelloWorld.h"

#include "Engine/Framework/framework.h"
#include "core/identifier.h"

#include "Components.h"
#include "Overlays.h"

extern void ExecuteRttiTest_BasicTypes();


class ResourcePaths
{
public:
	static constexpr const char* bmp_coin_bronze = "Resources/Pickups/coinBronze.png";
	static constexpr const char* bmp_coin_silver = "Resources/Pickups/coinSilver.png";
	static constexpr const char* bmp_coin_gold = "Resources/Pickups/coinGold.png";
};

class GameOverlay : public DebugOverlay
{
public:
	GameOverlay() : DebugOverlay(false, "GameOverlay") {}
	virtual ~GameOverlay() {}


	virtual void render_overlay() override
	{
		ImGuiIO& io = ImGui::GetIO();

		ImVec2 window_pos = ImVec2(5.0f, io.DisplaySize.y - 200.0f);
		ImVec2 window_pos_pivot = ImVec2(0.0, 0.0);

		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowSize({ io.DisplaySize.x - 10.0f, 190.0f }, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.0f);
		if (ImGui::Begin(_name.c_str(), &_isOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground))
		{
			ImGui::Text("Demo");
			ImGui::End();
		}

	}

};


template<typename T>
struct MemberTraits : MemberTraits<decltype(T)> {};

template<typename T>
struct MemberTraits<std::vector<T>> {
	using type = T;
};

void HelloWorldGame::start()
{
#ifdef _DEBUG
	ExecuteRttiTest_BasicTypes();
#endif
	std::vector<int> myVec;

	rtti::TypeInfo* info  =rtti::Registry::get<MemberTraits<std::vector<int>>::type>();

	// Register default overlays to the overlay manager
	GameEngine::instance()->get_overlay_manager()->register_overlay(new GameOverlay());

	using namespace framework;
	_world = std::make_shared<framework::World>();
	GameEngine::instance()->get_overlay_manager()->register_overlay(new EntityDebugOverlay(_world.get()));

	_parentEntity = _world->create_entity(XMFLOAT2(100, 0));
	_parentEntity->create_component<BitmapComponent>(ResourcePaths::bmp_coin_silver);
	_parentEntity->create_component<SimpleMovement>();
	_parentEntity->set_name("Center");

	for (int i =0; i < 10; ++i)
	{
		float d = 360.0f * i / 10.0f;
		float x = cos(XMConvertToRadians(d)) * 100.0f;
		float y = sin(XMConvertToRadians(d)) * 100.0f;

		EntityHandle ent = _world->create_entity();
		ent->set_local_position(x, y);
		ent->create_component<BitmapComponent>(ResourcePaths::bmp_coin_gold);
		ent->attach_to(_parentEntity);

		std::string name = "Coin_" + std::to_string(i);
		ent->set_name(name);

		XMFLOAT3 pos = ent->get_world_position();
		printf("%.2f - %.2f - %.2f\n", pos.x, pos.y, pos.z);
	}



}

void HelloWorldGame::end()
{

}

void HelloWorldGame::paint(graphics::D2DRenderContext const& ctx)
{
	auto engine = GameEngine::instance();
	engine->DrawSolidBackground(COLOR(0, 0, 0));

	_world->render();
}

void HelloWorldGame::tick(double deltaTime)
{
	_world->update(float(deltaTime));
}

void HelloWorldGame::debug_ui()
{
	ImGui::Begin("Game");
	
	static float s_rotation_speed = 0.0;
	using namespace framework;
	if (ImGui::SliderFloat("Rotation Speed", &s_rotation_speed, 0.0f, 100.0f))
	{
		Entity* ent = _world->get_entity(_rotatorEntity);
		if (ent)
		{
			if (auto comp = ent->get_component<SimpleMovement>(); comp) {
				comp->_speed = s_rotation_speed / 10.0f;
			}
		}
	}

	static float s_angle = 0.0;
	if (ImGui::SliderFloat("Angle", &s_angle, 0.0f, 360.0f))
	{
		Entity* rot = _world->get_entity(_parentEntity);
		if (rot)
		{
			rot->set_rotation(XMConvertToRadians(s_angle));
		}
	}


	ImGui::End();
}

