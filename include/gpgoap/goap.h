
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

#ifndef GOAP_BUILD
#ifdef NDEBUG
#pragma comment ( lib, "gpgoap-s.lib" )
#else
#pragma comment ( lib, "gpgoap-s-d.lib" )
#endif
#endif

#if _MSC_VER>=1900
#  define STDC99
#endif

#include "roaring.h"
#include "khash.h"
#include "trie.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"
{
#include <cstdbool>
#include <cstdint>
#else
#include <stdbool.h>
#include <stdint.h>
#endif


typedef khint64_t roaring_hash_t;

#define ROARING_HASH_INIT 0xCDCDCDCDCDCDCDCD
#define ROARING_HASH_MSB  0x8000000000000000

typedef roaring_bitmap_t bits_t;
typedef roaring_uint32_iterator_t bits_iterator_t;
typedef roaring_hash_t bits_hash_t;

// Describes the world state by listing values (t/f)
// for all known atoms...

typedef struct world_state_s {

	bits_t * values;	                    // Values for atoms.
    bits_t * care;	                        // Mask for atoms that do matter.
    bits_hash_t hash;

} world_state_t;


// Action planner that keeps track of world state
// atoms and its action repertoire...

typedef struct action_planner_s {

    struct trie ( int ) atoms;
    const char ** atom_names;                // Names associated with all world state atoms.

    struct trie ( int ) actions;
    const char ** act_names;                 // Names of all actions in repertoire.

    world_state_t * act_ante;                 // Preconditions for all actions.
    world_state_t * act_post;	                // Postconditions for all actions (action effects).

    int * act_costs;	                    // Cost for all actions.

} action_planner_t;


void roaring_bitmap_set ( roaring_bitmap_t * value_, const size_t bit_, const bool bitvalue_ );
uint32_t * roaring_bitmap_indices ( const roaring_bitmap_t * bf_ );

roaring_hash_t roaring_zobrist_hash_bit ( const uint32_t bit, const bool value );

void roaring_hash_print ( roaring_hash_t hash );

roaring_hash_t goap_world_state_hash ( const world_state_t * ws_ );
bool goap_action_planner_run_optimize ( action_planner_t * ap );


static inline uint64_t goap_mix ( const uint64_t y ) {

    uint64_t x = ( ( y >> 32 ) ^ y ) * 0xD6E8FEB86659FD93;
    x = ( ( x >> 32 ) ^ x ) * 0xD6E8FEB86659FD93;
    x = ( ( x >> 32 ) ^ x ) * 0xD6E8FEB86659FD93;
    return ( ( x >> 32 ) ^ x );
}

static inline uint64_t goap_revert_mix ( const uint64_t y ) {

    uint64_t x = ( ( y >> 32 ) ^ y ) * 0xCFEE444D8B59A89B;
    x = ( ( x >> 32 ) ^ x ) * 0xCFEE444D8B59A89B;
    x = ( ( x >> 32 ) ^ x ) * 0xCFEE444D8B59A89B;
    return ( ( x >> 32 ) ^ x );
}


static inline void print_bits_u32 ( const uint32_t  n ) {

    uint32_t i = 0x80000000;

    while ( i ) {

        putchar ( ( int ) ( ( n & i ) > 0 ) + ( int ) ( 48 ) );

        i >>= 1;
    }

    putchar ( '\n' );
}

static inline void print_bits_u64 ( const uint64_t  n ) {

    uint64_t i = 0x8000000000000000;

    while ( i ) {

        putchar ( ( int ) ( ( n & i ) > 0 ) + ( int ) ( 48 ) );

        i >>= 1;
    }

    putchar ( '\n' );
}

void goap_bits_print ( const bits_t * bf_, const uint32_t length_ );


extern void goap_action_planner_construct ( action_planner_t * ap );
extern void goap_action_planner_destroy ( action_planner_t * ap );

extern world_state_t * goap_world_state ( );
extern void goap_world_state_construct ( world_state_t * ws );
extern void goap_world_state_destroy ( world_state_t * ws );

extern void goap_world_state_print ( world_state_t * ws );

extern void goap_world_state_vector_destroy ( world_state_t * p ); // Release a vector of states...

// Set an atom of world_state to specified value...
extern bool goap_world_state_set ( action_planner_t * ap, world_state_t * ws, const char * atom_name, bool value );

// Add a precondition for named action...

extern bool goap_set_ante ( action_planner_t * ap, const char * action_name, const char * atom_name, bool value );

// Add a postcondition for named action...

extern bool goap_set_post ( action_planner_t * ap, const char * action_name, const char * atom_name, bool value );

// Set the cost for named action...

extern bool goap_set_cost ( action_planner_t * ap, const char * action_name, int cost );

// Describe the action planner by listing all actions
// with pre and post conditions. For debugging purpose...

extern void goap_description ( const action_planner_t * ap, char * buf, int sz );

// Describe the world_state by listing atoms that matter,
// in lowercase for false-valued, and uppercase for true-
// valued atoms...

extern void goap_world_state_description ( const action_planner_t * ap, const world_state_t * ws, char * buf, int sz );

// Given the specified 'from' state, list all possible 'to'
// states along with the action required, and the action
// cost. For internal use...

extern int  goap_get_possible_state_transitions ( action_planner_t * ap, world_state_t * fr, world_state_t ** to, const char *** action_names, int ** action_costs );

#ifdef __cplusplus
}
#endif
