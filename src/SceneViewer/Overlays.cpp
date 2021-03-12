#include "testbed.stdafx.h"
#include "Overlays.h"
#include <fmt/core.h>

namespace framework {

EntityDebugOverlay::EntityDebugOverlay(framework::World* world)
		: DebugOverlay(true, "EntityDebugOverlay")
		, _world(world)
		, _selected(nullptr) {
}

void EntityDebugOverlay::render_tree(framework::Entity* ent) {
	ImGuiTreeNodeFlags flags = (_selected == ent ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

	if (ent->_children.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx(ent, flags, ent->get_name())) {
		if (ImGui::IsItemToggledOpen()) {
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			_selected = ent;
		}

		for (framework::Entity* child : ent->_children) {
			render_tree(child);
		}

		ImGui::TreePop();
	} else {
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			_selected = ent;
		}
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
	for (rttr::type const& t : chain) {
		for (auto const& prop : t.get_properties()) {
			ImGui::PushID(&prop);

			rttr::property const& p = prop;
			std::string name = p.get_name().to_string();
			
			if (p.get_type() == rttr::type::get<WrapperFloat3>()) {
				float3 pos = p.get_value(obj).convert<WrapperFloat3>().value;
				if (ImGui::InputFloat3(name.c_str(), (float*)&pos, 2)) {
					p.set_value(obj, WrapperFloat3{ pos });
				}
			} else if (p.get_type() == rttr::type::get<WrapperFloat4>()) {
				float4 v = p.get_value(obj).convert<WrapperFloat4>().value;
				if (ImGui::InputFloat4(name.c_str(), (float*)&v, 2)) {
					p.set_value(obj, WrapperFloat4{ v });
				}
			} else  if (p.get_type() == rttr::type::get<float>()) {
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
				if (ImGui::InputText(name.c_str(), buff, 512)) {
					p.set_value(obj, buff);
				}
			}
			ImGui::PopID();
		}

		for (auto& fn : t.get_methods()) {
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

		std::vector<const char*> names{};
		std::vector<framework::Entity*> entities{};
		for (framework::Entity* ent : _world->_entities) {
			names.push_back(ent->get_name());
			entities.push_back(ent);
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

			ImGui::LabelText("Entity:", rot->get_name());
			rttr::instance ent_inst = *rot;
			render_object(ent_inst);

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
