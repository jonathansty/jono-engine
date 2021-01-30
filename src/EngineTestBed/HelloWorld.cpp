#include "testbed.stdafx.h"
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

	rtti::TypeInfo* info = rtti::Registry::get<MemberTraits<std::vector<int>>::type>();

	rtti::Object obj = rtti::Object::create<SimpleMovement2D>();
	info = rtti::Registry::get<SimpleMovement2D>();
	rtti::FunctionBase const* fn = info->find_function("reset");
	fn->invoke(obj);

	// Register default overlays to the overlay manager
	GameEngine::instance()->get_overlay_manager()->register_overlay(new GameOverlay());

	using namespace framework;
	_world = std::make_shared<framework::World>();
	GameEngine::instance()->get_overlay_manager()->register_overlay(new EntityDebugOverlay(_world.get()));

	_parentEntity = _world->create_entity(float2(100, 100));
	_parentEntity->create_component<BitmapComponent>(ResourcePaths::bmp_coin_silver);
	_parentEntity->create_component<SimpleMovement2D>();
	_parentEntity->set_name("Center");

	for (int i =0; i < 10; ++i)
	{
		float d = 360.0f * i / 10.0f;
		float x = cos(hlslpp::radians(hlslpp::float1(d))) * 100.0f;
		float y = sin(hlslpp::radians(hlslpp::float1(d))) * 100.0f;

		EntityHandle ent = _world->create_entity();
		ent->set_local_position(x, y);
		ent->create_component<BitmapComponent>(ResourcePaths::bmp_coin_gold);
		ent->attach_to(_parentEntity);

		std::string name = "Coin_" + std::to_string(i);
		ent->set_name(name);

		float4 pos = ent->get_world_position();
		printf("%.2f - %.2f - %.2f\n", float(pos.x), float(pos.y), float(pos.z));
	}



}

void HelloWorldGame::end()
{

}

void HelloWorldGame::paint(graphics::D2DRenderContext& ctx)
{
	ctx.draw_background(COLOR(0, 0, 0));

	std::vector<framework::Entity*> entities = _world->get_entities();
	for (framework::Entity const* ent : entities) {

		float4x4 transform = ent->get_world_transform();

		float3x3 c = float3x3(transform._11_12_14, transform._21_22_24, transform._41_42_44);
		ctx.set_world_matrix(c);

		if(BitmapComponent* comp = ent->get_component<BitmapComponent>(); comp)	{
			// center
			float2 pivot = float2(-comp->_bmp->GetWidth() / 2.0f, -comp->_bmp->GetHeight() / 2.0f);
			ctx.draw_bitmap(comp->_bmp, pivot.x, pivot.y);
		}
	}
	//_world->render();
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
			if (auto comp = ent->get_component<SimpleMovement2D>(); comp) {
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
			rot->set_rotation(hlslpp::radians(hlslpp::float1(s_angle)));
		}
	}


	ImGui::End();
}

