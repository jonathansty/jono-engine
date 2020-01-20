#pragma once

#define MAKE_VERSION(major, minor, patch)  (major << 24 | minor << 12 | patch)

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 2 

#define GAME_VERSION MAKE_VERSION(VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH)
