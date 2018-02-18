#pragma once

#define VERSION "0.9.05"

//#define SOUND_SYSTEM_DEBUG
//#define TRANSITIONS_DEBUG
//#define TECH_TREE_DEBUG
//#define FACTORY_DEBUG
//#define GAME_ENGINE_DEBUG
//#define COMBAT_DEBUG
//#define VICTORY_DEBUG
//#define AI_DEBUG
//#define MANAGER_DEBUG
//#define PARSER_DEBUG
//#define BUG_DEBUG
//#define PATHFINDING_DEBUG
//#define FLOWFIELDS_DEBUG

#define ZOOMLEVEL_ENABLE
#define SHADER_ENABLE

#define PATHFINDING_FLOWFIELD
#define PATHFINDING_FLOWFIELD_DYNAMIC

#define MAX_SOUNDS 32

#ifdef WITHGPERFTOOLS
#include <gperftools/profiler.h> 
#endif