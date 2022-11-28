# Jono Engine [![build-action](https://github.com/jonathansty/jono-engine/actions/workflows/main.yml/badge.svg)](https://github.com/jonathansty/jono-engine/actions/workflows/main.yml)

![Screenshot of the engine running as application](https://github.com/jonathansty/jono-engine/blob/main/Documentation/Images/readme_engine.png?raw=true)

Hobby engine mainly used for experimentation and learning purposes. This code is not intended to be used as production ready code but mostly as a playground and test area for learning about graphics, game engines and video game programming in general.

Originally this engine was a 2D only engine developed by Kevin Hoefman as part of the programming course of [Digital Arts and Entertainment](https://www.digitalartsandentertainment.be/). After my studies I took this engine and made some significant changes to it to migrate this to use D3D11 but still support the 2D portion of the drawing for quick prototyping of algorithms (For example pathfinding).

Some notable changes to the engine are:
* D2D backend migrated to D3D11 + "D2D on D3D11"
* Added ImGui debug tooling and framework to the engine to allow games to populate and create new debug windows
* Implemented `rttr` library to allow runtime type reflection for the engine. (Initially I implemented a similar version myself but ended up using an off the shelf engine to free up time)
* Graphics:
	* Forward rendering pass with PBR lighting
	* Cascade Shadow mapping
	* Simple gpu query based profiling for main rendering passes
* Integrated OpTick to allow CPU profiling and visualizing this with a flamegraph tool.
* Added thread-safe logging framework:
	* Output log window in imgui supports colouring, filtering and 'auto scroll'.
* ... 

# Overview
The engine depends on the following libraries:
* [hlslpp](https://github.com/redorav/hlslpp): Flexible math library that mimicks hlsl behaviours for various types.
* [assimp](https://github.com/assimp/assimp): model loading middleware to support different formats 
* [imgui](https://github.com/ocornut/imgui): immediate mode debugging and UI
* [Optick](https://github.com/bombomby/optick): Performance Profiling
* [rttr](https://github.com/rttrorg/rttr): Runtime type reflection
* [Sharpmake](https://github.com/ubisoft/Sharpmake): to generate the projects 
# Building
## Requirements
* Windows 10
* .NET framework 5.0.0 for Sharpmake
* [CMake](https://cmake.org/download/)
* [Python](https://www.python.org/) (for conan)
* [Conan package manager](https://conan.io/) installed and available on `PATH` environment variable
* [VS2022](https://visualstudio.microsoft.com/downloads/)

## Compiling
After cloning the repo the following steps must be executed in order:
1. Run `bootstrap.cmd`
2. Run Cmake initial setup on rttr (choose vs2022) 
3. Open the generated solution `build\`

The bootstrap script will install both debug and release versions off the required dependencies for this project. It's been setup to work in a multi configuration where changing configs in VS will use the right includes and libraries.
