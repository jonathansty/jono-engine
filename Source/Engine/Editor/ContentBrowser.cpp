#include "engine.pch.h"
#include "Widgets.h"
#include "Logging.h"

#include <shellapi.h>


bool ContentBrowser::s_Show = true;

std::string s_CurrentDirectory;
std::string s_SelectedFile;

struct
{
} s_ToolTipInfo;

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
    static std::string s_dir;

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
                bool bOpen = ImGui::TreeNodeEx(findData.cFileName, !bHasChildren ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_OpenOnDoubleClick);

                std::string d = fmt::format("{}\\{}", dir, findData.cFileName);
                s_dir = d;
                if(ImGui::IsItemClicked())
                {
                    LOG_INFO(Unknown, findData.cFileName);
                    s_CurrentDirectory = s_dir;
                    s_SelectedFile.clear();
                }

                if (ImGui::BeginPopup("dir_menu"))
                {
                    ImGui::Text("Directory");
                    ImGui::Separator();
                    if (ImGui::Selectable("Explore"))
                    {
                        std::filesystem::path tmp = d;

                        STARTUPINFOA startupInfo{};
                        PROCESS_INFORMATION info{};
                        ShellExecute(0, 0, tmp.c_str(), 0, 0, SW_SHOW);
                    }
                    ImGui::EndPopup();
                }


                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    ImGui::OpenPopup("dir_menu");
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

        if(ImGui::BeginChild("Assets", ImVec2(800,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar))
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
                            char tmp[MAX_PATH];
                            sprintf_s(tmp, "%s\\%s", s_CurrentDirectory.c_str(), findData.cFileName);

                            bool selected = false;
                            if(strcmp(tmp, s_SelectedFile.c_str()) == 0)
                            {
                                selected = true;
                            }

                            if(ImGui::Selectable(findData.cFileName, &selected))
                            {
                                selected = true;
                                s_SelectedFile = tmp;
                            }

                            if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            {
                                STARTUPINFOA startupInfo{};
                                PROCESS_INFORMATION info{};
                                ShellExecuteA(0, 0, tmp, 0, 0, SW_SHOW);
                            }

                            if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                            {
                                ImGui::OpenPopup("file_menu");
                                s_SelectedFile = tmp;
                            }
                        }

                    } while (FindNextFileA(findHandle, &findData));
                }
            }
        }
        if (ImGui::BeginPopup("file_menu"))
        {
            ImGui::Text("File");
            ImGui::Separator();
            if(ImGui::Selectable("Open"))
            {
                STARTUPINFOA startupInfo{};
                PROCESS_INFORMATION info{};
                ShellExecuteA(0, 0, s_SelectedFile.c_str(), 0, 0, SW_SHOW);
            }

            if(ImGui::Selectable("Explore"))
            {
                std::filesystem::path tmp = s_SelectedFile;

                STARTUPINFOA startupInfo{};
                PROCESS_INFORMATION info{};
                ShellExecute(0, 0, tmp.parent_path().c_str(), 0, 0, SW_SHOW);
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();
        ImGui::SameLine();


        if(ImGui::BeginChild("File", ImVec2(200, 0), false, 0))
        {
            if(!s_SelectedFile.empty())
            {
                WIN32_FIND_DATAA findData;
                HANDLE hFileHandle = FindFirstFileA(s_SelectedFile.c_str(), &findData);
                if (hFileHandle != INVALID_HANDLE_VALUE)
                {
                    uint32_t fileSize = (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;
                    ImGui::LabelText("Size", "%d (bytes)", fileSize);
                }
            }

        }
        ImGui::EndChild();

    }
    ImGui::End();



}
