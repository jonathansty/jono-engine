DependenciesRoot = "External/"

LibHlslpp = {
    Name = "hlslpp",
    Path = DependenciesRoot .. "hlslpp/",
    Includes = DependenciesRoot .. "hlslpp/include",
}

LibBox2D = {
    Name = "box2d",
    Path = DependenciesRoot .. "box2d/",
    Includes = DependenciesRoot .. "box2d/include",
}
LibFmt = {
    Name = "fmt",
    Path = DependenciesRoot .. "fmt/",
    Includes = DependenciesRoot .. "fmt/include",
}
LibEnkiTS = {
    Name = "EnkiTS",
    Path = DependenciesRoot .. "EnkiTS/",
    Includes = DependenciesRoot .. "EnkiTS/src",
    Files = {
        DependenciesRoot .. "EnkiTS/src/**.h",
        DependenciesRoot .. "EnkiTS/src/**.cpp"
    }
}

function LinkLibrary(Library)
    includedirs { Library.Includes }
    libdirs { Library.LibDirs }
    links { Library.Name }
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