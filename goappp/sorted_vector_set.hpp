
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ciso646>
#include <cassert>

#include <algorithm>
#include <functional>
#include <iostream.hpp>
#include <utility>


template<typename T, typename Container, typename Compare = std::less<T>>
class sorted_vector_set {

public:

	using reference = T&;
	using rv_reference = T&&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;
	using size_type = std::size_t;
    using value_type = T;
	using container = Container;
	using iterator = typename Container::iterator;
	using const_iterator = typename Container::const_iterator;

    sorted_vector_set ( ) noexcept { }
    sorted_vector_set ( std::initializer_list<T> & init ) : m_data { std::forward<std::initializer_list<T>> ( init ) } { }
    sorted_vector_set ( const sorted_vector_set & svs ) : m_data ( svs.m_data ) { }
    sorted_vector_set ( sorted_vector_set && svs ) noexcept : m_data ( std::forward<container> ( svs.m_data ) ) { std::cout << "svs move constructed\n"; }

    //*

    ~sorted_vector_set ( ) {

        // std::cout << ( m_data.empty ( ) ? "svs destroyed" : "svs was moved from" ) << std::endl;
    }

    //*/

    [[ maybe_unused ]] sorted_vector_set & operator = ( const sorted_vector_set & svs ) { m_data = svs.m_data; return *this;  }
    [[ maybe_unused ]] sorted_vector_set & operator = ( sorted_vector_set && svs ) noexcept { m_data = std::move ( svs.m_data ); return *this; }

    // Return iterator to an element with key equivalent to t_. If no such
    // element is found, past-the-end iterator is returned.
    inline iterator find ( const_reference t_ ) noexcept {
        const iterator it = lower_bound ( t_ );
        if ( std::end ( m_data ) == it or Compare ( ) ( *it, t_ ) ) {
            return std::end ( m_data );
        }
        return it;
    }

	inline const_iterator find ( const_reference t_ ) const noexcept {
		const const_iterator it = lower_bound ( t_ );
        if ( std::cend ( m_data ) == it or Compare ( ) ( *it, t_ ) ) {
            return std::cend ( m_data );
        }
        return it;
	}

    [[ maybe_unused ]] inline iterator insert ( const_reference t_ ) noexcept {
		const iterator it = lower_bound ( t_ );
		if ( it == m_data.end ( ) or Compare ( ) ( t_, *it ) ) {
			return m_data.insert ( it, t_ );
		}
		return it;
	}

    [[ maybe_unused ]] inline iterator insert ( const_iterator it_, const_reference t_ ) noexcept {
        return m_data.insert ( it_, t_ );
    }

    [[ maybe_unused ]] inline iterator insert ( const_iterator it_, rv_reference t_ ) noexcept {
        return m_data.insert ( it_, std::move ( t_ ) );
    }

    // IMPORTANT: Changes (possibly) the value, i.e. the key, if this does not respect
    // the sort-order, the set will no longer be sorted (the part of the key used for
    // sorting should be equal to the value of that part of the key of the update).
    [[ maybe_unused ]] inline iterator insert_or_update_unsafe ( const_reference t_ ) noexcept {
        iterator it = lower_bound ( t_ );
        if ( it == m_data.end ( ) or Compare ( ) ( t_, *it ) ) {
            it = m_data.insert ( it, t_ );
        }
        else {
            *it = t_;
        }
        return it;
    }

    // IMPORTANT: Changes the value, i.e. the key AND ASSUMES THE VALUE EXISTS.
    inline void update_unsafe ( const_reference t_ ) noexcept {
        *lower_bound ( t_ ) = t_;
    }

    // Gives direct acces to the underlying container, irrespective of ordering.
    container & container_ref ( ) noexcept { return m_data; }
    const container & container_ref ( ) const noexcept { return m_data; }

	inline bool member ( const_reference t_ ) const noexcept { return not ( not_member ( t_ ) ); }
	inline bool not_member ( const_reference t_ ) const noexcept { return Compare ( ) ( t_, *lower_bound ( t_ ) ); }

	inline iterator erase ( iterator p_ ) noexcept { return m_data.erase ( p_ ); }
	inline iterator erase ( const_iterator p_ ) noexcept { return m_data.erase ( p_ ); }
	inline iterator erase ( reference t_ ) noexcept { return _Erase ( t_ ); }
	inline iterator erase ( const_reference t_ ) noexcept { return _Erase ( t_ ); }

	inline iterator begin ( ) noexcept { return m_data.begin ( ); }
    inline const_iterator begin ( ) const noexcept { return m_data.cbegin ( ); }
	inline const_iterator cbegin ( ) const noexcept { return m_data.cbegin ( ); }

	inline iterator end ( ) noexcept { return m_data.end ( ); }
    inline const_iterator end ( ) const noexcept { return m_data.cend ( ); }
	inline const_iterator cend ( ) const noexcept { return m_data.cend ( ); }

	inline const_reference front ( ) const noexcept { return m_data.front ( ); }
	inline const_reference back ( ) const noexcept { return m_data.back ( ); }

	inline void reserve ( const size_type r_ ) { m_data.reserve ( r_ ); }
	inline void clear ( ) noexcept { m_data.clear ( ); }

	inline size_type size ( ) const noexcept { return m_data.size ( ); }
	inline size_type capacity ( ) const noexcept { return m_data.capacity ( ); }
	inline bool empty ( ) const noexcept { return m_data.empty ( ); }

    // This non-const operator allows assignment to the key (i.e. the key itself is modified),
    // better make sure that the assignment does not change the sort order!!!
	inline reference operator [ ] ( const size_type i_ ) noexcept { return m_data [ i_ ]; }
	inline const_reference operator [ ] ( const size_type i_ ) const noexcept { return m_data [ i_ ]; }
	inline reference nth ( const size_type i_ ) noexcept { return m_data [ i_ ]; }
	inline const_reference nth ( const size_type i_ ) const noexcept { return m_data [ i_ ]; }
	inline reference at ( const size_type i_ ) noexcept { return m_data.at ( i_ ); }
	inline const_reference at ( const size_type i_ ) const noexcept { return m_data.at ( i_ ); }

	inline iterator lower_bound ( const_reference t_ ) noexcept {
		return std::lower_bound ( std::begin ( m_data ), std::end ( m_data ), t_, Compare ( ) );
	}

	inline const_iterator lower_bound ( const_reference t_ ) const noexcept {
		return std::lower_bound ( std::cbegin ( m_data ), std::cend ( m_data ), t_, Compare ( ) );
	}

    bool operator == ( const_reference rhs ) const noexcept {
        if ( size ( ) != rhs.size ( ) ) {
            return false;
        }
        if constexpr ( std::is_pod<T>::value ) {
            std::memcmp ( m_data.data ( ), rhs.m_data.data ( ), sizeof ( T ) * size ( ) ) == 0;
        }
        else {
            std::equal ( begin ( ), end ( ), rhs.begin ( ) );
        }
    }

    bool operator != ( const_reference rhs ) const noexcept {
        return not ( *this == rhs );
    }

	private:

	inline iterator _Erase ( const_reference t_ ) noexcept {
		const iterator it = lower_bound ( t_ );
		return Compare ( ) ( t_, *it ) ? std::end ( m_data ) : m_data.erase ( it );
	}

	container m_data;
};
