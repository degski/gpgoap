
#include <string.h>
#include <stdio.h>

#include <iostream.hpp>
#include <vector>

#if 0

#include <goap.hpp>


int main ( ) {

    Goap g;

    g.ante ( "scout", "armedwithgun", true );
    g.post ( "scout", "enemyvisible", true );

    g.ante ( "approach", "enemyvisible", true );
    g.post ( "approach", "nearenemy", true );

    g.ante ( "aim", "enemyvisible", true );
    g.ante ( "aim", "weaponloaded", true );
    g.post ( "aim", "enemylinedup", true );

    g.ante ( "shoot", "enemylinedup", true );
    g.cost ( "shoot", 3 );
    g.post ( "shoot", "enemyalive", false );

    g.ante ( "load", "enemyvisible", true );
    g.ante ( "load", "armedwithgun", true );
    g.post ( "load", "weaponloaded", true );

    g.ante ( "detonatebomb", "armedwithbomb", true );
    g.ante ( "detonatebomb", "nearenemy", true );
    g.cost ( "detonatebomb", 5 );
    g.post ( "detonatebomb", "alive", false );
    g.post ( "detonatebomb", "enemyalive", false );

    g.ante ( "flee", "enemyvisible", true );
    g.post ( "flee", "nearenemy", false );

    g.fact ( "enemyvisible", false );
    g.fact ( "armedwithgun", true );
    g.fact ( "weaponloaded", false );
    g.fact ( "enemylinedup", false );
    g.fact ( "enemyalive", true );
    g.fact ( "armedwithbomb", true );
    g.fact ( "nearenemy", false );
    g.fact ( "alive", true );

    g.goal ( "enemyalive", false );
    // g.goal ( "alive", false ); // add this to avoid suicide actions in plan.

    std::cout << g.opti ( ) << nl;

    g.plan ( );

    std::cout << g << nl;

    return 0;
}


#else

#include <goap.h>


int main ( ) {

    static action_planner_t ap;
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

    char desc [ 4096 ];
    goap_description ( &ap, desc, sizeof ( desc ) );
    LOGI ( "%s", desc );

    world_state_t fr;
    goap_world_state_construct ( &fr );
    goap_world_state_set ( &ap, &fr, "enemyvisible", false );
    goap_world_state_set ( &ap, &fr, "armedwithgun", true );
    goap_world_state_set ( &ap, &fr, "weaponloaded", false );
    goap_world_state_set ( &ap, &fr, "enemylinedup", false );
    goap_world_state_set ( &ap, &fr, "enemyalive", true );
    goap_world_state_set ( &ap, &fr, "armedwithbomb", true );
    goap_world_state_set ( &ap, &fr, "nearenemy", false );
    goap_world_state_set ( &ap, &fr, "alive", true );

    goap_set_cost ( &ap, "detonatebomb", 5 );	// make suicide more expensive than shooting.

    world_state_t goal;
    goap_world_state_construct ( &goal );
    goap_world_state_set ( &ap, &goal, "enemyalive", false );
    // goap_world_state_set( &ap, &goal, "alive", false ); // add this to avoid suicide actions in plan.


    /*
    action_plan_t * plan = NULL;

    const int plan_cost = astar_plan ( &ap, &fr, &goal, &plan ); // , &states );

    LOGI ( "plan_cost = %d", plan_cost );
    goap_world_state_description ( &ap, &fr, desc, sizeof ( desc ) );
    LOGI ( "%-23s%s", "", desc );

    for ( int i = 0, l = vector_size ( plan ); i<l && i<16; ++i ) {

    goap_world_state_description ( &ap, &( plan + i )->world_state, desc, sizeof ( desc ) );
    LOGI ( "%d: %-20s%s", i, plan [ i ].action, desc );
    }
    */

    return 0;
}

#endif
