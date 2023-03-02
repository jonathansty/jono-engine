#include "engine.pch.h"
#include "Widgets.h"
#include "Logging.h"

#include <shellapi.h>


bool ContentBrowser::s_Show = true;

std::string s_CurrentDirectory;

bool HasChildren(std::string dir)
{
    char tmp[MAX_PATH];
    sprintf_s(tmp, "%s\\*", dir.c_str());
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(tmp, &findData);

    if (findHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((strcmp(findData.cFileName, ".") == 0) || (strcmp(findData.cFileName, "..") == 0))
                continue;

            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                continue;


            return true;
        } while (FindNextFileA(findHandle, &findData) != 0);
        FindClose(findHandle);
    }

    return false;
}
void BuildFileTree(std::string dir)
{
    WIN32_FIND_DATAA findData;

    std::string f = dir + "\\*";
    HANDLE findHandle = FindFirstFileA(f.c_str(), &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((strcmp(findData.cFileName, ".") == 0) || (strcmp(findData.cFileName, "..") == 0))
                continue;

            if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                bool bHasChildren = HasChildren(dir + "\\" + std::string(findData.cFileName));
                bool bOpen = ImGui::TreeNodeEx(findData.cFileName, !bHasChildren ? ImGuiTreeNodeFlags_Leaf : 0);
                if(ImGui::IsItemClicked())
                {
                    LOG_INFO(Unknown, findData.cFileName);
                    s_CurrentDirectory = fmt::format("{}\\{}", dir, findData.cFileName);
                }
                if(bOpen)
                {
                    BuildFileTree(dir + "\\" + std::string(findData.cFileName));
                    ImGui::TreePop();
                }

            }
        } while (FindNextFileA(findHandle, &findData) != 0);
        FindClose(findHandle);
    }
}

void ContentBrowser::Build(ImGuiID* dockID)
{
    if (dockID && (*dockID != 0))
    {
        ImGui::SetNextWindowDockID(*dockID, ImGuiCond_Once);
    }

    if (ImGui::Begin("Content Browser", &s_Show, 0))
    {
        // #TODO: Implement recursive directory iteration in platform IO

        std::vector<const char*> files;


        char cwd[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, cwd);
        std::string root = std::string(cwd) + "\\Resources";

        if (ImGui::BeginChild("TreeView", ImVec2(300, 0)))
        {
            if (ImGui::TreeNodeEx("/", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth))
            {
                BuildFileTree(root);
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        if(ImGui::BeginChild("Assets", ImVec2(0,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar))
        {
            ImGui::Text("DIR: %s", s_CurrentDirectory.c_str());
            ImGui::Spacing();
            if(!s_CurrentDirectory.empty())
            {

                char dir[MAX_PATH];
                sprintf_s(dir, "%s\\*", s_CurrentDirectory.c_str());
                WIN32_FIND_DATAA findData;
                HANDLE findHandle = FindFirstFileA(dir, &findData);
                if (findHandle != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                        {
                            bool selected = false;
                            ImGui::Selectable(findData.cFileName, &selected);
                            if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            {

                                char tmp[MAX_PATH];
                                sprintf_s(tmp, "%s\\%s", s_CurrentDirectory.c_str(), findData.cFileName);

                                STARTUPINFOA startupInfo{};
                                PROCESS_INFORMATION info{};
                                ShellExecuteA(0, 0, tmp, 0, 0, SW_SHOW);
                            }
                        }

                    } while (FindNextFileA(findHandle, &findData));
                }
            }
        }
        ImGui::EndChild();

    }
    ImGui::End();



}
