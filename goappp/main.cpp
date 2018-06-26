
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

#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <vector>


// std::ostream & nl ( std::ostream  & out_ ) { return out_ << '\n'; }

template <typename T>
struct relocator {

    using value_type = T;
    using pointer = value_type * ;

    template <class U> struct rebind { typedef relocator<U> other; };

    relocator ( ) { };
    relocator ( const relocator & ) = delete;
    relocator ( relocator && ) = delete;
    ~relocator ( ) noexcept {
        if ( m_pointer ) {
            std::cout << "freed " << m_pointer << nl;
            std::free ( m_pointer );
        }
    }

    relocator & operator = ( const relocator & ) = delete;
    relocator & operator = ( relocator && ) = delete;

    template <class U> relocator ( relocator<U> const & ) noexcept { }

    pointer allocate ( std::size_t n ) {
        if ( m_pointer ) {
            pointer p = m_pointer;
            m_pointer = static_cast<pointer> ( std::realloc ( m_pointer, n * sizeof ( T ) ) );
            std::cout << "realloced " << p << " to " << m_pointer << nl;
        }
        else {
            m_pointer = static_cast<pointer> ( std::malloc ( n * sizeof ( T ) ) );
            std::cout << "malloced " << m_pointer << nl;
        }
        return m_pointer;
    }

    void deallocate ( pointer, std::size_t ) noexcept { }

    private:

    pointer m_pointer = nullptr;
};

template <typename T, typename U>
bool operator == ( relocator<T> const & l, relocator<U> const & r ) noexcept {
    return l.m_pointer == r.m_pointer;
}

template <typename T, typename U>
bool operator != ( relocator<T> const& l, relocator<U> const& r ) noexcept {
    return l.m_pointer != r.m_pointer;
}

int __cdecl main ( ) {
    {
        std::vector<int, relocator<int>> v1, v2;

        for ( int i = 0; i < 20000; ++i ) {
            v1.push_back ( i );
            v2.push_back ( i );
        }
    }

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
