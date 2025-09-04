DependenciesRoot = "External/"

LibAssimp = {
    Name = "assimp",
    Path = DependenciesRoot .. "assimp/",
    Includes = DependenciesRoot .. "assimp/binaries/include",
    LibDirs = DependenciesRoot .. "assimp/binaries/lib",
    Library = "assimp-vc143-mt"
}

LibImPlot = {
    Name = "implot",
    Path = DependenciesRoot .. "implot/",
    Includes = DependenciesRoot .. "implot/",
    Files = {
        DependenciesRoot .. "implot/implot.h",
        DependenciesRoot .. "implot/implot.cpp",
        DependenciesRoot .. "implot/implot_internal.h",
        DependenciesRoot .. "implot/implot_items.cpp",
        DependenciesRoot .. "implot/implot_demo.cpp"
    }
}

LibImgui = {
    Name = "imgui",
    Path = DependenciesRoot .. "imgui/",
    Includes = DependenciesRoot .. "imgui/",
    Files = {
        DependenciesRoot .. "imgui/imgui.h",
        DependenciesRoot .. "imgui/imgui.cpp",
        DependenciesRoot .. "imgui/imgui_draw.cpp",
        DependenciesRoot .. "imgui/imgui_internal.h",
        DependenciesRoot .. "imgui/imgui_tables.cpp",
        DependenciesRoot .. "imgui/imgui_widgets.cpp",
        DependenciesRoot .. "imgui/imgui_rectpack.h",
        DependenciesRoot .. "imgui/imstb_rectpack.h",
        DependenciesRoot .. "imgui/imstb_textedit.h",
        DependenciesRoot .. "imgui/imstb_truetype.h",
        DependenciesRoot .. "imgui/imconfig.h",
        DependenciesRoot .. "imgui/misc/debuggers/imgui.natvis",
        DependenciesRoot .. "imgui/backends/imgui_impl_dx11.h",
        DependenciesRoot .. "imgui/backends/imgui_impl_dx11.cpp",
        DependenciesRoot .. "imgui/backends/imgui_impl_win32.cpp",
        DependenciesRoot .. "imgui/backends/imgui_impl_win32.h"
    }
}

LibSDL2 = {
    Name = "SDL2",
    Path = DependenciesRoot .. "SDL2/SDL2-2.24.1/",
    Includes = DependenciesRoot .. "SDL2/SDL2-2.24.1/include",
    LibDirs = DependenciesRoot .. "SDL2/SDL2-2.24.1/lib/x64"
}
LibHlslpp = {
    Name = "hlslpp",
    Path = DependenciesRoot .. "hlslpp/",
    Includes = DependenciesRoot .. "hlslpp/include",
}

LibBox2D = {
    Name = "box2d",
    Path = DependenciesRoot .. "box2d/",
    LibDirs = DependenciesRoot .. "box2d/bin",
    Includes = DependenciesRoot .. "box2d/include",
}
LibFmt = {
    Name = "fmt",
    Path = DependenciesRoot .. "fmt/",
    Includes = DependenciesRoot .. "fmt/include",
}
LibEnkiTS = {
    Name = "EnkiTS",
    Library = "EnkiTS",
    Path = DependenciesRoot .. "EnkiTS/",
    Includes = DependenciesRoot .. "EnkiTS/src",
    Files = {
        DependenciesRoot .. "EnkiTS/src/**.h",
        DependenciesRoot .. "EnkiTS/src/**.cpp"
    }
}

LibOptick = {
    Name = "Optick",
    Path = DependenciesRoot .. "Optick/",
    Includes = DependenciesRoot .. "Optick/include",
    Files = {
        DependenciesRoot .. "Optick/src/**.h",
        DependenciesRoot .. "Optick/src/**.cpp"
    }
}

LibDirectXTK = {
    Name = "DirectXTK",
    Path = DependenciesRoot .. "DirectXTK/",
    Includes = DependenciesRoot .. "DirectXTK/Inc",
    LibDirs = DependenciesRoot .. "DirectXTK/Bin/Desktop_2022/x64/Release",
    Library = "DirectXTK"
}

function LinkLibrary(Library)
    includedirs { Library.Includes }
    libdirs { Library.LibDirs }
    if Library.Library ~= nil then
        links { Library.Library }
    end
end

function AddLibraryIncludes(Library)
    includedirs 
    { 
        Library.Includes
    }
end

function LibraryProject(Library)
    project(Library.Name)
        kind "StaticLib"
        language "C++"  
        targetdir "bin/%{prj.name}/%{cfg.buildcfg}"
        files 
        { 
            Library.Files
        }
        includedirs { Library.Includes }
end