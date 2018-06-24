
#pragma once

#include "./gpgoap/goap.h"
#include "./gpgoap/astar.h"

#ifdef NDEBUG
#pragma comment ( lib, "gpgoap-s.lib" )
#else
#pragma comment ( lib, "gpgoap-s-d.lib" )
#endif
