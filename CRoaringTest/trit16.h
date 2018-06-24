
#pragma once

#include "trit.h"


typedef uint32_t trit16_t;


static inline uint32_t trit16_read ( trit16_t a_, int32_t idx_ ) {

    return ( a_ >> ( idx_ << 1 ) ) & TRIT16_2L;
}


#define t16_high TRIT16_HB
#define t16_low TRIT16_LB
#define t16_ones TRIT16_NT
#define t16_zeros TRIT16_00
#define t16_mask( i ) ( TRIT16_2L << ( i ) )
#define t16_set( i, v ) ( ( v ) << ( i ) )
#define t16_left( a ) ( ( a ) << 1 )
#define t16_right( a ) ( ( a ) >> 1 )
#define t16_not( a ) ( ~( a ) )
#define t16_andnot( a, b ) ( ( trit16_t ) ( ~( a ) ) & ( trit16_t ) ( b ) )
#define t16_and( a, b ) ( ( a ) & ( b ) )
#define t16_or( a, b ) ( ( a ) | ( b ) )
#define t16_xor( a, b ) ( ( a ) ^ ( b ) )
#define t16_test( a, b ) ( ( a ) == ( b ) )

TRIT_SET ( 16, uint32_t )
