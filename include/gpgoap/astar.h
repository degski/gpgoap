
/*
    Copyright 2012 Abraham T. Stolk

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the
    License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
    implied. See the License for the specific language governing
    permissions and limitations under the License.
*/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifndef LOGI
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif
#define LOGI(...) { printf( __VA_ARGS__ ); printf("\n"); }
#define LOGW(...) { printf( "WRN " __VA_ARGS__ ); printf("\n"); }
#define LOGE(...) { printf( "ERR " __VA_ARGS__ ); printf("\n"); }
#endif

#include "goap.h"

struct astar_node;

// A node in our network of world states...

typedef struct astar_node_s {

	world_state_t ws;		            // The state of the world at this node.
	int g;				                // The cost so far.
	int h;				                // The heuristic for remaining cost (don't overestimate!)
	int f;				                // g+h combined.
	const char* action_name;		    // How did we get to this node?
	world_state_t parent_ws;		    // Where did we come from?

} astar_node_t;

typedef struct action_plan_s {

    const char* action;
    world_state_t world_state;

} action_plan_t;


// Make a plan of actions that will reach desired
// world state. Returns total cost of the plan...

extern int astar_plan (

    action_planner_t* ap, 		    // the goap action planner that holds atoms and action repertoire
    world_state_t* start, 		    // the current world state
    world_state_t* goal, 		    // the desired world state
    action_plan_t ** plan           // for returning all actions that make up plan
);

#ifdef __cplusplus
}
#endif
