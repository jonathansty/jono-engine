require "dependencies"

-- Directories
SourceDirectory = "Source/"
ExternalDirectory = "External/"

-- premake5.lua
workspace "jono-engine"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    cppdialect "C++20"
    architecture "x86_64"
    systemversion "latest"
    language "C++"
    -- TODO: Disable exception handling and rtti
    --   Blocked by fmt library and some command line parsing
    exceptionhandling "On"
    rtti "On"
    objdir "%{wks.location}/obj"
    targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
    toolset "msc"

    includedirs { SourceDirectory}

    filter "toolset:msc*"
        flags
        {
            "multiprocessorcompile",
        }
        buildoptions
        {
            "/permissive-",
        }
        defines { "WIN64"}
   
group "Libraries"
    LibraryProject(LibEnkiTS)
    LibraryProject(LibOptick)
    LibraryProject(LibImgui)

    LibraryProject(LibImPlot)
        LinkLibrary(LibImgui)

group "Engine"

project "CLI"
    kind "StaticLib"
    language "C++"  
    targetdir "bin/cli/%{cfg.buildcfg}"
    files 
    { 
        SourceDirectory .. "CLI/**.h", 
        SourceDirectory .. "CLI/**.cpp" 
    }
   includedirs { ExternalDirectory .. "fmt/include" }
   AddLibraryIncludes(LibFmt)

project "Core"
    kind "StaticLib"
    language "C++"  
    targetdir "bin/core/%{cfg.buildcfg}"
    files 
    { 
        SourceDirectory .. "Core/**.h", 
        SourceDirectory .. "Core/**.cpp" 
    }
    includedirs { SourceDirectory .. "CLI" }
    includedirs { SourceDirectory .. "Core" }

    LinkLibrary(LibEnkiTS)
    AddLibraryIncludes(LibBox2D)
    AddLibraryIncludes(LibFmt)
    AddLibraryIncludes(LibHlslpp)
    dependson { "CLI" }

project "Graphics"
    kind "StaticLib"
    language "C++"  
    targetdir "bin/Graphics/%{cfg.buildcfg}"
    files 
    { 
        SourceDirectory .. "Graphics/**.h", 
        SourceDirectory .. "Graphics/**.cpp" 
    }
    dependson {"Core"}
    links { "Core" }

    includedirs { SourceDirectory .. "Graphics" }
    AddLibraryIncludes(LibHlslpp)
    AddLibraryIncludes(LibBox2D)
    AddLibraryIncludes(LibFmt)
    LinkLibrary(LibEnkiTS)



project "Engine"
   kind "StaticLib"
   language "C++"
   targetdir "bin/engine/%{cfg.buildcfg}"

   files 
   { 
    SourceDirectory .. "Engine/**.h", 
    SourceDirectory .. "Engine/**.cpp",
   -- SourceDirectory .. "Engine/**.hlsl" 
   }

   links { "Core", "Graphics" }
   includedirs { 
        SourceDirectory .. "/Engine", 
        SourceDirectory .. "/CLI", 
        SourceDirectory .. "/Core", 
        SourceDirectory .. "/Graphics"
    }
    AddLibraryIncludes(LibHlslpp)
    AddLibraryIncludes(LibFmt)

    LinkLibrary(LibEnkiTS)
    LinkLibrary(LibBox2D)
    LinkLibrary(LibOptick)
    LinkLibrary(LibAssimp)
    LinkLibrary(LibImgui)
    LinkLibrary(LibImPlot)
    LinkLibrary(LibSDL2)
    LinkLibrary(LibDirectXTK)

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

group "Games"

project "SceneViewer"
    kind "ConsoleApp"
    language "C++"  
    targetdir "bin/SceneViewer/%{cfg.buildcfg}"
    files 
    { 
        SourceDirectory .. "SceneViewer/**.h", 
        SourceDirectory .. "SceneViewer/**.cpp" 
    }
    includedirs
    {
        SourceDirectory .. "Engine",
        SourceDirectory .. "Core",
        SourceDirectory .. "CLI",
        SourceDirectory .. "Graphics"
    }

    links { 
        "Core", 
        "Graphics", 
        "Engine",
        "CLI" 
    }

    AddLibraryIncludes(LibHlslpp)
    AddLibraryIncludes(LibFmt)

    LinkLibrary(LibEnkiTS)
    LinkLibrary(LibBox2D)
    LinkLibrary(LibOptick)
    LinkLibrary(LibAssimp)
    LinkLibrary(LibImgui)
    LinkLibrary(LibImPlot)
    LinkLibrary(LibSDL2)
    LinkLibrary(LibDirectXTK)
