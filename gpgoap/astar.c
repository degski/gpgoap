
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

#include <iso646.h>
#include <intrin.h>
#include <stdbool.h>

#include "../include/gpgoap/vector.h"
#include "../include/gpgoap/khash.h"
#include "../include/gpgoap/astar.h"
#include "../include/gpgoap/goap.h"

#include <limits.h>


typedef struct world_state_map world_state_map_t;

KHASH_MAP_INIT_INT64 ( world_state_map_t, astar_node_t )


void states_init ( khash_t ( world_state_map_t ) ** worlds ) {

    *worlds = kh_init ( world_state_map_t );
}

void states_release ( khash_t ( world_state_map_t ) * world ) {

    kh_destroy ( world_state_map_t, world );
}

khint_t states_size ( khash_t ( world_state_map_t ) * world ) {

    return kh_size ( world );
}

bool states_empty ( khash_t ( world_state_map_t ) * world ) {

    return 0 == kh_size ( world );
}

bool set_state ( khash_t ( world_state_map_t ) * world, world_state_t * state, astar_node_t * value ) {

    bool added = false;

    khiter_t k = kh_get ( world_state_map_t, world, state->hash );

    if ( k == kh_end ( world ) ) {

        int r; // return value

        k = kh_put ( world_state_map_t, world, state->hash, &r );

        added = true;
    }

    kh_value ( world, k ) = *value;

    return added;
}

astar_node_t * insert_state ( khash_t ( world_state_map_t ) * world, world_state_t *state ) {

    khiter_t k = kh_get ( world_state_map_t, world, state->hash );

    if ( k == kh_end ( world ) ) {

        int r; // return value

        k = kh_put ( world_state_map_t, world, state->hash, &r );
    }

    return &kh_value ( world, k );
}

khiter_t get_state ( khash_t ( world_state_map_t ) * world, world_state_t *state ) {

    khiter_t k = kh_get ( world_state_map_t, world, state->hash );

    return kh_end ( world ) == k ? -1 : k;
}

bool has_state ( khash_t ( world_state_map_t ) * world, world_state_t *state ) {

    return kh_end ( world ) != kh_get ( world_state_map_t, world, state->hash );
}

astar_node_t * get_state_value_ptr_by_key ( khash_t ( world_state_map_t ) * world, world_state_t *state ) {

    khiter_t k = kh_get ( world_state_map_t, world, state->hash );

    return kh_end ( world ) == k ? NULL : &kh_value ( world, k );
}

astar_node_t * get_state_value_ptr_by_idx ( khash_t ( world_state_map_t ) * world, int idx ) {

    return &kh_value ( world, idx );
}

bool del_state_by_key ( khash_t ( world_state_map_t ) * world, world_state_t *state ) {

    bool deleted = false;

    khiter_t k = kh_get ( world_state_map_t, world, state->hash );

    if ( k != kh_end ( world ) ) {

        kh_del ( world_state_map_t, world, k );

        deleted = true;
    }

    return deleted;
}

void del_state_by_idx ( khash_t ( world_state_map_t ) * world, int idx ) {

    kh_del ( world_state_map_t, world, idx );
}


// This is our heuristic: estimate for remaining distance is
// the nr of mismatched atoms that matter...

static int calc_h ( world_state_t * fr, world_state_t * to ) {

    bits_t * dif = roaring_bitmap_xor ( fr->values, to->values );

    const int c = roaring_bitmap_and_cardinality ( dif, to->care );

    roaring_bitmap_free ( dif );

    return c;
}



static int lowest_rank ( khash_t ( world_state_map_t ) * world ) {

    khiter_t lowest_idx = -1;
    int lowest_val = INT_MAX;

    for ( khiter_t k = kh_begin ( world ); k != kh_end ( world ); ++k ) {

        if ( kh_exist ( world, k ) ) {

            const int f = kh_value ( world, k ).f;

            if ( f < lowest_val ) {

                lowest_val = f;
                lowest_idx = k;
            }
        }
    }

    return lowest_idx;
}


int astar_plan (

    action_planner_t* ap,
    world_state_t* start,
    world_state_t* goal,
    action_plan_t ** plan ) {

    khash_t ( world_state_map_t ) * opened;
    khash_t ( world_state_map_t ) * closed;

    states_init ( &opened );
    states_init ( &closed );

    // Put start in opened list...

    astar_node_t * n0 = insert_state ( opened, start );

    n0->ws = *start;
    n0->parent_ws = *start;
    n0->g = 0;
    n0->h = calc_h ( start, goal );
    n0->f = n0->g + n0->h;
    n0->action_name = 0;

    const char** action_names = NULL;
    int* action_costs = NULL;
    world_state_t* to = NULL;

    int plan_cost = -1;

    do {

        if ( states_empty ( opened ) ) { LOGI ( "Did not find a path." ); goto exit; }

        // Find the node with lowest rank...

        const int lowest_idx = lowest_rank ( opened );

        // Remove the node with the lowest rank...

        astar_node_t cur = *get_state_value_ptr_by_idx ( opened, lowest_idx );

        del_state_by_idx ( opened, lowest_idx );

        // If it matches the goal, we are done!..

        bits_t *f = roaring_bitmap_and ( cur.ws.values, goal->care ), *t = roaring_bitmap_and ( goal->values, goal->care );

        const bool match = roaring_bitmap_equals ( f, t );

        roaring_bitmap_free ( t );
        roaring_bitmap_free ( f );

        if ( match ) {

            // Reconstruct plan...

            astar_node_t * curnode = &cur;
            action_plan_t * p = *plan;

            while ( curnode and curnode->action_name ) {

                const size_t s = vector_size ( p );

                vector_push_back_default ( p );

                p [ s ].action = curnode->action_name;
                p [ s ].world_state = curnode->ws;

                curnode = get_state_value_ptr_by_key ( closed, &curnode->parent_ws );
            }

            vector_reverse ( p ); // Is this required?..

            *plan = p;
            plan_cost = cur.f;

            goto exit; // Success!!!!
        }

        // Add it to closed...

        set_state ( closed, &cur.ws, &cur );

        // Iterate over neighbours...

        vector_clear ( to );
        vector_clear ( action_costs );
        vector_clear ( action_names ); // Is a non-op the first call..

        const int num_transitions = goap_get_possible_state_transitions ( ap, &cur.ws, &to, &action_names, &action_costs );

        for ( int i = 0; i < num_transitions; ++i ) {

            const int cost = cur.g + action_costs [ i ];

            int idx_o = get_state ( opened, &to [ i ] );
            int idx_c = get_state ( closed, &to [ i ] );

            // if neighbor in OPEN and cost less than g(neighbor):

            if ( idx_o >= 0 && cost < get_state_value_ptr_by_idx ( opened, idx_o )->g ) {

                // Remove neighbor from OPEN, because new path is better...

                del_state_by_idx ( opened, idx_o );
                idx_o = -1;
            }

            // If neighbor in CLOSED and cost less than g(neighbor):

            if ( idx_c >= 0 && cost < get_state_value_ptr_by_idx ( closed, idx_c )->g ) {

                // Remove neighbor from CLOSED...

                del_state_by_idx ( closed, idx_c );
                idx_c = -1;
            }

            // If neighbor not in OPEN and neighbor not in CLOSED:

            if ( idx_c == -1 && idx_o == -1 ) {

                astar_node_t * s = insert_state ( opened, &to [ i ] );

                s->ws = to [ i ];
                s->g = cost;
                s->h = calc_h ( &s->ws, goal );
                s->f = s->g + s->h;
                s->action_name = action_names [ i ];
                s->parent_ws = cur.ws;
            }
        }

    } while ( true );

exit:

    vector_free ( action_names ); // is a non-op the if nullptr is passed...
    vector_free ( action_costs );
    vector_free ( to );

    states_release ( closed );
    states_release ( opened );

    return plan_cost;
}
