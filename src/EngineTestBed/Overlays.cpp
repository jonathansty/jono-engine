#include "testbed.stdafx.h"
#include "RTTI/rtti.h"
#include "Overlays.h"

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

void EntityDebugOverlay::render_object(rtti::Object& obj) {
	ImGui::PushID(&obj);
	std::vector<rtti::TypeInfo*> chain;
	rtti::TypeInfo* parent = obj.get_type();
	while (parent) {
		chain.push_back(parent);
		parent = parent->_parent;
	}

	std::reverse(chain.begin(), chain.end());
	for (rtti::TypeInfo* t : chain) {
		for (auto const& prop : t->_properties) {
			ImGui::PushID(&prop);

			rtti::Property const& p = prop.second;
			if (p.type == rtti::Registry::get<float3>()) {
				float3 pos = obj.get_property<float3>(p.name);
				if (ImGui::InputFloat3(p.name.c_str(), (float*)&pos, 2)) {
					obj.set_property(p.name, pos);
				}
			} else if (p.type == rtti::Registry::get<float4>()) {
				float4 rot = obj.get_property<float4>(p.name);
				if (ImGui::InputFloat4(p.name.c_str(), (float*)&rot, 2)) {
					obj.set_property(p.name, rot);
				}
			} else if (p.type == rtti::Registry::get<float>()) {
				float v = obj.get_property<float>(p.name);
				if (ImGui::InputFloat(p.name.c_str(), &v, 0.01f, 0.1f, 2)) {
					obj.set_property(p.name, v);
				}
			} else if (p.type == rtti::Registry::get<int>()) {
				int v = obj.get_property<int>(p.name);
				if (ImGui::InputInt(p.name.c_str(), &v)) {
					obj.set_property(p.name, v);
				}
			} else if (p.type == rtti::Registry::get<std::string>()) {
				std::string v = obj.get_property<std::string>(p.name);

				char buff[512]{};
				assert(v.size() < 512);
				memcpy(buff, v.data(), v.size());
				if (ImGui::InputText(p.name.c_str(), buff, 512)) {
					obj.set_property<std::string>(p.name, buff);
				}
			}
			ImGui::PopID();
		}

		for (auto& fn : t->_functions) {
			ImGui::PushID(&fn);
			if (ImGui::Button(fn.first.c_str())) {
				fn.second->invoke(obj);
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
			rtti::TypeInfo* i = rot->get_type();
			rtti::Object obj = rtti::Object::create_as_ref(rot);

			ImGui::LabelText("Entity:", rot->get_name());
			render_object(obj);

			ImGui::Indent();
			for (framework::Component* comp : rot->_components) {
				ImGui::PushID(comp);
				rtti::Object compObj = rtti::Object::create_as_ref(comp);

				ImGui::Checkbox("", &comp->_active);
				ImGui::SameLine();
				ImGui::Text(compObj.get_type()->get_name());

				render_object(compObj);
				ImGui::PopID();
			}
			ImGui::Unindent();
		}
	}
	ImGui::End();
}

} // namespace framework
