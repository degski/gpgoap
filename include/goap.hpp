
#pragma once

#include <stdio.h>

#include "./gpgoap/vector.h"
#include "./gpgoap/astar.h"	// for A* search over world_state space.
#include "./gpgoap/goap.h"	    // for planner interface.


#ifdef NDEBUG
#pragma comment ( lib, "gpgoap-s.lib" )
#else
#pragma comment ( lib, "gpgoap-s-d.lib" )
#endif


struct Goap {

    using name_t = char*;
    using const_name_t = const char*;

    Goap ( ) noexcept {

        goap_action_planner_construct ( &m_planner ); // Clear all information...
        goap_world_state_construct ( &m_state );
        goap_world_state_construct ( &m_goal );
    }

    ~Goap ( ) noexcept {

        goap_world_state_destroy ( &m_goal );
        goap_world_state_destroy ( &m_state );
        goap_action_planner_destroy ( &m_planner ); // Clear all information...
    }

    void ante ( const_name_t action_name_, const_name_t atom_name_, const bool value_ ) noexcept {

        goap_set_ante ( &m_planner, action_name_, atom_name_, value_ );
    }

    void post ( const_name_t action_name_, const_name_t atom_name_, const bool value_ ) noexcept {

        goap_set_post ( &m_planner, action_name_, atom_name_, value_ );
    }

    void cost ( const_name_t action_name_, const int value_ ) noexcept {

        goap_set_cost ( &m_planner, action_name_, value_ );
    }

    void fact ( const_name_t atom_name_, const bool value_ ) noexcept {

        goap_world_state_set ( &m_planner, &m_state, atom_name_, value_ );
    }

    void goal ( const_name_t atom_name_, const bool value_ ) noexcept {

        goap_world_state_set ( &m_planner, &m_goal, atom_name_, value_ );
    }

    bool opti ( ) noexcept {

        return goap_action_planner_run_optimize ( &m_planner );
    }

    int plan ( ) {

        vector_clear ( m_plan );

        return m_plan_cost = astar_plan ( &m_planner, &m_state, &m_goal, &m_plan );
    }

    template<typename S>
    friend S & operator << ( S & out_, const Goap & g_ ) {

        g_.print ( );

        return out_;
    }

    private:

    void print ( ) const noexcept {

        char desc [ 4096 ];
        goap_description ( &m_planner, desc, sizeof ( desc ) );

        LOGI ( "Plan cost: %d", m_plan_cost );

        goap_world_state_description ( &m_planner, &m_state, desc, sizeof ( desc ) );
        LOGI ( "%-23s%s", "", desc );

        for ( int i = 0; i < vector_size ( m_plan ); ++i ) {

            goap_world_state_description ( &m_planner, &( m_plan + i )->world_state, desc, sizeof ( desc ) );
            LOGI ( "%d: %-20s%s", i, m_plan [ i ].action, desc );
        }
    }

    action_planner_t m_planner;
    world_state_t m_state, m_goal;
    action_plan_t * m_plan = nullptr;
    int m_plan_cost = 0;
};
