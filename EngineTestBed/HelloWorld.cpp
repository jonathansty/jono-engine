#include "stdafx.h"
#include "HelloWorld.h"

#include "EngineFiles/Framework/framework.h"
#include "core/identifier.h"

#include "Components.h"

extern void ExecuteRttiTest_BasicTypes();


class ResourcePaths
{
public:
	static constexpr const char* bmp_coin_bronze = "Resources/Pickups/coinBronze.png";
	static constexpr const char* bmp_coin_silver = "Resources/Pickups/coinSilver.png";
	static constexpr const char* bmp_coin_gold = "Resources/Pickups/coinGold.png";
};

class EntityDebugOverlay : public DebugOverlay
{
public:
	EntityDebugOverlay(framework::World* world)
		: DebugOverlay(false, "EntityDebugOverlay")
		, _world(world)
	{
	}


	void RenderTree(framework::Entity* ent)
	{
		ImGuiTreeNodeFlags flags = (_selected == ent ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

		if (ent->_children.size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		if (ImGui::TreeNodeEx(ent, flags, ent->get_name()))
		{
			if (ImGui::IsItemToggledOpen())
			{

			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				_selected = ent;
			}


			for (framework::Entity* child : ent->_children)
			{
				RenderTree(child);
			}

			ImGui::TreePop();
		}
		else {
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				_selected = ent;
			}
		}
	}

	void RenderObject(rtti::Object& obj)
	{
		ImGui::PushID(&obj);
		std::vector<rtti::TypeInfo*> chain;
		rtti::TypeInfo* parent = obj.get_type();
		while (parent)
		{
			chain.push_back(parent);
			parent = parent->_parent;
		}

		std::reverse(chain.begin(), chain.end());
		for (rtti::TypeInfo* t : chain)
		{
			for (auto const& prop : t->_properties)
			{
				ImGui::PushID(&prop);

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
				else if (p.type == rtti::Registry::get<float>())
				{
					float* v = obj.get_property<float>(p.name);
					ImGui::InputFloat(p.name.c_str(), v, 0.01f, 0.1f, 2);
				}
				else if (p.type == rtti::Registry::get<int>())
				{
					int* v = obj.get_property<int>(p.name);
					ImGui::InputInt(p.name.c_str(), v);
				}
				else if (p.type == rtti::Registry::get<std::string>())
				{
					std::string* v = obj.get_property<std::string>(p.name);

					char buff[512]{};
					assert(v->size() < 512);
					memcpy(buff, v->data(), v->size());
					if (ImGui::InputText(p.name.c_str(), buff, 512))
					{
						*v = buff;
					}

				}
				ImGui::PopID();
			}
		}
		ImGui::PopID();
	}

	virtual void render_overlay() override
	{
		if (_isOpen)
		{
			static int s_current = 0;
			ImGui::Begin(_name.c_str(), &_isOpen);
			ImGui::Text("Number Of Entities: %d", _world->_entities.size());

			std::vector<const char*> names{};
			std::vector<framework::Entity*> entities{};
			for (framework::Entity* ent : _world->_entities)
			{
				names.push_back(ent->get_name());
				entities.push_back(ent);
			}

			RenderTree(_world->_root);

			if (_selected)
			{
				// Display type
				{
					ImGui::Separator();
					framework::Entity* rot = _selected;
					rtti::TypeInfo* i = rot->get_type();
					rtti::Object obj = rtti::Object::create_as_ref(rot);

					ImGui::LabelText("Entity:", rot->get_name());
					RenderObject(obj);

					for (framework::Component* comp : rot->_components)
					{
						rtti::Object compObj = rtti::Object::create_as_ref(comp);
						ImGui::Text(compObj.get_type()->get_name());
						RenderObject(compObj);
					}
				}
			}

			ImGui::End();
		}
	}

private:
	framework::Entity* _selected;
	framework::World* _world;
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

void HelloWorldGame::GameStart()
{
#ifdef _DEBUG
	ExecuteRttiTest_BasicTypes();
#endif
	std::vector<int> myVec;

	rtti::TypeInfo* info  =rtti::Registry::get<MemberTraits<std::vector<int>>::type>();

	// Register default overlays to the overlay manager
	GameEngine::Instance()->get_overlay_manager()->register_overlay(new GameOverlay());

	using namespace framework;
	_world = std::make_unique<framework::World>();
	GameEngine::Instance()->get_overlay_manager()->register_overlay(new EntityDebugOverlay(_world.get()));

	_parentEntity = _world->create_entity(XMFLOAT2(100, 0));
	_world->get_entity(_parentEntity)->create_component<BitmapComponent>(ResourcePaths::bmp_coin_silver);
	_world->get_entity(_parentEntity)->create_component<SimpleMovement>();
	_world->get_entity(_parentEntity)->set_name("Center");

	for (int i =0; i < 10; ++i)
	{
		float d = 360.0f * i / 10.0f;
		float x = cos(XMConvertToRadians(d)) * 100.0f;
		float y = sin(XMConvertToRadians(d)) * 100.0f;

		World::EntityId ent = _world->create_entity();
		_world->get_entity(ent)->set_local_position(x, y);
		_world->get_entity(ent)->create_component<BitmapComponent>(ResourcePaths::bmp_coin_gold);
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

