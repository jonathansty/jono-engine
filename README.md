# Jono Engine
![Screenshot of the engine running as application](https://github.com/jonathansty/jono-engine/blob/main/Documentation/Images/readme_engine.png)

Hobby engine mainly used for experimentation and learning purposes. This code is not intended to be used as production ready code but mostly as a playground and test area for learning about graphics, game engines and video game programming in general.

# Overview
The engine depends on the following libraries:
* [hlslpp](https://github.com/redorav/hlslpp): Flexible math library that mimicks hlsl behaviours for various types.
* assimp: model loading middleware to support different formats 
* imgui: immediate mode debugging and UI
* Optick: Performance Profiling
* rttr: Runtime type reflection
# Building
## Requirements
* Windows 10
* [Python](https://www.python.org/) (for conan)
* [Conan package manager](https://conan.io/) installed and available on `PATH` environment variable
* [VS2022](https://visualstudio.microsoft.com/downloads/)

## Compiling
After cloning the repo the following steps must be executed in order:
1. Run `bootstrap.cmd`
2. Run Cmake initial setup on rttr (choose vs2022) 
3. Open the generated solution `build\`

The bootstrap script will install both debug and release versions off the required dependencies for this project. It's been setup to work in a multi configuration where changing configs in VS will use the right includes and libraries.
