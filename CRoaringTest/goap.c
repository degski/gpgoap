
#include "goap.h"

#include "../include/gpgoap/vector.h"

#include <inttypes.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


void goap_action_planner_construct ( ap_t * ap ) {

    trie_init ( &ap->atoms );
    vector_allocate ( ap->atom_names );

    trie_init ( &ap->actions );
    vector_allocate ( ap->act_names );

    vector_allocate ( ap->act_ante );
    vector_allocate ( ap->act_post );
    vector_allocate ( ap->act_cost );
}

void goap_ws_ante_construct ( ws_ante_t * ws ) {

    vector_allocate ( ws->values );
}

void goap_ws_ante_destroy ( ws_ante_t * ws ) {

    vector_free ( ws->values );
}

void goap_ws_ante_vector_destroy ( ws_ante_t * p ) {

    int32_t i = ( int32_t ) vector_size ( p );

    do {

        --i;

        vector_free ( p [ i ].values );

    } while ( i );

    vector_free ( p );
}

void goap_ws_post_construct ( ws_post_t * ws ) {

    vector_allocate ( ws->values );
    ws->enabled = true;
}

void goap_ws_post_destroy ( ws_post_t * ws ) {

    vector_free ( ws->values );
}

void goap_ws_post_vector_destroy ( ws_post_t * p ) {

    int32_t i = ( int32_t ) vector_size ( p );

    do {

        --i;

        goap_ws_post_destroy ( p + i );

    } while ( i );

    vector_free ( p );
}

void goap_action_planner_destroy ( ap_t* ap ) {

    vector_free ( ap->act_cost );

    goap_ws_post_vector_destroy ( ap->act_post );
    goap_ws_ante_vector_destroy ( ap->act_ante );

    vector_free ( ap->act_names );
    trie_release ( &ap->actions );

    vector_free ( ap->atom_names );
    trie_release ( &ap->atoms );
}


ws_ante_t * goap_ws_ante ( void ) { // A factory...

    ws_ante_t * ws = ( ws_ante_t * ) malloc ( sizeof ( ws_ante_t ) );

    goap_ws_ante_construct ( ws );

    return ws;
}

ws_post_t * goap_ws_post ( void ) { // A factory...

    ws_post_t * ws = ( ws_post_t * ) malloc ( sizeof ( ws_post_t ) );

    goap_ws_post_construct ( ws );

    return ws;
}


void goap_ws_ante_print ( ws_ante_t * ws ) {

    tritset_print ( ws->values );
}

void goap_ws_post_print ( ws_post_t * ws ) {

    tritset_print ( ws->values );
}


static idx_t goap_atom_name_idx ( ap_t * ap, char * atom_name ) {

    idx_t idx;

    idx.added = false;

    if ( not ( trie_get ( &ap->atoms, atom_name, &idx.value ) ) ) {

        idx.value = ( int32_t ) ap->atoms.size;
        idx.added = true;

        trie_set ( &ap->atoms, atom_name, idx.value );
        vector_push_back ( ap->atom_names, atom_name );
    }

    return idx;
}


static idx_t goap_action_name_idx ( ap_t * ap, char * action_name ) {

    idx_t idx;

    idx.added = false;

    if ( not ( trie_get ( &ap->actions, action_name, &idx.value ) ) ) {

        idx.value = ( int32_t ) ap->actions.size;
        idx.added = true;

        trie_set ( &ap->actions, action_name, idx.value );

        vector_push_back ( ap->act_names, action_name );

        vector_emplace_back ( ap->act_ante, goap_ws_ante_construct );
        vector_emplace_back ( ap->act_post, goap_ws_post_construct );

        vector_push_back ( ap->act_cost, 1 ); // Default cost is 1.
    }

    return idx;
}


static void goap_ws_set_idx_ante ( ws_ante_t * ws, int32_t idx, bool value ) {

    ws->values = tritset_write ( ws->values, idx, value );
}

static void goap_ws_set_idx_post ( ws_post_t * ws, int32_t idx, bool value ) {

    ws->values  = tritset_write ( ws->values, idx, value );
}


void goap_ws_set_ante ( ap_t * ap, ws_ante_t * ws, char * atom_name, bool value ) {

    goap_ws_set_idx_ante ( ws, goap_atom_name_idx ( ap, atom_name ).value, value );
}

static void goap_ws_set_post ( ap_t * ap, ws_post_t * ws, char * atom_name, bool value ) {

    goap_ws_set_idx_post ( ws, goap_atom_name_idx ( ap, atom_name ).value, value );
}


void goap_set_ante ( ap_t * ap, char * action_name, char * atom_name, bool value ) {

    idx_t i = goap_action_name_idx ( ap, action_name );

	goap_ws_set_ante ( ap, ap->act_ante + i.value, atom_name, value );
}

void goap_set_post ( ap_t * ap, char * action_name, char * atom_name, bool value ) {

    idx_t i = goap_action_name_idx ( ap, action_name );

	goap_ws_set_post ( ap, ap->act_post + i.value, atom_name, value );
}


void goap_set_cost ( ap_t * ap, char * action_name, int32_t cost ) {

    idx_t i = goap_action_name_idx ( ap, action_name );

	ap->act_cost [ i.value ] = cost;
}


void goap_set_action ( ap_t * ap, char * action_name, bool enabled ) {

    idx_t i = goap_action_name_idx ( ap, action_name );

    ap->act_post [ i.value ].enabled = enabled;
}


void goap_ws_ante_description ( ap_t * ap, ws_ante_t * ws, char * buf, int32_t sz ) {

    int32_t added = 0;
    tritset_t t = ws->values;

    char upval [ 1024 ];

    for ( int32_t i = 0, li = ( int32_t ) vector_size ( t ); i < li; ++i ) {

        char* val = ap->atom_names [ sm_magnitude ( t [ i ] ) ];
        size_t l;

        for ( l = 0; l < strlen ( val ); ++l ) {

            upval [ l ] = ( val [ l ] - 32 );
        }

        upval [ l++ ] = 0;

        added = snprintf ( buf, ( size_t ) sz, "%s,", sm_negative ( t [ i ] ) ? val : upval );
        buf += added; sz -= added;
    }
}


void goap_description ( ap_t * ap, char * buf, int32_t sz ) {

    int32_t added = 0;

    for ( int32_t a = 0, la = ( int32_t ) ap->actions.size; a < la; ++a ) {

        added = snprintf ( buf, ( size_t ) sz, "%s:\n", ap->act_names [ a ] );
        sz -= added; buf += added;

        tritset_t va = ap->act_ante [ a ].values;

        for ( int32_t i = 0, li = ( int32_t ) vector_size ( va ); i < li; ++i ) {

            smu_t v = tritset_read ( va, i );

            added = snprintf ( buf, ( size_t ) sz, "  %s==%u\n", ap->atom_names [ sm_magnitude ( v ) ], sm_negative ( v ) ? false : true );
            sz -= added; buf += added;
        }

        tritset_t vp = ap->act_post [ a ].values;

        for ( int32_t i = 0, li = ( int32_t ) vector_size ( vp ); i < li; ++i ) {

            smu_t v = tritset_read ( vp, i );

            added = snprintf ( buf, ( size_t ) sz, "  %s:=%u\n", ap->atom_names [ sm_magnitude ( v ) ], sm_negative ( v ) ? false : true );
            sz -= added; buf += added;
        }
    }
}


/*

void goap_description ( ap_t * ap, char * buf, int32_t sz ) {

    int32_t added = 0;

    for ( int32_t a = 0, la = ( int32_t ) ap->actions.size; a < la; ++a ) {

        added = snprintf ( buf, ( size_t ) sz, "%s:\n", ap->act_names [ a ] );
        sz -= added; buf += added;

        ws_ante_t sa = ap->act_ante [ a ];
        int32_t v_size = sizeof ( *sa.values ) << 2, s_size = ( int32_t ) vector_size ( sa.values );

        for ( int32_t i = 0, k = 0; i < s_size; ++i ) {

            for ( int32_t j = 0; j < v_size; ++j, ++k ) {

                smu_t v = trit32_read ( sa.values [ i ], j );

                if ( v & TRIT_HB ) {

                    added = snprintf ( buf, ( size_t ) sz, "  %s==%u\n", ap->atom_names [ k ], v & TRIT_LB );
                    sz -= added; buf += added;
                }
            }
        }

        ws_post_t sp = ap->act_post [ a ];
        v_size = sizeof ( *sp.values ) << 2; s_size = ( int32_t ) vector_size ( sp.values );

        for ( int32_t i = 0, k = 0; i < s_size; ++i ) {

            for ( int32_t j = 0; j < v_size; ++j, ++k ) {

                smu_t v = trit32_read ( sp.values [ i ], j );

                if ( v & TRIT_HB ) {

                    added = snprintf ( buf, ( size_t ) sz, "  %s:=%u\n", ap->atom_names [ k ], v & TRIT_LB );
                    sz -= added; buf += added;
                }
            }
        }
    }
}

*/


typedef struct transition_s transition_t;

struct transition_s { // total 32 bytes

    char * name;
    tritset_t values;
    transition_t * parent;

    int32_t cost;

    char padding [ 4 ];
};

typedef transition_t * transition_ptr_t;
typedef transition_t * transition_vec_t;



static transition_vec_t goap_get_possible_actions ( ap_t * ap, transition_ptr_t parent ) {

    transition_vec_t children;

    vector_allocate ( children );

    for ( int32_t i = 0, li = ( int32_t ) ap->actions.size; i < li; ++i ) {

        ws_post_t * post = ap->act_post + i;

        if ( post->enabled and tritset_sub ( parent->values, post->values ) ) {                                                                         //or trit32_set_sub ( post->values, parent->values ) ) {

            //*

            printf ( "V%i", i );
            tritset_print ( parent->values );
            printf ( "%s\n", ap->act_names [ i ] );
            printf ( "P " );
            tritset_print ( post->values );
            printf ( "A " );
            tritset_print ( ap->act_ante [ i ].values );
            printf ( "G " );
            tritset_print ( tritset_man ( parent->values, ap->act_ante [ i ].values, post->values ) );
            putchar ( '\n' );

            //*/

            size_t size = vector_size ( children );
            vector_push_back_default ( children );

            transition_ptr_t child = children + size;

            child->name     = ap->act_names [ i ];
            child->values   = tritset_man ( parent->values, ap->act_ante [ i ].values, post->values );
            child->parent   = parent;
            child->cost     = parent->cost + ap->act_cost [ i ];
        }
    }

    return children;
}



typedef struct walk_s {

    transition_ptr_t list, best;
    int32_t cost;

} walk_t;



static void goap_walk ( ap_t * ap, walk_t * walk, ws_ante_t * from, transition_vec_t parent ) {

    for ( int32_t c = 0, lc = ( int32_t ) vector_size ( parent ); c < lc; ++c ) {

        if ( parent [ c ].cost < walk->cost ) {

            if ( tritset_sub ( from->values, parent [ c ].values ) ) { // Reached goal?..

                walk->best = parent + c;
                walk->cost = parent [ c ].cost;
            }

            else if ( ( parent [ c ].cost + 1 ) < walk->cost ) {

                transition_vec_t children = goap_get_possible_actions ( ap, parent + c );

                vector_set_capacity ( children, ( size_t ) walk->list ); // Create the linked list...
                walk->list = children;

                goap_walk ( ap, walk, from, children );
            }
        }
    }
}


action_plan_t goap_plan ( ap_t * ap, ws_ante_t * f_, ws_ante_t * g_ ) {

    transition_vec_t parent = NULL;
    vector_init ( parent, 1 );
    vector_set_capacity ( parent, ( size_t ) NULL ); // Start of the linked list (for clean-up)...

    parent [ 0 ].name   = "goal";
    parent [ 0 ].values = g_->values;

    walk_t walk;

    walk.list = parent;
    walk.best = NULL;
    walk.cost = INT_MAX;

    // Walk the tree constructed with goal (parent) as
    // root, working backwards...

    goap_walk ( ap, &walk, f_, parent );

    // Build action plan...

    action_plan_t plan;

    memset ( &plan, 0, sizeof ( action_plan_t ) );

    if ( walk.best ) {

        printf ( "\n%i ", walk.cost );

        plan.cost = walk.cost;
        plan.state.values = walk.best->values;

        vector_allocate ( plan.action_names );

        tritset_t * t = &walk.best->values;

        while ( walk.best->parent ) {

            printf ( "%s, ", walk.best->name );
            vector_push_back ( plan.action_names, walk.best->name );
            walk.best = walk.best->parent;
        }

        *t = NULL; // Transfer ownership to plan.state...

        putchar ( '\n' );
    }

    // Clean up...

    while ( walk.list ) {

        for ( int32_t i = 0, li = ( int32_t ) vector_size ( walk.list ); i < li; ++i ) {

            vector_free ( walk.list [ i ].values );
        }

        transition_ptr_t t = walk.list;
        walk.list = ( transition_ptr_t ) vector_capacity ( t );
        vector_free ( t );
    }

    return plan;
}
