-- premake5.lua
workspace "DAE Game Engine"
   configurations { "Debug", "Release" }

project "Game"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"

   includedirs{
       "Box2D/"
   }

   libdirs{
       "Box2D/Build/bin/x86_64/Debug/"
   }

   links{
       "box2D"
   }

   files { "**.h", "**.cpp" }

   pchheader("src/stdafx.h")
   pchsource("src/stdafx.cpp");

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"