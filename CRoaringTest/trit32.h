
#pragma once

#include "trit.h"


typedef uint64_t trit32_t;


static inline uint64_t trit32_read ( trit32_t a_, int32_t idx_ ) {

    return ( a_ >> ( idx_ << 1 ) ) & TRIT32_2L;
}


#define t32_high TRIT32_HB
#define t32_low TRIT32_LB
#define t32_ones TRIT32_NT
#define t32_zeros TRIT32_00
#define t32_mask( i ) ( TRIT32_2L << ( i ) )
#define t32_set( i, v ) ( ( v ) << ( i ) )
#define t32_left( a ) ( ( a ) << 1 )
#define t32_right( a ) ( ( a ) >> 1 )
#define t32_not( a ) ( ~( a ) )
#define t32_andnot( a, b ) ( ( trit32_t ) ( ~( a ) ) & ( trit32_t ) ( b ) )
#define t32_and( a, b ) ( ( a ) & ( b ) )
#define t32_or( a, b ) ( ( a ) | ( b ) )
#define t32_xor( a, b ) ( ( a ) ^ ( b ) )
#define t32_test( a, b ) ( ( a ) == ( b ) )

TRIT_SET ( 32, uint64_t )
