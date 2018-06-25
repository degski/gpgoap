
#include <ciso646>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream.hpp> // <iostream> + nl, sp etc. defined.
#include <iterator>
#include <list>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

// namespace fs = std::experimental::filesystem;

#include <autotimer.hpp>
#include <snitch.hpp>

#include "goappp.hpp"

using int_vector = std::vector<int, relocator<int>>;

int __cdecl main ( ) {

    std::cout << "enter scope" << nl;

    {
        int_vector v1;

        for ( int i = 0; i < 20000; ++i ) {
            v1.push_back ( i );
            std::cout << "pushed " << i << nl;
        }

        for ( auto i : v1 ) {
            std::cout << i << nl;
        }
    }

    std::cout << "out of scope" << nl;

    return 0;
};


int __cdecl main233546576 ( ) {

    {

        planner<std::uint16_t> ap;

        ap.action_ante ( "scout", "armedwithgun", true );
        ap.action_post ( "scout", "enemyvisible", true );

        ap.action_ante ( "approach", "enemyvisible", true );
        ap.action_post ( "approach", "nearenemy", true );

        ap.action_ante ( "aim", "enemyvisible", true );
        ap.action_ante ( "aim", "weaponloaded", true );
        ap.action_post ( "aim", "enemylinedup", true );

        ap.action_ante ( "shoot", "enemylinedup", true );
        ap.action_post ( "shoot", "enemyalive", false );

        ap.action_ante ( "load", "enemyvisible", true );
        ap.action_ante ( "load", "armedwithgun", true );
        ap.action_post ( "load", "weaponloaded", true );

        ap.action_ante ( "detonatebomb", "armedwithbomb", true );
        ap.action_ante ( "detonatebomb", "nearenemy", true );
        ap.action_post ( "detonatebomb", "alive", false );
        ap.action_post ( "detonatebomb", "enemyalive", false );

        ap.action_ante ( "flee", "enemyvisible", true );
        ap.action_post ( "flee", "nearenemy", false );

        ap.action_cost ( "detonatebomb", cost_type { 5 } );

        ap.current_world ( "enemyvisible", false );
        ap.current_world ( "armedwithgun", true );
        ap.current_world ( "weaponloaded", false );
        ap.current_world ( "enemylinedup", false );
        ap.current_world ( "enemyalive", true );
        ap.current_world ( "armedwithbomb", true );
        ap.current_world ( "nearenemy", false );
        ap.current_world ( "alive", true );

        ap.target_attributes ( "enemyalive", false );
        ap.target_attributes ( "alive", true );

        ap.print_actions ( );
        ap.print_current_world ( ); std::cout << nl;
        ap.print_target_attributes ( ); std::cout << nl;

        ap.plan ( );
    }

    std::cout << sizeof ( state<std::uint16_t> ) << nl;
    std::cout << sizeof ( std::unique_ptr<state<std::uint16_t>> ) << nl;
    std::cout << sizeof ( states<std::uint16_t> ) << nl;

    return 0;
}
