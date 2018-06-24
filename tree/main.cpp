
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



int main346546 ( ) {

    tree<state<std::uint16_t>> tr;

    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 1, 2, 3 }, 4, 7 );
    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 3, 2, 3 }, 5, 9 );
    tr.emplace_back ( tr.root ( ), std::vector<std::uint16_t> { 1, 2, 6 }, 1, 5 );

    std::cout << tr.root ( ).children.size ( ) << nl;

	return 0;
}

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

int main ( ) {
    using namespace nana;

    //Define a form.
    form fm;

    //Define a label and display a text.
    label lab { fm, "Hello, <bold blue size=16>Nana C++ Library</>" };
    lab.format ( true );

    //Define a button and answer the click event.
    button btn { fm, "Quit" };
    btn.events ( ).click ( [ &fm ] {
        fm.close ( );
    } );

    //Layout management
    fm.div ( "vert <><<><weight=80% text><>><><weight=24<><button><>><>" );
    fm [ "text" ] << lab;
    fm [ "button" ] << btn;
    fm.collocate ( );

    //Show the form
    fm.show ( );

    //Start to event loop process, it blocks until the form is closed.
    exec ( );
}
