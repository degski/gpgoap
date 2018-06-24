
#pragma once

#include "tritset.h"

#include "../include/gpgoap/trie.h"
#include "../include/gpgoap/vector.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>



typedef struct world_state_ante_s {

    tritset_t values;

} ws_ante_t;

typedef struct world_state_post_s {

    tritset_t values;
    bool enabled;

} ws_post_t;

typedef struct action_planner_s {

    struct trie ( int32_t ) atoms;
    char ** atom_names;                 // Names associated with all world state atoms.

    struct trie ( int32_t ) actions;
    char ** act_names;                  // Names of all actions in repertoire.

    ws_ante_t * act_ante;               // Preconditions for all actions.
    ws_post_t * act_post;               // Postconditions for all actions (action effects).

    int32_t * act_cost;	                // Cost for all actions.

} ap_t;

typedef struct idx_s {

    int32_t value;
    bool added;

} idx_t;

typedef struct action_plan_s {

    const char** action_names;
    int32_t cost;
    ws_ante_t state;

} action_plan_t;


void goap_action_planner_construct ( ap_t* ap );
void goap_action_planner_destroy ( ap_t* ap );

void goap_ws_ante_vector_destroy ( ws_ante_t * p );
void goap_ws_post_vector_destroy ( ws_post_t * p );

void goap_ws_ante_construct ( ws_ante_t * ws );
void goap_ws_post_construct ( ws_post_t * ws );

void goap_ws_ante_destroy ( ws_ante_t * ws );
void goap_ws_post_destroy ( ws_post_t * ws );

ws_ante_t * goap_ws_ante ( void ); // A factory...
ws_post_t * goap_ws_post ( void ); // A factory...

void goap_ws_ante_print ( ws_ante_t * ws );
void goap_ws_post_print ( ws_post_t * ws );

void goap_ws_set_ante ( ap_t * ap, ws_ante_t * ws, char * atom_name, bool value );

void goap_set_ante ( ap_t * ap, char * action_name, char * atom_name, bool value );
void goap_set_post ( ap_t * ap, char * action_name, char * atom_name, bool value );
void goap_set_cost ( ap_t * ap, char * action_name, int32_t cost );
void goap_set_action ( ap_t * ap, char * action_name, bool enabled );

void goap_ws_ante_description ( ap_t * ap, ws_ante_t * ws, char * buf, int32_t sz );
void goap_description ( ap_t * ap, char * buf, int32_t sz );

action_plan_t goap_plan (

    ap_t * ap, 		            // the goap action planner that holds atoms and action repertoire
    ws_ante_t * start, 		    // the current world state
    ws_ante_t * goal );         // for returning all actions that make up plan
