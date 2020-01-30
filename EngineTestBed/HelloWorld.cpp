#include "stdafx.h"
#include "HelloWorld.h"

#include "EngineFiles/Framework/framework.h"
#include "core/identifier.h"

extern void ExecuteRttiTest_BasicTypes();

class SimpleMovement : public framework::Component
{
	REFLECT(SimpleMovement)
public:
	SimpleMovement() : Component("") {}

	SimpleMovement(std::string const& name, XMFLOAT2 pos, float speed)
		: Component(name)
		, _speed(speed)
		, _elapsed(0.0)
		, _offset(pos)

	{
	}

	~SimpleMovement()
	{
	}

	void update(float dt) override
	{
		_elapsed += dt*_speed;
		framework::Entity* ent = get_entity();
		ent->set_position(_offset.x + cos(_elapsed) * 100.0, _offset.y + sin(_elapsed) * 100.0);
	}

	XMFLOAT2 _offset;
	float _elapsed = 0.0;
	float _speed;

};

class BitmapComponent : public framework::Component
{
	REFLECT(BitmapComponent)
public:
	BitmapComponent() : framework::Component("")
	{

	}

	BitmapComponent(std::string const& name, std::string const& path) 
		: framework::Component(name)
	{
		_bmp = new Bitmap(String(path.c_str()));
	}
	~BitmapComponent() 
	{
		delete _bmp;
	}

	void render() 
	{
		XMFLOAT3 pos = get_entity()->get_world_position();
		GameEngine::Instance()->DrawBitmap(_bmp, pos.x, pos.y);
	}


private:
	Bitmap* _bmp;

};

IMPL_REFLECT(BitmapComponent) {}
IMPL_REFLECT(SimpleMovement) {}

class ResourcePaths
{
public:
	static constexpr const char* bmp_coin_bronze = "Resources/Pickups/coinBronze.png";
	static constexpr const char* bmp_coin_silver = "Resources/Pickups/coinSilver.png";
	static constexpr const char* bmp_coin_gold = "Resources/Pickups/coinGold.png";
};

void HelloWorldGame::GameStart()
{
#ifdef _DEBUG
	ExecuteRttiTest_BasicTypes();
#endif

	using namespace framework;
	_world = std::make_unique<framework::World>();

	//_rotatorEntity = _world->create_entity(XMFLOAT2(50, 50));
	//_world->get_entity(_rotatorEntity)->create_component<SimpleMovement>("Movement", XMFLOAT2{ 100,100 }, 10.f);
	//_world->get_entity(_rotatorEntity)->create_component<BitmapComponent>("Sprite", ResourcePaths::bmp_coin_bronze);

	_parentEntity = _world->create_entity(XMFLOAT2(100, 0));
	_world->get_entity(_parentEntity)->create_component<BitmapComponent>("",ResourcePaths::bmp_coin_silver);
	_world->get_entity(_parentEntity)->set_name("Center");

	for (int i =0; i < 10; ++i)
	{
		float d = 360.0 * i / 10.0;
		float x = cos(XMConvertToRadians(d)) * 100.0;
		float y = sin(XMConvertToRadians(d)) * 100.0;

		World::EntityId ent = _world->create_entity();
		_world->get_entity(ent)->set_position(x, y);
		_world->get_entity(ent)->create_component<BitmapComponent>("Unnamed", ResourcePaths::bmp_coin_gold);
		_world->get_entity(ent)->attach_to(_world->get_entity(_parentEntity));

		std::string name = "Coin_" + std::to_string(i);
		_world->get_entity(ent)->set_name(name);

		XMFLOAT3 pos = _world->get_entity(ent)->get_world_position();
		printf("%.2f - %.2f - %.2f\n", pos.x, pos.y, pos.z);
	}



}

void HelloWorldGame::GameEnd()
{

}

void HelloWorldGame::GamePaint(RECT rect)
{
	auto engine = GameEngine::Instance();
	engine->DrawSolidBackground(COLOR(0, 0, 0));

	_world->render();
}

void HelloWorldGame::GameTick(double deltaTime)
{
	_world->update(float(deltaTime));
}

void HelloWorldGame::DebugUI()
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
				comp->_speed = s_rotation_speed / 10.0;
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

	// Display type
	{
		ImGui::Separator();
		Entity* rot = _world->get_entity(_parentEntity);
		rtti::TypeInfo* i = rot->get_type();
		rtti::Object obj = rtti::Object::create_as_ref(rot);

		ImGui::LabelText("Entity:", rot->get_name());
		for (auto const& prop : i->_properties)
		{
			rtti::Property const& p = prop.second;
			
			if (p.type == rtti::Registry::get<XMFLOAT3>())
			{
				XMFLOAT3* pos = obj.get_property<XMFLOAT3>(p.name);
				ImGui::InputFloat3(p.name.c_str(), (float*)pos, 2);

			}
			else if (p.type == rtti::Registry::get<XMVECTOR>())
			{
				XMVECTOR* rot = obj.get_property<XMVECTOR>(p.name);
				ImGui::InputFloat4(p.name.c_str(), (float*)rot, 2);
			}
		}
	}

	ImGui::End();
	static bool s_open = false;
	ImGui::Begin("Types", &s_open);

	using namespace rtti;
	ImGui::Columns(4);
	ImGui::Text("Key");
	ImGui::NextColumn();

	ImGui::Text("Name");
	ImGui::NextColumn();

	ImGui::Text("Size");
	ImGui::NextColumn();

	ImGui::Text("Primitive");
	ImGui::NextColumn();
	ImGui::Separator();

	// Loop over each type
	Registry::for_each_type([](std::pair<std::type_index, TypeInfo*> info) {
		ImGui::Text("%u", info.first.hash_code());
		ImGui::NextColumn();

		ImGui::Text("%s", info.second->get_name());
		ImGui::NextColumn();

		ImGui::Text("%d", info.second->get_size());
		ImGui::NextColumn();

		bool is_primive = info.second->is_primitive();
		ImGui::Checkbox("", &is_primive);
		ImGui::NextColumn();

	});

	ImGui::End();
}
