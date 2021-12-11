#include "sceneviewer.pch.h"
#include "Overlays.h"
#include <fmt/core.h>

#include "core/Math.h"

namespace framework {

EntityDebugOverlay::EntityDebugOverlay(framework::World* world)
		: DebugOverlay(true, "EntityDebugOverlay")
		, _world(world)
		, _selected() {
}

void EntityDebugOverlay::render_tree(framework::EntityHandle ent) {
	ImGuiTreeNodeFlags flags = (_selected == ent ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

	flags |= ImGuiTreeNodeFlags_DefaultOpen;
	if (ent->_children.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx(ent, flags, ent->get_name())) {
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			_selected = ent;
		}

		for (framework::EntityHandle child : ent->_children) {
			if(child.is_valid())
				render_tree(child);
		}

		ImGui::TreePop();
	} else {
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			_selected = ent;
		}
	}

	if (_selected && _selected.get() != _world->_root && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete))) {
		
		_world->remove_entity(_selected);
	}
}

void EntityDebugOverlay::render_object(rttr::instance& obj) {
	ImGui::PushID(&obj);
	std::vector<rttr::type> chain;
	rttr::type parent = obj.get_derived_type();
	while (!parent.get_base_classes().empty()) {
		chain.push_back(parent);
		parent = *(parent.get_base_classes().begin());
	}
	chain.push_back(parent);

	std::reverse(chain.begin(), chain.end());
	for (rttr::type const& chainType : chain) {
		for (auto const& prop : chainType.get_properties()) {
			ImGui::PushID(&prop);

			rttr::property const& p = prop;
			std::string name = p.get_name().to_string();
			
			auto t = p.get_type();
			if (t == rttr::type::get<WrapperFloat3>()) {
				float3 pos = p.get_value(obj).convert<WrapperFloat3>().value;
				if (ImGui::InputFloat3(name.c_str(), (float*)&pos, 2)) {
					p.set_value(obj, WrapperFloat3{ pos });
				}
			} else if (t == rttr::type::get<WrapperFloat4>()) {
				float4 v = p.get_value(obj).convert<WrapperFloat4>().value;
				if (ImGui::InputFloat4(name.c_str(), (float*)&v, 2)) {
					p.set_value(obj, WrapperFloat4{ v });
				}
			} else if (t == rttr::type::get<WrapperQuat>()) {
				hlslpp::quaternion quat = p.get_value(obj).convert<WrapperQuat>().value;
				hlslpp::float3 euler = hlslpp::degrees(hlslpp_helpers::to_euler(quat));
				if (ImGui::InputFloat3(name.c_str(), (float*)&euler, 2)) {
					p.set_value(obj, WrapperQuat{ hlslpp::euler(hlslpp::radians(euler)) });
				}
			} else if (t == rttr::type::get<float>()) {
				float v = p.get_value(obj).convert<float>();
				if (ImGui::InputFloat(p.get_name().to_string().c_str(), &v, 0.01f, 0.1f, 2)) {
					p.set_value(obj, v);
				}
			} else if (p.get_type() == rttr::type::get<int>()) {
				int v = p.get_value(obj).convert<int>();
				if (ImGui::InputInt(name.c_str(), &v)) {
					p.set_value(obj, v);
				}
			} else if (p.get_type() == rttr::type::get<std::string>()) {
				std::string v = p.get_value(obj).convert<std::string>();

				char buff[512]{};
				assert(v.size() < 512);
				memcpy(buff, v.data(), v.size());
				if (ImGui::InputText(name.c_str(), buff, 512, ImGuiInputTextFlags_EnterReturnsTrue)) {
					p.set_value(obj, std::string(buff));
				}
			}
			ImGui::PopID();
		}

		for (auto& fn : chainType.get_methods()) {
			ImGui::PushID(&fn);
			if (ImGui::Button(fn.get_name().to_string().c_str())) {
				fn.invoke(obj);
			}
			ImGui::PopID();
		}
	}
	ImGui::PopID();
}

void EntityDebugOverlay::render_overlay() {
	static int s_current = 0;
	if (ImGui::Begin("Scene Outliner", &_isOpen)) {
		ImGui::Text("Number Of Entities: %d", _world->_entities.size());

		static char tmp[255]{};
		ImGui::PushID("#EntityName");
		ImGui::InputText("", tmp, 255, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopID();
		ImGui::SameLine();
		if (ImGui::Button("Create")) {
			auto handle = _world->create_entity();
			handle->set_name(tmp);

			if (_selected) {
				_world->attach_to(_selected, handle);
			}
		}

		std::vector<const char*> names{};
		std::vector<framework::Entity*> entities{};
		for (framework::Entity* ent : _world->_entities) {
			if(ent) {
				names.push_back(ent->get_name());
				entities.push_back(ent);
			}
		}

		render_tree(_world->_root);
	}
	ImGui::End();

	bool open = _selected;
	if (ImGui::Begin("Entity Properties", &open)) {
		// Display type
		if (_selected) {
			ImGui::Separator();
			framework::Entity* rot = _selected;
			rttr::type const& i = rttr::type::get(*rot);

			ImGui::LabelText("Entity", rot->get_name());
			ImGui::LabelText("Id", "%d", _selected.id);
			ImGui::LabelText("Generation", "%d", _selected.generation);
			rttr::instance ent_inst = *rot;
			render_object(ent_inst);

			auto types = rttr::type::get<Component>().get_derived_classes();
			std::vector<const char*> type_names;
			for (auto t : types) {
				type_names.push_back(t.get_name().data());
			}
			static int s_selected_type = 0;
			ImGui::PushID("Type");
			ImGui::Combo("",&s_selected_type, type_names.data(), int(type_names.size()));
			ImGui::PopID();
			ImGui::SameLine();
			if (ImGui::Button("+")) {
				auto t = rttr::type::get_by_name(type_names[s_selected_type]);
				rttr::variant obj = t.create();
				rot->create_component(t);
			}

			ImGui::Indent();
			for (framework::Component* comp : rot->_components) {
				ImGui::PushID(comp);

				rttr::type t = rttr::type::get(*comp);
				ImGui::Checkbox("", &comp->_active);
				ImGui::SameLine();

				ImGui::Text(t.get_name().to_string().c_str());

				rttr::instance cmp_inst = *comp;
				render_object(cmp_inst);
				ImGui::PopID();
			}
			ImGui::Unindent();
		}
	}
	ImGui::End();
}

} // namespace framework
