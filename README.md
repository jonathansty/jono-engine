# Jono Engine
## Overview
Homebrew graphics/game engine used for prototyping quickly and experimenting with new game engine techniques. 

## Windows: Building
### Requirements
* [Conan package manager](https://conan.io/) installed and on `PATH`
* [VS2019](https://visualstudio.microsoft.com/downloads/)

### Compiling
After cloning the repo the following steps must be executed in order:
1. Run `bootstrap.cmd`
2. Open the generated solution `build\`

The bootstrap script will install both debug and release versions off the required dependencies for this project. It's been setup to work in a multi configuration where changing configs in VS will use the right includes and libraries.