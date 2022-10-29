#pragma once
#include "OverlayManager.h"

class RTTIDebugOverlay : public DebugOverlay
{

public:
	RTTIDebugOverlay() : DebugOverlay(false, "RTTIOverlay") {}
	~RTTIDebugOverlay() {}

	virtual void RenderOverlay() override
	{
		if (_isOpen)
		{
			ImGui::Begin("Types", &_isOpen);

			using namespace rttr;
			ImGui::Columns(5);
			ImGui::Text("Key");
			ImGui::NextColumn();

			ImGui::Text("Name");
			ImGui::NextColumn();

			ImGui::Text("Size");
			ImGui::NextColumn();

			ImGui::Text("IsClass"); ImGui::NextColumn(); ImGui::Text("IsArithmic"); ImGui::NextColumn();

			ImGui::Separator();

			// Loop over each type
			std::for_each(rttr::type::get_types().begin(), rttr::type::get_types().end(), [](rttr::type const& info) {
				ImGui::Text("%u", (unsigned int)(info.get_id()));
				ImGui::NextColumn();

				ImGui::Text("%s", info.get_name().to_string().c_str());
				ImGui::NextColumn();

				ImGui::Text("%d", int(info.get_sizeof()));
				ImGui::NextColumn();

				bool is_class = info.is_class();
				ImGui::Checkbox("##is_class", &is_class);
				ImGui::NextColumn();

				bool is_arithmetic = info.is_arithmetic();
				ImGui::Checkbox("##is_arithmetic", &is_arithmetic);
				ImGui::NextColumn();
			});
			ImGui::End();
		}
	}
};
