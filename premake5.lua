-- premake5.lua
workspace "DAE Game Engine"
   configurations { "Debug", "Release" }

project "Game"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   includedirs{
       "Box2D/",
       "src"
   }

   libdirs{
       "Box2D/Build/bin/x86_64/Debug/"
   }

   links{
       "box2D",
       "SDL2"
   }

   files { "src/**.h", "src/**.cpp" }
   removefiles{
       "src/EngineFiles/SVGParser.cpp",
       "src/EngineFiles/SVGParser.h",
   }

   pchheader("stdafx.h")
   pchsource("stdafx.cpp");

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"