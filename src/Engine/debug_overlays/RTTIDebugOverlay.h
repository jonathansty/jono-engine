#pragma once
#include "OverlayManager.h"
#include "rtti/rtti.h"

class RTTIDebugOverlay : public DebugOverlay
{

public:
	RTTIDebugOverlay() : DebugOverlay(false, "RTTIOverlay") {}
	~RTTIDebugOverlay() {}

	virtual void render_overlay() override
	{
		if (_isOpen)
		{
			ImGui::Begin("Types", &_isOpen);

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
	}
};
