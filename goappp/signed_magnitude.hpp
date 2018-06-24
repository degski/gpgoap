
#pragma once

#include <cassert>
#include <ciso646>

#include <array>
#include <iostream.hpp> // <iostream> + nl, sp etc. defined...
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

  //this struct is 8 bits wide with a magnitude and a binary value and the compiler does everything appropriately automatically.

template<typename T, typename = std::enable_if_t<std::is_unsigned<T>::value and std::is_integral<T>::value, T>>
struct smint {

    using value_type = T;

    // Primitives

    static constexpr T sm_msb ( ) noexcept { return T ( 1 ) << ( sizeof ( T ) * 8 - 1 ); } // Return a T with the most significant bit set.
    static constexpr bool sm_negative ( const T v ) noexcept { return v & sm_msb ( ); }
    static constexpr bool sm_positive ( const T v ) noexcept { return not ( sm_negative ( v ) ); }
    static constexpr std::int32_t sm_sgn ( const T v ) noexcept { return ( std::int32_t ) not ( v & sm_msb ( ) ) * 2 - 1; }
    static constexpr T sm_mag ( const T v ) noexcept { return v & ~sm_msb ( ); }
    static constexpr T sm_neg ( const T v ) noexcept { return v ^ sm_msb ( ); }

    // Conversion to 2's complement.
    template<typename U, typename = std::enable_if_t<std::is_same<std::uint8_t, U>::value, U>>
    static constexpr std::int8_t sm_to_int ( const U a ) noexcept {
        const U mag = sm_mag ( a );
        return sm_negative ( a ) ? -( ( std::int8_t ) mag ) : ( ( std::int8_t ) mag );
    }
    // Conversion to 2's complement.
    template<typename U, typename = std::enable_if_t<std::is_same<std::uint16_t, U>::value, U>>
    static constexpr std::int16_t sm_to_int ( const U a ) noexcept {
        const U mag = sm_mag ( a );
        return sm_negative ( a ) ? -( ( std::int16_t ) mag ) : ( ( std::int16_t ) mag );
    }
    // Conversion to 2's complement.
    template<typename U, typename = std::enable_if_t<std::is_same<std::uint32_t, U>::value, U>>
    static constexpr std::int32_t sm_to_int ( const U a ) noexcept {
        const U mag = sm_mag ( a );
        return sm_negative ( a ) ? -( ( std::int32_t ) mag ) : ( ( std::int32_t ) mag );
    }
    // Conversion to 2's complement.
    template<typename U, typename = std::enable_if_t<std::is_same<std::uint64_t, U>::value, U>>
    static constexpr std::int64_t sm_to_int ( const U a ) noexcept {
        const U mag = sm_mag ( a );
        return sm_negative ( a ) ? -( ( std::int64_t ) mag ) : ( ( std::int64_t ) mag );
    }
    // Compare Signed Magnitude Integers.
    template<typename U>
    struct sm_compare {
        constexpr bool operator ( ) ( const U a, const U b ) const noexcept {
            return sm_to_int ( a ) < sm_to_int ( b );
        }
    };
    // Compare Magnitude of Signed Magnitude Integers.
    template<typename U>
    struct sm_compare_magnitude {
        constexpr bool operator ( ) ( const U a, const U b ) const noexcept {
            return sm_mag ( a ) < sm_mag ( b );
        }
    };

    constexpr bool operator < ( const smint rhs ) const noexcept {
        return sm_to_int ( value ) < sm_to_int ( rhs );
    }

    constexpr bool operator > ( const smint rhs ) const noexcept {
        return sm_to_int ( value ) > sm_to_int ( rhs );
    }

    constexpr bool operator <= ( const smint rhs ) const noexcept {
        return sm_to_int ( value ) <= sm_to_int ( rhs );
    }

    constexpr bool operator >= ( const smint rhs ) const noexcept {
        return sm_to_int ( value ) >= sm_to_int ( rhs );
    }

    constexpr bool operator == ( const smint rhs ) const noexcept {
        return value == rhs;
    }

    constexpr bool operator != ( const smint rhs ) const noexcept {
        return value != rhs;
    }

    constexpr T magnitude ( ) const noexcept {
        return sm_mag ( value );
    }

    constexpr std::int32_t sign ( ) const noexcept {
        return sm_sgn ( value );
    }

    T value;
};
