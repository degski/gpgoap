
#include "goap.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
/*

The compiler often has different options for how it can optimize and emit the code. For example, dividing a 32-bit integer by the constant value 3

i = i / 3;

can be generated as a division instruction, but division instructions are slow, so it may be better to generate this as the equivalent of

i = (((int64_t)i * 0x55555556) >> 32) - (i >> 31);


*/

#define LOGI(...) { printf( __VA_ARGS__ ); printf("\n"); }
#define LOGW(...) { printf( "WRN " __VA_ARGS__ ); printf("\n"); }
#define LOGE(...) { printf( "ERR " __VA_ARGS__ ); printf("\n"); }



sm_t * svman2 ( sm_t * parent, sm_t * ante, sm_t * post ) {

    const int32_t lpa = ( int32_t ) vector_size ( parent ), lan = ( int32_t ) vector_size ( ante ), lpo = ( int32_t ) vector_size ( post );
    int32_t pa = 0, po = 0;

    sm_t * vec = NULL;
    vector_allocate ( vec );

    while ( pa < lpa and po < lpo ) {

        if ( parent [ pa ] != post [ po ] ) {

            vector_push_back ( vec, parent [ pa ] );
        }

        else {

            ++po;
        }

        ++pa;
    }

    for ( ; pa < lpa; ++pa ) { // add the rest of parent...

        vector_push_back ( vec, parent [ pa ] );
    }

    for ( int32_t an = 0; an < lan; ++an ) {

        vec = tritset_insert ( vec, ante [ an ] );
    }

    return vec;
}




sm_t * svman ( sm_t * parent, sm_t * ante, sm_t * post ) {

    size_t a_size = vector_size ( ante );

    sm_t * vec = NULL;
    vector_init ( vec, a_size );
    memcpy ( vec, ante, a_size * sizeof ( sm_t ) );

    sm_t *v = vec, *pa = parent, *lpa = parent + vector_size ( parent ), *po = post, *lpo = post + vector_size ( post );

    while ( pa < lpa and po < lpo ) {

        if ( *pa != *po ) {

            //while ( sm_magnitude ( *v ) < sm_magnitude ( *pa ) ) ++v;

            ++v;

            vector_grow_by ( vec, 1u );
            memmove ( v + 1u, v, ( size_t ) ( vec + vector_size ( vec ) - v ) * sizeof ( sm_t ) );
            *v = *pa;

        }
        else {

            ++po;
        }

        ++pa;
    }

    for ( ; pa < lpa; ++pa ) { // add the rest of parent...

        //while ( sm_magnitude ( *v ) < sm_magnitude ( *pa ) ) ++v;

        ++v;

        vector_grow_by ( vec, 1u );
        memmove ( v + 1u, v, ( size_t ) ( vec + vector_size ( vec ) - v ) * sizeof ( sm_t ) );
        *v = *pa;
    }

    return vec;
}



static inline int __cdecl compare_sm ( const void * a, const void * b ) {

    return sm_magnitude ( *( ( const sm_t * ) a ) ) - sm_magnitude ( *( ( const sm_t * ) b ) ); // erroneous shortcut (fails if INT_MIN is present)
}



static tritset_t tritset_sort ( tritset_t vec ) { // Insertion sort...

    const size_t v_size = vector_size ( vec );

    if ( v_size < 2u ) {

        return vec;
    }

    else if ( v_size == 2u ) {

        if ( sm_magnitude ( vec [ 0 ] ) > sm_magnitude ( vec [ 1 ] ) ) {

            const sm_t t = vec [ 0 ]; vec [ 0 ] = vec [ 1 ]; vec [ 1 ] = t;
        }

        return vec;
    }

    else {

        for ( sm_t *vi = vec + 1, *vli = vec + v_size, *vj = vi, *vk = vj - 1; vi < vli; ++vi, vj = vi, vk = vj - 1 ) {

            const sm_t v = sm_magnitude ( *vi );

            while ( vj > vec and sm_magnitude ( *vk ) > v ) {

                *vj-- = *vk--;
            }

            *vj = *vi;
        }

        return vec;
    }
}

tritset_t svman4 ( tritset_t a_, tritset_t b_, tritset_t c_ ) { // parent, ante, post

    tritset_t vec = NULL;
    vector_init ( vec, vector_size ( b_ ) );
    memcpy ( vec, b_, vector_size ( b_ ) * sizeof ( sm_t ) );

    tritset_t a = a_, la = a_ + vector_size ( a_ );

    for ( tritset_t c = c_, lc = c_ + vector_size ( c_ ); a < la and c < lc; ++a ) {

        if ( *a != *c ) {

            vector_push_back ( vec, *a );
        }

        else {

            ++c;
        }
    }

    for ( ; a < la; ++a ) {

        vector_push_back ( vec, *a );
    }

    return tritset_sort ( vec );
}



void sorted_vector_float_print ( float * vec ) {

    for ( int32_t i = 0, li = ( int32_t ) vector_size ( vec ); i < li; ++i ) {

        printf ( "%+.0f ", ( double ) vec [ i ] );
    }

    putchar ( '\n' );
}





int __cdecl main ( void ) {

    /*

    tritset_t a = NULL; // parent
    vector_allocate ( a );

    a = tritset_write ( a, 5, false );
    a = tritset_write ( a, 7, false );

    tritset_print ( a );

    tritset_t b = NULL; // post
    vector_allocate ( b );

    b = tritset_write ( b, 5, false );

    tritset_print ( b );

    tritset_t c = NULL; // ante
    vector_allocate ( c );

    c = tritset_write ( c, 4, true );

    tritset_print ( c );

    tritset_t d = svman4 ( a, c, b );

    tritset_print ( d );

    exit ( 0 );

    */

    static ap_t ap;
    goap_action_planner_construct ( &ap );

    goap_set_ante ( &ap, "scout", "armedwithgun", true );
    goap_set_post ( &ap, "scout", "enemyvisible", true );

    goap_set_ante ( &ap, "approach", "enemyvisible", true );
    goap_set_post ( &ap, "approach", "nearenemy", true );

    goap_set_ante ( &ap, "aim", "enemyvisible", true );
    goap_set_ante ( &ap, "aim", "weaponloaded", true );
    goap_set_post ( &ap, "aim", "enemylinedup", true );

    goap_set_ante ( &ap, "shoot", "enemylinedup", true );
    goap_set_post ( &ap, "shoot", "enemyalive", false );

    goap_set_ante ( &ap, "load", "enemyvisible", true );
    goap_set_ante ( &ap, "load", "armedwithgun", true );
    goap_set_post ( &ap, "load", "weaponloaded", true );

    goap_set_ante ( &ap, "detonatebomb", "armedwithbomb", true );
    goap_set_ante ( &ap, "detonatebomb", "nearenemy", true );
    goap_set_post ( &ap, "detonatebomb", "alive", false );
    goap_set_post ( &ap, "detonatebomb", "enemyalive", false );

    goap_set_ante ( &ap, "flee", "enemyvisible", true );
    goap_set_post ( &ap, "flee", "nearenemy", false );

    // goap_set_action ( &ap, "nothing", true );

    char desc [ 4096 ];
    goap_description ( &ap, desc, sizeof ( desc ) );
    LOGI ( "%s", desc );

    ws_ante_t fr;

    goap_ws_ante_construct ( &fr );
    goap_ws_set_ante ( &ap, &fr, "enemyvisible", false );
    goap_ws_set_ante ( &ap, &fr, "armedwithgun", true );
    goap_ws_set_ante ( &ap, &fr, "weaponloaded", false );
    goap_ws_set_ante ( &ap, &fr, "enemylinedup", false );
    goap_ws_set_ante ( &ap, &fr, "enemyalive", true );
    goap_ws_set_ante ( &ap, &fr, "armedwithbomb", true );
    goap_ws_set_ante ( &ap, &fr, "nearenemy", false );
    goap_ws_set_ante ( &ap, &fr, "alive", true );

    goap_set_cost ( &ap, "detonatebomb", 5 );	// make suicide more expensive than shooting.

    ws_ante_t goal;

    goap_ws_ante_construct ( &goal );

    goap_ws_set_ante ( &ap, &goal, "enemyalive", false );
    goap_ws_set_ante ( &ap, &goal, "alive", true ); // add this to avoid suicide actions in plan.


    /*


    transition_t * c = goap_get_possible_state_transitions ( &ap, &fr );

    printf ( "%llu %s\n", vector_size ( c ), c->action_name );

    goap_ws_set_ante ( &ap, &c->state, "nearenemy", true );

    transition_t * d = goap_get_possible_state_transitions ( &ap, &c->state );

    printf ( "%llu %s\n", vector_size ( d ), d [ 0 ].action_name );
    printf ( "%llu %s\n", vector_size ( d ), d [ 1 ].action_name );
    printf ( "%llu %s\n", vector_size ( d ), d [ 2 ].action_name );
    printf ( "%llu %s\n", vector_size ( d ), d [ 3 ].action_name );

    */



    action_plan_t plan = goap_plan ( &ap, &fr, &goal );

    LOGI ( "plan_cost = %d", plan.cost );

    /*

    LOGI ( "plan_cost = %d", plan_cost );
    goap_ws_ante_description ( &ap, &fr, desc, sizeof ( desc ) );
    LOGI ( "%-23s%s", "", desc );

    for ( int i = 0, l = vector_size ( plan ); i<l && i<16; ++i ) {

    goap_ws_ante_description ( &ap, &( plan + i )->world_state, desc, sizeof ( desc ) );
    LOGI ( "%d: %-20s%s", i, plan [ i ].action, desc );
    }

    */

    return 0;
}
