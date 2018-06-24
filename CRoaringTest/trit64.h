
#pragma once

#include "trit.h"

#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE41

typedef __m128i trit64_t;

static inline trit64_t trit64_mask ( int32_t idx_, uint64_t value_ ) {

    return idx_ / 64 ? _mm_set_epi64x ( ( int64_t ) ( value_ << ( idx_ % 64 ) ), 0 ) : _mm_set_epi64x ( 0, ( int64_t ) ( value_ << ( idx_ % 64 ) ) );
}

static inline uint64_t trit64_read ( trit64_t a_, int32_t idx_ ) {

    return ( uint64_t ) ( ( ( uint8_t * ) &a_ ) [ idx_ / 4 ] >> ( ( ( idx_ ) % 4 ) << 1 ) ) & TRIT32_2L;
}

#define t64_high _mm_set1_epi64x ( ( long long ) TRIT32_HB )
#define t64_low _mm_set1_epi64x ( ( long long ) TRIT32_LB )
#define t64_ones _mm_xor_si128 ( _mm_setzero_si128 ( ), _mm_setzero_si128 ( ) )
#define t64_zeros _mm_setzero_si128 ( )
#define t64_mask( i ) trit64_mask ( ( i ), TRIT32_2L )
#define t64_set( i, v ) trit64_mask ( ( i ), ( v ) )
#define t64_left( a ) _mm_slli_si128 ( ( a ), 1 )
#define t64_right( a ) _mm_srli_si128 ( ( a ), 1 )
#define t64_not( a ) _mm_xor_si128 ( ( a ), t64_ones )
#define t64_andnot( a, b ) _mm_andnot_si128 ( ( a ), ( b ) )
#define t64_and( a, b ) _mm_and_si128 ( ( a ), ( b ) )
#define t64_or( a, b ) _mm_or_si128 ( ( a ), ( b ) )
#define t64_xor( a, b ) _mm_xor_si128 ( ( a ), ( b ) )
#define t64_test( a, b ) ( ! _mm_testz_si128 ( ( a ), ( b ) ) )

TRIT_SET ( 64, uint64_t )
