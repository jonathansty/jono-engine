#include "engine.pch.h"
#include "Widgets.h"

#include "Core/Logging.h"

bool DebugLog::s_Show = true;

void DebugLog::Build(ImGuiID* dockID)
{
    if (dockID && (*dockID != 0))
    {
        ImGui::SetNextWindowDockID(*dockID, ImGuiCond_Once);
    }

    if (ImGui::Begin("Output Log", &s_Show, 0))
    {
        static bool s_scroll_to_bottom = true;
        static bool s_shorten_file = true;
        static char s_filter[256] = {};
        ImGui::InputText("Filter", s_filter, 512);
        ImGui::SameLine();
        ImGui::Checkbox("Scroll To Bottom", &s_scroll_to_bottom);
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            Logger::instance()->clear();
        }

        // ImGui::BeginTable("LogData", 3);
        ImGui::BeginChild("123", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
        auto const& buffer = Logger::instance()->GetBuffer();
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
        {
            LogEntry const* entry = *it;
            if (s_filter[0] != '\0')
            {
                std::string msg = entry->to_message();
                bool bPassed = (msg.find(s_filter) != std::string::npos);
                if (!bPassed)
                    continue;
            }

            switch (entry->_severity)
            {
                case Logger::Severity::Info:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3, 0.3, 1.0, 1.0));
                    break;
                case Logger::Severity::Warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.1, 1.0));
                    break;
                case Logger::Severity::Error:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9, 0.1, 0.1, 1.0));
                    break;
                case Logger::Severity::Verbose:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
                default:
                    break;
            }

            std::string filename = entry->_file ? entry->_file : "null";
            if (s_shorten_file && entry->_file)
            {
                auto path = std::filesystem::path(filename);
                filename = fmt::format("{}", path.filename().string());
            }
            ImGui::Text("[%s(%d)][%s][%s] %s", filename.c_str(), it->_line, logging::to_string(it->_category), logging::to_string(it->_severity), it->_message.c_str());
            ImGui::PopStyleColor();
        }
        if (Logger::instance()->_hasNewMessages && s_scroll_to_bottom)
        {
            Logger::instance()->_hasNewMessages = false;
            ImGui::SetScrollHereY();
        }
        // ImGui::EndTable();
        ImGui::EndChild();
    }
    ImGui::End();
}
