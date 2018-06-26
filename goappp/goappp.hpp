
#pragma once

#include <cassert>
#include <ciso646>
#include <cstdint>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream.hpp>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>

#include <cityhash.h>
#include <farmhash.h>
#include <tsl/htrie_map.h> // https://github.com/Tessil/hat-trie
#include <fluent/named_type.hpp> // https://github.com/joboccara/NamedType

#include "sorted_vector_set.hpp"

using cost_type = fluent::NamedType<std::int32_t, struct cost_type_s>;

namespace std {

template<>
struct is_pod<cost_type> : public std::true_type { };
}


// namespace goap {

// attribute<T> allows for sparse representation of attributes (sorted vectors
// of attribute<T>). F.e. { -3, 6, 9, -10 }, attribute 3 is off, attribute 6 is
// on, etc etc.

template<typename T, typename = std::enable_if_t<std::is_unsigned<T>::value and std::is_integral<T>::value, T>>
struct attribute {

    private:

    template<typename U>
    static constexpr std::int32_t attribute_width ( ) noexcept { return ( std::int32_t ) sizeof ( U ) * 8 - 1; }

    public:

    using value_type = T;

    T value : 1;
    T index : attribute_width<T> ( );

    attribute ( ) = delete; // I cannot think of a reasonable default value.
    attribute ( const attribute & a ) { *( ( T* ) this ) = *( ( T* ) & a ); }
    attribute ( const bool v, const T i ) noexcept : value ( v ), index ( i ) { }
    attribute ( const T i ) noexcept : value ( false ), index ( i ) { }

    [[ maybe_unused ]] constexpr attribute & operator = ( const attribute & a ) { *( ( T* ) this ) = *( ( T* ) & a ); return *this; }

    constexpr bool operator == ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." ); // This should not happen, but apparently it can happen, hence the check.
        return *( ( T* ) this ) == *( ( T* ) &rhs );
    }

    constexpr bool operator != ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." );
        return *( ( T* ) this ) != *( ( T* ) &rhs );
    }

    constexpr bool operator < ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." );
        return index < rhs.index;
    }

    constexpr bool operator > ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." );
        return index > rhs.index;
    }

    constexpr bool operator <= ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." );
        return index <= rhs.index;
    }

    constexpr bool operator >= ( const attribute rhs ) const noexcept {
        static_assert ( sizeof ( *this ) == sizeof ( T ), "Error: bit-field padding added." );
        return index >= rhs.index;
    }

    friend std::basic_ostream<char> & operator << ( std::basic_ostream<char> & os, const attribute a ) noexcept {
        std::basic_ostringstream<char> oss;
        oss << ( a.value ? '+' : '-' ) << a.index;
        os << oss.str ( );
        return os;
    }
};

namespace std {

template<typename T>
struct is_pod<attribute<T>> : public std::true_type { };
}


// The attributes are stored in a flat set, maintained fully sorted at all times.
// An attribute can be either true or false, or is assumed to be, when not present,
// un-determined. Every action has (non/a/some) pre-requisite attribute(s), and has
// one or more result attributes (required, an action has to have a result).

template<typename T>
using attributes = sorted_vector_set<attribute<T>, std::vector<attribute<T>>>;

template<typename T>
std::basic_ostream<char> & operator << ( std::basic_ostream<char> & out, const attributes<T> a ) noexcept {
    auto it = a.cbegin ( );
    out << *it;
    for ( ; it != a.cend ( ); ) {
        out << " " << *++it;
    }
    return out;
}

// Are attributes a a subset of attributes b?
template<typename T>
bool is_subset ( const attributes<T> & a, const attributes<T> & b ) noexcept {
    if ( a.size ( ) > b.size ( ) ) {
        return false;
    }
    const auto ea = a.cend ( ), eb = b.cend ( );
    auto ia = a.cbegin ( ), ib = b.cbegin ( );
    while ( ia != ea and ib != eb ) {
        ia += *ia == *ib; ++ib;
    }
    return ia == ea;
}


template<typename T>
struct action_ante_s {      // The pre-requisites of a single action.
    attributes<T> values;
};

template<typename T>
struct action_post_s {      // The results of a single action.
    attributes<T> values;
    bool enabled = true;    // Actions are enabled by default.
};


template<typename T>
using action_ante = std::vector<action_ante_s<T>>;
template<typename T>
using action_post = std::vector<action_post_s<T>>;
template<typename T>
using world = action_ante_s<T>;


template<typename T>
struct planner;

// Provides/creates bi-map (string <--> index). A hat-trie for
// string -> index, a std::vector for index -> string.

template<typename T, typename = std::enable_if_t<std::is_unsigned<T>::value and std::is_integral<T>::value, T>>
class indexer {

    friend planner<T>;

    struct str_hash_s {
        std::size_t operator ( ) ( const char * key, std::size_t key_size ) const noexcept {
            return CityHash64 ( key, key_size );
            // return util::Hash64 ( key, key_size ); // FarmHash.
        }
    };

    public:

    using name_index_map = tsl::htrie_map<char, T, str_hash_s>;
    using name_index_iterator = typename name_index_map::iterator;
    using const_name_index_iterator = typename name_index_map::const_iterator;

    using name_vector = std::vector<std::string>;
    using name_vector_iterator = typename name_vector::iterator;
    using const_name_vector_iterator = typename name_vector::const_iterator;

    private:

    name_index_map m_indices;   // Mapping names to indices.
    name_vector m_names;        // Names from indices.

    public:

    name_vector_iterator begin ( ) noexcept { return m_names.begin ( ); }
    const_name_vector_iterator begin ( ) const noexcept { return m_names.begin ( ); }
    const_name_vector_iterator cbegin ( ) const noexcept { return m_names.cbegin ( ); }

    name_vector_iterator end ( ) noexcept { return m_names.end ( ); }
    const_name_vector_iterator end ( ) const noexcept { return m_names.end ( ); }
    const_name_vector_iterator cend ( ) const noexcept { return m_names.cend ( ); }

    const name_index_map & indices ( ) const noexcept {
        return m_indices;
    }

    const name_vector & names ( ) const noexcept {
        return m_names;
    }

    std::size_t size ( ) const noexcept {
        return m_names.size ( );
    }

    bool is_created ( const std::string & v ) const noexcept {
        return m_indices.end ( ) != m_indices.find ( v );
    }

    bool is_created ( const attribute<T> a ) const noexcept {
        return ( std::size_t ) a.index < m_names.size ( );
    }

    // Get string index (non-const overload). Creates a new index on first use,
    // return a std::pair of index and a bool (true if a new index was created).
    std::pair<T, bool> operator [ ] ( std::string && s ) {
        const auto it = m_indices.find ( s );
        if ( m_indices.end ( ) != it ) {
            return { it.value ( ), false };
        }
        else {
            const T idx = ( T ) m_names.size ( );
            m_indices.insert ( s, idx );
            m_names.emplace_back ( std::move ( s ) );
            return { idx, true };
        }
    }

    // Get string index (const overload), asserts the string exists.
    T operator [ ] ( const std::string & s ) const noexcept {
        assert ( m_indices.end ( ) != m_indices.find ( s ) );
        return m_indices.find ( s ).value ( );
    }

    // Get string index. Asserts there are no newly created key-value pairs.
    T at ( const std::string & s ) const noexcept {
        assert ( m_indices.end ( ) != m_indices.find ( s ) );
        return m_indices.find ( s ).value ( );
    }

    const std::string & operator [ ] ( const attribute<T> a ) const noexcept {
        assert ( is_created ( a ) );
        return m_names [ a.index ];
    }

    const std::string & operator [ ] ( const std::size_t i ) const noexcept {
        assert ( i < m_names.size ( ) );
        return m_names [ i ];
    }

    const std::string & operator [ ] ( const T i ) const noexcept {
        assert ( ( std::size_t ) i < m_names.size ( ) );
        return m_names [ ( std::size_t ) i ];
    }

    friend std::basic_ostream<char> & operator << ( std::basic_ostream<char> & os, const indexer indexer ) noexcept {
        name_vector n ( indexer.m_names );
        std::sort ( std::begin ( n ), std::end ( n ) );
        for ( const std::string & a : n ) {
            std::cout << a << '.' << ( std::size_t ) indexer.m_indices.at ( a ) << nl;
        }
        return os;
    }
};


template<typename T>
struct state;

template<typename T>
using states = std::vector<state<T>>;

template<typename T>
struct state {

    states<T> children; // 16
    const state<T> * parent = nullptr; // 8

    attributes<T> values; // 16
    cost_type cost { 0 }; // 4
    T id = std::numeric_limits<T>::max ( ); // 2

    state ( const attributes<T> & v ) noexcept : values ( v ) { }
    state ( const state * p, cost_type && c, const T i ) noexcept :
        parent { p },
        cost { std::forward<cost_type> ( c ) },
        id { i } { }
    state ( const state * p, attributes<T> && v, cost_type && c, const T i ) noexcept :
        parent { p },
        values { std::forward<attributes<T>> ( v ) },
        cost { std::forward<cost_type> ( c ) },
        id { i } { }
};

template<typename T>
struct state_tree {

    state_tree ( ) = delete;
    state_tree ( const attributes<T> & v ) : root { state<T> { v } } { }

    state<T> root;
};



template<typename T>
struct action_plan {

    std::vector<T> ids;
    world<T> world;
    cost_type cost { 0 };

    friend std::basic_ostream<char> & operator << ( std::basic_ostream<char> & os, const action_plan plan ) noexcept {


        return os;
    }
};


template<typename T>
struct planner {

    indexer<T> m_attributes;                    // World world attributes (properties of the world of the world), every attribute has an index.
    indexer<T> m_actions;                       // What to do, how to act, every action has an index.

    action_ante<T> m_action_ante;               // Pre-conditions (action_ante) for all actions.
    action_post<T> m_action_post;               // Post-conditions for all actions (action effects), can be dis-abled by action index.
    std::vector<cost_type> m_action_cost;         // Cost for all actions, default action_cost is 1.

    world<T> m_world;
    world<T> m_target_attributes;               // The required attributes of the next world (the goal).

    std::size_t action_index ( std::string && action ) {

        const std::pair<std::size_t, bool> a = m_actions [ std::move ( action ) ];

        if ( a.second ) {

            m_action_ante.emplace_back ( );
            m_action_post.emplace_back ( );
            m_action_cost.emplace_back ( 1 );   // The default action cost of any action is 1.
        }

        return a.first;

    }

    public:

    planner ( ) {

        m_actions.m_indices.insert ( std::string ( "goal" ), std::numeric_limits<T>::max ( ) );

        m_action_ante.reserve ( 16 );
        m_action_post.reserve ( 16 );
        m_action_cost.reserve ( 16 );
    }

    // Below are all setters, the set_ prefix has been omitted, on first use
    // a new index will be created, so typos are not good. The print_actions ()
    // function should be used to check for errors. Both actions and attributes
    // are required to be lower-case only.

    // Set or update pre-(ex-ante-)condition of an action.
    void action_ante ( std::string && action, std::string && attribute_, const bool value ) {
        assert ( tolower ( action ) == action );
        assert ( tolower ( attribute_ ) == attribute_ );
        m_action_ante [ action_index ( std::move ( action ) ) ].values.insert_or_update_unsafe ( attribute<T> { value, m_attributes [ std::move ( attribute_ ) ].first } );
    }

    // Set or update post-condition of an action.
    void action_post ( std::string && action, std::string && attribute_, const bool value ) {
        assert ( tolower ( action ) == action );
        assert ( tolower ( attribute_ ) == attribute_ );
        m_action_post [ action_index ( std::move ( action ) ) ].values.insert_or_update_unsafe ( attribute<T> { value, m_attributes [ std::move ( attribute_ ) ].first } );
    }

    // Set cost of an action, the default is 1, higher means less likely to be taken.
    void action_cost ( const std::string & action, const cost_type value ) noexcept {
        m_action_cost [ m_actions.at ( action ) ] = value;
    }

    // Activate or deactivate an action (like if tired cannot run).
    void action_activation ( const std::string & action, const bool value ) noexcept {
        m_action_post [ m_actions.at ( action ) ].enable = value;
    }

    // Keeps track of the attributes (on/off) of the world.
    void current_world ( const std::string & attribute_, const bool value ) { // Sets the current world world.
        m_world.values.insert_or_update_unsafe ( attribute<T> { value, m_attributes.at ( attribute_ ) } );
    }

    // The value of the target world.
    void target_attributes ( const std::string & attribute_, const bool value ) {
        m_target_attributes.values.insert_or_update_unsafe ( attribute<T> { value, m_attributes.at ( attribute_ ) } );
    }

    void target_attributes_erase ( const std::string & attribute_ ) noexcept {
        m_target_attributes.values.erase ( attribute<T> ( m_attributes.at ( attribute_ ) ) );
    }

    // Checks that all actions have at least one post-condition value set.
    bool verify_actions ( ) const noexcept {
        for ( const auto & p : m_action_post ) {
            if ( p.values.empty ( ) ) {
                return false;
            }
        }
        return true;
    }

    // The actual planner.

    // private:

    // Back-propagate un-satisfied pre-conditions and target post-conditions.
    void backpropagate_attributes ( attributes<T> & child_attributes, const attributes<T> & parent, const attributes<T> & post, const attributes<T> & ante ) const noexcept {
        using const_iterator = typename attributes<T>::const_iterator;
        using container = typename attributes<T>::container;
        const const_iterator epa = parent.cend ( ), ean = ante.cend ( ), epo = post.cend ( );
        const_iterator ipa = parent.cbegin ( ), ipo = post.cbegin ( );
        child_attributes.reserve ( parent.size ( ) + ante.size ( ) ); // Reserve space for a full merge (possibly over-sized).
        container & acr = child_attributes.container_ref ( );
        while ( ipa != epa ) {
            if ( *ipa != *ipo )
                acr.push_back ( *ipa );
            else
                ++ipo;
            ++ipa;
            if ( epo == ipo ) {
                if ( ipa != epa ) {
                    acr.insert ( acr.cend ( ), ipa, epa );                                          // Add the rest of parent.
                }
                break;
            }
        }
        for ( const auto a : ante ) {
            child_attributes.insert ( a );
        }
    }

    states<T> & children ( state<T> & parent ) noexcept {
        const std::size_t z = m_action_post.size ( );
        for ( std::size_t i = 0; i < z; ++i ) {
            const auto & api = m_action_post [ i ];
            if ( api.enabled and is_subset ( api.values, parent.values ) ) {
                parent.children.emplace_back ( & parent, cost_type { parent.cost.get ( ) + m_action_cost [ i ].get ( ) }, ( T ) i );
                backpropagate_attributes ( parent.children.back ( ).values, parent.values, api.values, m_action_ante [ i ].values );
                // std::cout << m_actions [ i ] << " ";
                // print_attributes ( parent.children.back ( )->values );
            }
        }
        // if ( parent.children.size ( ) ) { std::cout << nl; }
        return parent.children;
    }

    mutable const state<T> * best_ptr = nullptr;
    mutable cost_type best_cost { std::numeric_limits<cost_type::UnderlyingType>::max ( ) };

    // Recursive Depth First Search, depth is limited by best_cost sofar.
    // This is much easier/cheaper than doing a non-recurive BFS (which
    // would be optimal), as we would have to keep track of visited nodes.
    void dfs ( states<T> & parent, const world<T> & ante ) noexcept {
        for ( auto & t : parent ) {
            if ( t.cost.get ( ) < best_cost.get ( ) ) {
                if ( is_subset ( t.values, ante.values ) ) { // Reached goal?
                    best_ptr = &t;
                    best_cost = t.cost;
                }
                else if ( ( t.cost.get ( ) + 1 ) < best_cost.get ( ) ) {
                    dfs ( children ( t ), ante );
                }
            }
        }
    }

    public:

    action_plan<T> plan ( ) noexcept {

        // Walk the tree constructed with goal (parent) as
        // root, working backwards.

        state_tree<T> tree ( m_target_attributes.values );
        dfs ( children ( tree.root ), m_world );

        // Build action plan.

        action_plan<T> plan;

        plan.world = m_world;

        if ( nullptr != best_ptr ) {

            std::cout << nl << "cost " << best_cost.get ( ) << ": ";
            plan.cost = best_cost;

            if ( nullptr != best_ptr->parent ) {

                const std::size_t id = best_ptr->id;
                std::cout << m_actions [ id ];
                plan.ids.push_back ( id );
                for ( const auto v : m_action_post [ id ].values ) {
                    plan.world.values.update_unsafe ( v );
                }
                best_ptr = best_ptr->parent;
            }

            while ( nullptr != best_ptr->parent ) {

                const std::size_t id = best_ptr->id;
                std::cout << ", " << m_actions [ id ];
                plan.ids.push_back ( id );
                for ( const auto v : m_action_post [ id ].values ) {
                    plan.world.values.update_unsafe ( v );
                }
                best_ptr = best_ptr->parent;
            }

            std::cout << nl;

            print_world ( plan.world );
        }

        else {

            std::cout << "No viable plan to achieve goal has been found." << nl;
        }

        std::cout << "plan completed." << nl;

        return plan;
    }

    // Print stuff.

    static std::string toupper ( const std::string & s ) noexcept {
        std::string t;
        std::transform ( std::begin ( s ), std::end ( s ), std::back_inserter ( t ), ::toupper );
        return t;
    }

    static std::string tolower ( const std::string & s ) noexcept {
        std::string t;
        std::transform ( std::begin ( s ), std::end ( s ), std::back_inserter ( t ), ::tolower );
        return t;
    }

    void print_attributes ( const attributes<T> & values_ ) const noexcept {
        std::vector<std::string> values;
        values.reserve ( values_.size ( ) );
        std::transform ( std::begin ( values_ ), std::end ( values_ ), std::back_inserter ( values ), [ & ] ( const attribute<T> v ) { return v.value ? toupper ( m_attributes [ v ] ) : m_attributes [ v ]; } );
        std::sort ( std::begin ( values ), std::end ( values ), [ & ] ( const std::string & a, const std::string & b ) { return _stricmp ( a.c_str ( ), b.c_str ( ) ) < 0; } );
        for ( const auto & v : values ) {
            std::cout << '<' << v << '>';
        }
        std::cout << nl;
    }

    template<typename State>
    void print_world ( const State & world ) const noexcept {
        print_attributes ( world.values );
    }

    void print_attributes ( ) const noexcept {
        std::cout << m_attributes;
    }

    void print_indexer ( ) const noexcept {
        std::cout << m_actions;
    }

    void print_current_world ( ) const noexcept {
        print_world ( m_world );
    }

    void print_target_attributes ( ) const noexcept {
        print_world ( m_target_attributes );
    }

    void print_action ( const std::size_t i ) const noexcept {
        std::cout << "Action " << std::setw ( 3 ) << i << ": " << m_actions [ i ] << nl;
        std::cout << "Enabled   : " << std::boolalpha << m_action_post [ i ].enabled << nl;
        std::cout << "Ante(s)   : "; print_world ( m_action_ante [ i ] );
        std::cout << "Post(s)   : "; print_world ( m_action_post [ i ] );
        std::cout << "Cost      : " << m_action_cost [ i ].get ( ) << nl;
    }

    void print_actions ( ) const noexcept {
        for ( std::size_t i = 0, z = m_action_ante.size ( ); i < z; ++i ) {
            print_action ( i ); std::cout << nl;
        }
    }
};


using idx_t = struct idx_s {

    std::int32_t value;
    bool added;
};

template<typename T>
struct action_plan_t {

    std::vector<std::string> action_names;
    std::int32_t action_cost;
    world<T> world;
};


template<typename T, typename S>
T pun_as ( const S & val ) noexcept {
    T t { };
    std::memcpy ( &t, &val, std::min ( sizeof ( T ), sizeof ( S ) ) );
    return t;
}

//} // namespace goap
