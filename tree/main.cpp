
#include <ciso646>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream.hpp> // <iostream> + nl, sp etc. defined...
#include <iterator>
#include <list>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <boost/container/deque.hpp>


template<typename T>
struct node;

template<typename T> using node_pointer = node<T> *;
template<typename T> using node_const_pointer = const node<T> *;
template<typename T> using node_reference = node<T> &;
template<typename T> using node_const_reference = const node<T> &;
template<typename T> using node_rv_reference = node<T> &&;
template<typename T> using store = boost::container::deque<node<T>>;


template<typename T>
struct state {

    std::vector<T> values;
    std::int32_t cost = 0;
    T id = std::numeric_limits<T>::max ( );

    state ( ) noexcept { }
    state ( const std::int32_t c, const T id ) noexcept :
        cost ( c ),
        id ( id ) { }
    state ( std::vector<T> && v, const std::int32_t c, const T id ) noexcept :
        values ( std::forward<std::vector<T>> ( v ) ),
        cost ( c ),
        id ( id ) { }
};


template<typename T>
struct tree;


template<typename T>
struct node {

    node_pointer<T> parent = nullptr;
    std::vector<node_pointer<T>> children;

    node ( ) noexcept { }
    node ( node_reference<T> p ) noexcept :
        parent ( & p ) { }
    template<typename... Args>
    node ( node_reference<T> p, Args && ...args ) :
        parent ( & p ),
        data ( std::forward<Args> ( args )... ) {

        parent->children.emplace_back ( this );
    }

    T data;
};


template<typename T>
struct tree {

    store<T> store;

    node_reference<T> root ( ) {
        return store.front ( );
    }

    template<typename... Args>
    node_reference<T> emplace_back ( Args && ...args ) {
        return store.emplace_back ( std::forward<Args> ( args )... );
    }

    tree ( ) : store ( 1 ) { }
};



int main ( ) {

    tree<state<std::uint16_t>> tr;

    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 1, 2, 3 }, 4, 7 );
    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 3, 2, 3 }, 5, 9 );
    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 1, 2, 6 }, 1, 5 );

    std::cout << tr.root ( ).children.size ( ) << nl;

	return 0;
}
