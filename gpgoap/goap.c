
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

#if defined( _MSC_VER )
#	define _CRT_SECURE_NO_WARNINGS
#endif

#include "../include/gpgoap/vector.h"
#include "../include/gpgoap/goap.h"
#include "../include/gpgoap/astar.h"

#include <inttypes.h>
#include <iso646.h>
#include <string.h>
#include <stdio.h>


void roaring_bitmap_set ( roaring_bitmap_t * value_, const size_t bit_, const bool bitvalue_ ) {

    if ( bitvalue_ ) roaring_bitmap_add ( value_, bit_ ); else roaring_bitmap_remove ( value_, bit_ );
}

uint32_t * roaring_bitmap_indices ( const roaring_bitmap_t * bits_ ) {

    size_t c = roaring_bitmap_get_cardinality ( bits_ );

    uint32_t * ans = NULL;

    vector_init ( ans, c );

    roaring_bitmap_to_uint32_array ( bits_, ans );

    return ans;
}


// Zobrist Hashing of bit's...

typedef struct zobrist_hash_s { roaring_hash_t m_true, m_false; } zobrist_hash_t;

static zobrist_hash_t * zobrist_hash = NULL;

roaring_hash_t roaring_zobrist_hash_bit ( const uint32_t bit, const bool value ) {

    if ( ( size_t ) bit < vector_size ( zobrist_hash ) ) {

        return value ? zobrist_hash [ bit ].m_true : zobrist_hash [ bit ].m_false;
    }

    else { // Create a global lookup table, adding the missing hashes on the fly...

        for ( uint32_t f = ( uint32_t ) vector_size ( zobrist_hash ); f <= bit; ++f ) {

            const size_t size = vector_size ( zobrist_hash );
            vector_push_back_default ( zobrist_hash );

            ( zobrist_hash + size )->m_true  = goap_mix ( bit                    );
            ( zobrist_hash + size )->m_false = goap_mix ( bit | ROARING_HASH_MSB );
        }

        return value ? zobrist_hash [ bit ].m_true : zobrist_hash [ bit ].m_false;
    }
}

void roaring_hash_print ( const roaring_hash_t hash ) {

    printf ( "0x%" PRIX64 "\n", hash );
}



roaring_hash_t goap_world_state_hash ( const world_state_t * ws_ ) {

    bits_t * const values = ws_->values;

    bits_iterator_t ws_it;
    roaring_init_iterator ( ws_->care, &ws_it );

    roaring_hash_t hash = ROARING_HASH_INIT;

    while ( ws_it.has_value ) {

        hash ^= roaring_zobrist_hash_bit ( ws_it.current_value, roaring_bitmap_contains ( values, ws_it.current_value ) );

        roaring_advance_uint32_iterator ( &ws_it );
    }

    return hash;
}


void goap_bits_print ( const bits_t * bits_, const uint32_t length_ ) {

    uint32_t * ans = roaring_bitmap_indices ( bits_ );

    char * bits = NULL;

    const uint32_t l = roaring_bitmap_maximum ( bits_ ) + 1;

    vector_init ( bits, ( l > length_ ? l : length_ ) );

    for ( int bit = 0, end = vector_size ( ans ); bit < end; ++bit ) {

        bits [ ans [ bit ] ] = 1;
    }

    for ( int bit = vector_size ( bits ) - 1; bit >= 0; --bit ) {

        putchar ( ( int ) ( 1 == bits [ bit ] ) + ( int ) 48 );
    }

    printf ( " %8llu %8lu\n", vector_size ( ans ), ( unsigned long ) roaring_bitmap_maximum ( bits_ ) );

    vector_free ( ans );
    vector_free ( bits );
}

void goap_world_state_print ( world_state_t * ws ) {

    goap_bits_print ( ws->values, 32 );
    goap_bits_print ( ws->care, 32 );
    roaring_hash_print ( ws->hash );
}


static int goap_atom_name_idx ( action_planner_t* ap, const char* atom_name ) {

    int idx = -1;

    if ( not ( trie_get ( &ap->atoms, atom_name, &idx ) ) ) {

        idx = ap->atoms.size;

        trie_set ( &ap->atoms, atom_name, idx );
        vector_push_back ( ap->atom_names, atom_name );
    }

    return idx;
}

static int goap_action_name_idx ( action_planner_t* ap, const char* action_name ) {

    int idx = -1;

    if ( not ( trie_get ( &ap->actions, action_name, &idx ) ) ) {

        idx = ap->actions.size;

        trie_set ( &ap->actions, action_name, idx );

        vector_push_back ( ap->act_names, action_name );

        vector_emplace_back ( ap->act_ante, goap_world_state_construct );
        vector_emplace_back ( ap->act_post, goap_world_state_construct );

        vector_push_back ( ap->act_costs, 1 ); // Default cost is 1.
    }

    return idx;
}



void goap_action_planner_construct ( action_planner_t* ap ) {

    trie_init ( &ap->atoms );
    ap->atom_names = NULL;

    trie_init ( &ap->actions );
    ap->act_names = NULL;

    ap->act_ante = NULL;
    ap->act_post = NULL;

    ap->act_costs = NULL;
}

void goap_action_planner_destroy ( action_planner_t* ap ) {

    vector_free ( ap->act_costs );

    goap_world_state_vector_destroy ( ap->act_post );
    goap_world_state_vector_destroy ( ap->act_ante );

    vector_free ( ap->act_names );
    trie_release ( &ap->actions );

    vector_free ( ap->atom_names );
    trie_release ( &ap->atoms );
}


world_state_t * goap_world_state ( ) { // A factory...

    world_state_t * ws = ( world_state_t * ) malloc ( sizeof ( world_state_t ) );

    goap_world_state_construct ( ws );

    return ws;
}

void goap_world_state_construct ( world_state_t * ws ) {

    ws->values = roaring_bitmap_create ( );
    ws->care = roaring_bitmap_create ( );
    ws->hash = ROARING_HASH_INIT;
}

void goap_world_state_destroy ( world_state_t * ws ) {

    roaring_bitmap_free ( ws->care );
    roaring_bitmap_free ( ws->values );
}

static bool goap_world_state_vector_run_optimize ( world_state_t * p ) {

    bool opti = false;

    for ( size_t i = 0ULL, e = vector_size ( p ); i < e; ++i ) {

        opti |= roaring_bitmap_run_optimize ( p [ i ].values );
        opti |= roaring_bitmap_run_optimize ( p [ i ].care );
    }

    return opti;
}

bool goap_action_planner_run_optimize ( action_planner_t * ap ) {

    return goap_world_state_vector_run_optimize ( ap->act_ante ) | goap_world_state_vector_run_optimize ( ap->act_post );
}


// Release a vector of world_state_t's...

void goap_world_state_vector_destroy ( world_state_t * p ) {

    int i = ( int ) vector_size ( p );

    do {

        --i;

        roaring_bitmap_free ( p [ i ].values );
        roaring_bitmap_free ( p [ i ].care );

    } while ( i );

    vector_free ( p );
}


static bool goap_world_state_set_idx ( world_state_t * ws, const int idx, bool value ) {

    if ( idx == -1 ) return false;

    if ( not ( roaring_bitmap_contains ( ws->care, idx ) ) ) { // We are setting a new value...

        roaring_bitmap_set ( ws->values, idx, value );
        roaring_bitmap_add ( ws->care, idx );

        ws->hash ^= roaring_zobrist_hash_bit ( idx, value );
    }

    else { // We are updating an existing value...

        const bool current_value = roaring_bitmap_contains ( ws->values, idx );

        if ( value != current_value ) { // A transition...

            // true -> false or false -> true...

            roaring_bitmap_set ( ws->values, idx, value );

            ws->hash ^= roaring_zobrist_hash_bit ( idx, current_value );
            ws->hash ^= roaring_zobrist_hash_bit ( idx, value );
        }
    }

    assert ( ws->hash == goap_world_state_hash ( ws ) );

    return true;
}


bool goap_world_state_set ( action_planner_t * ap, world_state_t * ws, const char * atom_name, bool value ) {

    return goap_world_state_set_idx ( ws, goap_atom_name_idx ( ap, atom_name ), value );
}


bool goap_set_ante ( action_planner_t * ap, const char * action_name, const char * atom_name, bool value ) {

	const int actidx = goap_action_name_idx ( ap, action_name );
	if ( actidx == -1 ) return false;

	goap_world_state_set ( ap, ap->act_ante + actidx, atom_name, value );

	return true;
}


bool goap_set_post ( action_planner_t * ap, const char * action_name, const char * atom_name, bool value ) {

	const int actidx = goap_action_name_idx ( ap, action_name );
	if ( actidx == -1 ) return false;

	goap_world_state_set ( ap, ap->act_post + actidx, atom_name, value );

	return true;
}


bool goap_set_cost ( action_planner_t * ap, const char * action_name, int cost ) {

	const int actidx = goap_action_name_idx ( ap, action_name );
	if ( actidx == -1 ) return false;

	ap->act_costs [ actidx ] = cost;

	return true;
}


void goap_world_state_description ( const action_planner_t * ap, const world_state_t * ws, char * buf, int sz ) {

    int added = 0;

    bits_iterator_t goap_it;

    roaring_init_iterator ( ws->care, &goap_it );

    while ( goap_it.has_value ) {

        const char* val = ap->atom_names [ goap_it.current_value ];
        char upval [ 128 ];
        size_t j;

        for ( j = 0; j < strlen ( val ); ++j ) {

            upval [ j ] = ( val [ j ] - 32 );
        }

        upval [ j++ ] = 0;

        added = snprintf ( buf, sz, "%s,", roaring_bitmap_contains ( ws->values, goap_it.current_value ) ? upval : val );
        buf += added; sz -= added;

        roaring_advance_uint32_iterator ( &goap_it );
    }
}

void goap_description ( const action_planner_t * ap, char * buf, int sz ) {

    int added = 0;

    bits_iterator_t goap_it;

    for ( int a = 0; a < ap->actions.size; ++a ) {

        added = snprintf ( buf, sz, "%s:\n", ap->act_names [ a ] );
        sz -= added; buf += added;

        world_state_t pre = ap->act_ante [ a ];
        world_state_t pst = ap->act_post [ a ];

        roaring_init_iterator ( pre.care, &goap_it );

        while ( goap_it.has_value ) {

            added = snprintf ( buf, sz, "  %s==%d\n", ap->atom_names [ goap_it.current_value ], roaring_bitmap_contains ( pre.values, goap_it.current_value ) );
            sz -= added; buf += added;

            roaring_advance_uint32_iterator ( &goap_it );
        }

        roaring_init_iterator ( pst.care, &goap_it );

        while ( goap_it.has_value ) {

            added = snprintf ( buf, sz, "  %s:=%d\n", ap->atom_names [ goap_it.current_value ], roaring_bitmap_contains ( pst.values, goap_it.current_value ) );
            sz -= added; buf += added;

            roaring_advance_uint32_iterator ( &goap_it );
        }
    }
}


static void set_ws ( world_state_t * t, world_state_t * pst, world_state_t * f ) {

    // Set value...

    t->values = roaring_bitmap_andnot ( f->values, pst->care );
    bits_t * const tmp = roaring_bitmap_and ( pst->values, pst->care );
    roaring_bitmap_or_inplace ( t->values, tmp );
    roaring_bitmap_free ( tmp );

    // Set care...

    t->care = roaring_bitmap_or ( f->care, pst->care );

    // Set hash...

    bits_t * const changed_bits = roaring_bitmap_xor ( f->values, t->values );

    if ( roaring_bitmap_is_empty ( changed_bits ) ) {

        t->hash = f->hash;
    }

    else {

        bits_hash_t hash = f->hash;

        bits_iterator_t goap_it;
        roaring_init_iterator ( changed_bits, &goap_it );

        while ( goap_it.has_value ) {

            const bool f_value = roaring_bitmap_contains ( f->values, goap_it.current_value );
            const bool t_value = roaring_bitmap_contains ( t->values, goap_it.current_value );

            if ( f_value != t_value ) {

                // // A transition: true -> false or false -> true...

                hash ^= roaring_zobrist_hash_bit ( goap_it.current_value, f_value );
                hash ^= roaring_zobrist_hash_bit ( goap_it.current_value, t_value );
            }

            roaring_advance_uint32_iterator ( &goap_it );
        }

        t->hash = hash;
    }

    assert ( t->hash == goap_world_state_hash ( t ) );

    roaring_bitmap_free ( changed_bits );
}


int goap_get_possible_state_transitions ( action_planner_t * ap, world_state_t * fr, world_state_t ** to_, const char *** action_names, int ** action_costs ) {

    const char ** a = *action_names;
    int * c = *action_costs;
    world_state_t * to = *to_;

    for ( int i = 0; i < ap->actions.size; ++i ) {

        // See if precondition is met...

        bits_t * const care = ( ap->act_ante + i )->care;
        bits_t * const x = roaring_bitmap_and ( ( ap->act_ante + i )->values, care );
        bits_t * const y = roaring_bitmap_and ( fr->values, care );

        const bool met = roaring_bitmap_equals ( x, y );

        roaring_bitmap_free ( y );
        roaring_bitmap_free ( x );

        if ( met ) {

            vector_push_back ( a, ap->act_names [ i ] );
            vector_push_back ( c, ap->act_costs [ i ] );

            // Do the action: create a new world_state_t...

            const size_t size = vector_size ( to );
            vector_push_back_default ( to );

            set_ws ( to + size, ap->act_post + i, fr );
        }
    }

    *action_names = a;
    *action_costs = c;
    *to_ = to;

    return vector_size ( a );
}
