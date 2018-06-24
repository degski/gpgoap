
#pragma once

#include "../include/gpgoap/vector.h"

#include <assert.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TRIT_INDETERMINATE 0u // and 1u
#define TRIT_UNUSED 1u
#define TRIT_FALSE 2u
#define TRIT_TRUE 3u
#define TRIT_MASK 3u
#define TRIT_LB 1u
#define TRIT_HB 2u

#define TRIT16_00 0x00000000
#define TRIT16_2L 0x00000003
#define TRIT16_HB 0xAAAAAAAA
#define TRIT16_LB 0x55555555
#define TRIT16_MS 0x80000000
#define TRIT16_NT 0xFFFFFFFF
#define TRIT16_FF 0x000000FF

#define TRIT32_00 0x0000000000000000
#define TRIT32_2L 0x0000000000000003
#define TRIT32_HB 0xAAAAAAAAAAAAAAAA
#define TRIT32_LB 0x5555555555555555
#define TRIT32_MS 0x8000000000000000
#define TRIT32_NT 0xFFFFFFFFFFFFFFFF
#define TRIT32_FF 0x00000000000000FF

// If 0, Full Kleene Logic applies, else
// Weak Kleene Logic (Bochvar Logic)...

#define WEAK_KLEENE 1

#define trit_vector_grow(vec, count) \
do { \
\
    size_t size = vector_size ( ( vec ) ); \
\
    if ( ( count ) > size ) { \
\
	    vector_grow ( ( vec ), ( count ) ); \
        vector_set_size ( ( vec ), ( count ) ); \
        memset ( ( void* ) ( ( vec ) + ( size ) ), 0, ( ( count ) - ( size ) ) * sizeof ( *( vec ) ) ); \
    } \
\
} while ( 0 )

#define trit_vector_grow_by(vec, count) \
do { \
\
    size_t size = vector_size ( ( vec ) ); \
\
	vector_grow ( ( vec ), ( size + count ) ); \
    vector_set_size ( ( vec ), ( size + count ) ); \
    memset ( ( void* ) ( ( vec ) + ( size ) ), 0, ( count ) * sizeof ( *( vec ) ); \
\
} while ( 0 )

#define trit_set_active( bits ) \
static inline bool trit##bits##_set_active ( trit##bits##_t * a_, int32_t idx_ ) { \
\
    return ( int32_t ) vector_size ( a_ ) < ( ( idx_ / ( bits - 1 ) ) + 1 ) ? false : ( bool ) ( trit##bits##_read ( a_ [ idx_ / ( bits - 1 ) ], idx_ % ( bits - 1 ) ) & 0x02 ); \
}


#define trit_set_read( bits, type ) \
static inline type trit##bits##_set_read ( trit##bits##_t * a_, int32_t idx_ ) { \
\
    return ( int32_t ) vector_size ( a_ ) < ( ( idx_ / ( bits - 1 ) ) + 1 ) ? 0x00 : trit##bits##_read ( a_ [ idx_ / ( bits - 1 ) ], idx_ % ( bits - 1 ) ); \
}


#define trit_set_write( bits, type ) \
static inline trit##bits##_t * trit##bits##_set_write ( trit##bits##_t * a_, int32_t idx_, type value_ ) { \
\
    int32_t i = idx_ / bits; \
    trit##bits##_t * a = a_; \
\
    if ( a_ == NULL ) { \
\
        vector_init ( a, i + 1 ); \
    } \
\
    else if ( ( int32_t ) vector_size ( a ) < i + 1 ) { \
\
        trit_vector_grow ( a, ( size_t ) ( i + 1 ) ); \
    } \
\
    a [ i ] = trit##bits##_write ( a [ i ], idx_ % bits, value_ ); \
\
    assert ( ( int32_t ) vector_size ( a ) > i ); \
\
    return a; \
}

// Distinguishes +0.0 from -0.0...
//
// All integers with 6 or fewer significant decimal digits,
// and any number that can be written as 2n such that n is
// a whole number from - 126 to 127, can be converted into
// an IEEE 754 floating - point value without loss of
// precision.
//
// The above limits the implementation to 1 million - 1 active trits.

#define trit_is_float_positive( a ) \
(!(*(( uint32_t * ) &(a) ) & 0x80000000 ))

#define trit_is_float_negative( a ) \
(!trit_is_float_positive( ( a ) ))


#define trit_set_indexed( bits, type ) \
static inline float * trit##bits##_set_indexed ( trit##bits##_t * a_ ) { \
\
    int32_t size = ( int32_t ) vector_size ( a_ ) * ( int32_t ) bits; \
\
    float * indices; \
\
    vector_allocate ( indices ); \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        type t = trit##bits##_set_read ( a_, i ); \
\
        if ( t & TRIT_HB ) { \
\
            if ( t & TRIT_LB ) { \
\
                vector_push_back ( indices,    ( float ) i   ); \
            } \
\
            else { \
\
                vector_push_back ( indices, -( ( float ) i ) ); \
            } \
        } \
    } \
\
    return indices; \
}

#define trit_set_neg_inplace( bits ) \
static inline void trit##bits##_set_neg_inplace ( trit##bits##_t * a_ ) { \
\
    size_t size = vector_size ( a_ ); \
\
    for ( size_t i = 0; i < size; ++i ) { \
\
        a_ [ i ] = trit##bits##_neg ( a_ [ i ] ); \
    } \
}

#define trit_set_neg( bits ) \
static inline trit##bits##_t * trit##bits##_set_neg ( trit##bits##_t * a_ ) { \
\
    size_t size = vector_size ( a_ ); \
\
    trit##bits##_t * v = NULL; \
    vector_init ( v, size ); \
\
    for ( size_t i = 0; i < size; ++i ) { \
\
        v [ 0 ] = trit##bits##_neg ( a_ [ i ] ); \
    } \
\
    return v; \
}

#if WEAK_KLEENE

#define TRIT_SET_UNARY( bits, op ) \
static inline trit##bits##_t * trit##bits##_set##op ( trit##bits##_t * a_ ) { \
\
    int32_t size = ( int32_t ) vector_size ( a_ ); \
\
    trit##bits##_t * v = NULL; \
    vector_init ( v, size ); \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        v [ i ] = trit##bits##op ( a_ [ i ] ); \
    } \
\
    return v; \
}

#define TRIT_SET_BINARY( bits, op ) \
static inline trit##bits##_t * trit##bits##_set##op ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    int32_t size = ( int32_t ) min ( vector_size ( a_ ), vector_size ( b_ ) ); \
\
    trit##bits##_t * v = NULL; \
    vector_init ( v, size ); \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        v [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
    } \
\
    return v; \
}

#define TRIT_SET_TERNARY( bits, op ) \
static inline trit##bits##_t * trit##bits##_set##op ( trit##bits##_t * a_, trit##bits##_t * b_, trit##bits##_t * c_ ) { \
\
    int32_t size = ( int32_t ) min ( vector_size ( a_ ), vector_size ( b_ ) ); \
\
    trit##bits##_t * v = NULL; \
    vector_init ( v, size ); \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        v [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ], c_ [ i ] ); \
    } \
\
    return v; \
}

#define TRIT_SET_BINARY_INPLACE( bits, op ) \
static inline void trit##bits##_set##op##_inplace ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    int32_t size = ( int32_t ) min ( vector_size ( a_ ), vector_size ( b_ ) ); \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        a_ [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
    } \
}

#else

#define TRIT_SET_BINARY( bits, op ) \
static inline trit##bits##_t * trit##bits##_set##op ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    trit##bits##_t * v = NULL; \
\
    int32_t size_a = ( int32_t ) vector_size ( a_ ), size_b = ( int32_t ) vector_size ( b_ ); \
\
    if ( size_a == size_b ) { \
\
        vector_init ( v, size_a ); \
\
        for ( int32_t i = 0; i < size_a; ++i ) { \
\
            v [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
\
    else if ( size_a < size_b ) { \
\
        trit_vector_grow ( a_, size_b ); \
        vector_init ( v, size_b ); \
\
        for ( int32_t i = 0; i < size_b; ++i ) { \
\
            v [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
\
    else { \
\
        trit_vector_grow ( b_, size_a ); \
        vector_init ( v, size_a ); \
\
        for ( int32_t i = 0; i < size_a; ++i ) { \
\
            v [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
\
    return v; \
}

#define TRIT_SET_BINARY_INPLACE( bits, op ) \
static inline void trit##bits##_set##op##_inplace ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    trit##bits##_t * v = NULL; \
\
    int32_t size_a = ( int32_t ) vector_size ( a_ ), size_b = ( int32_t ) vector_size ( b_ ); \
\
    if ( size_a == size_b ) { \
\
        for ( int32_t i = 0; i < size_a; ++i ) { \
\
            a_ [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
\
    else if ( size_a < size_b ) { \
\
        trit_vector_grow ( a_, size_b ); \
\
        for ( int32_t i = 0; i < size_b; ++i ) { \
\
            a_ [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
\
    else { \
\
        trit_vector_grow ( b_, size_a ); \
\
        for ( int32_t i = 0; i < size_a; ++i ) { \
\
            a_ [ i ] = trit##bits##op ( a_ [ i ], b_ [ i ] ); \
        } \
    } \
}

#endif

#define trit_set_sub( bits ) \
static inline bool trit##bits##_set_sub ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    int32_t size = ( int32_t ) min ( vector_size ( a_ ), vector_size ( b_ ) ); \
\
    int32_t b = 0; \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        if ( ! ( trit##bits##_sub ( a_ [ i ], b_ [ i ] ) ) ) { \
\
            ++b; \
        } \
    } \
\
    return b == 0; \
}

#define trit_set_test( bits ) \
static inline bool trit##bits##_set_test ( trit##bits##_t * a_, trit##bits##_t * b_ ) { \
\
    int32_t size = ( int32_t ) vector_size ( a_ ); \
\
    if ( ( int32_t ) vector_size ( b_ ) != size ) return false; \
\
    int32_t b = 0; \
\
    for ( int32_t i = 0; i < size; ++i ) { \
\
        if ( not ( t##bits##_test ( a_ [ i ], b_ [ i ] ) ) ) { \
\
            ++b; \
        } \
    } \
\
    return b == 0; \
}

#define trit_set_print_bits( bits ) \
static inline void trit##bits##_set_print_bits ( trit##bits##_t * a_ ) { \
\
    int32_t size = ( int32_t ) vector_size ( a_ ) - 1; \
\
    for ( int32_t j = size; j > -1; --j ) { \
\
        trit##bits##_print_bits ( a_ [ j ] ); \
    } \
\
    putchar ( '\n' ); \
}

#define trit_set_print( bits ) \
static inline void trit##bits##_set_print ( trit##bits##_t * a_ ) { \
\
    int32_t size = ( int32_t ) vector_size ( a_ ) - 1; \
\
    for ( int32_t j = size; j > -1; --j ) { \
\
        trit##bits##_print ( a_ [ j ] ); \
    } \
\
    putchar ( '\n' ); \
}

// t##bits##_mask ( ( idx_ *= 2 ) )
#define trit_write( bits, type ) \
static inline trit##bits##_t trit##bits##_write ( trit##bits##_t a_, int32_t idx_, type value_ ) { \
\
    trit##bits##_t b = t##bits##_andnot ( t##bits##_mask ( ( idx_ *= 2 ) ), a_ ); \
\
    return value_ > ( type ) 1 ? t##bits##_or ( b, t##bits##_set ( idx_, value_ ) ) : b; \
}

#define trit_neg( bits, type ) \
static inline trit##bits##_t trit##bits##_neg ( trit##bits##_t a_ ) { \
\
    return t##bits##_xor ( a_, t##bits##_and ( t##bits##_right ( a_ ), t##bits##_low ) ); \
}

#if WEAK_KLEENE

#define trit_min( bits, type ) \
static inline trit##bits##_t trit##bits##_min ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    return t##bits##_and ( a_, b_ ); \
}

#define trit_max( bits, type ) \
static inline trit##bits##_t trit##bits##_max ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    return t##bits##_or ( a_, b_ ); \
}

#else

#define trit_min( bits, type ) \
static inline trit##bits##_t trit##bits##_min ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    trit##bits##_t a_xor_b = t##bits##_xor ( a_, b_ ); \
\
    return t##bits##_or ( t##bits##_and ( a_, b_ ), t##bits##_and ( t##bits##_xor ( a_xor_b, t##bits##_left ( t##bits##_and ( a_xor_b, t##bits##_low ) ) ), t##bits##_and ( a_xor_b, t##bits##_high ) ) ); \
}

#define trit_max( bits, type ) \
static inline trit##bits##_t trit##bits##_max ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    trit##bits##_t a_xor_b = t##bits##_xor ( a_, b_ ); \
\
    return t##bits##_xor ( t##bits##_or ( a_, b_ ), t##bits##_and ( t##bits##_xor ( a_xor_b, t##bits##_left ( t##bits##_and ( a_xor_b, t##bits##_low ) ) ), t##bits##_and ( a_xor_b, t##bits##_high ) ) ); \
}

#endif


#define trit_imp( bits, type ) \
static inline trit##bits##_t trit##bits##_imp ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    return trit##bits##_max ( trit##bits##_neg ( a_ ), b_ ); \
}

#define trit_xor( bits, type ) \
static inline trit##bits##_t trit##bits##_xor ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    trit##bits##_t msk = t##bits##_and ( t##bits##_and ( a_, b_ ), t##bits##_high ); \
\
    return t##bits##_or ( t##bits##_and ( t##bits##_xor ( a_, b_ ), t##bits##_right ( msk ) ), msk ); \
}

#define trit_equ( bits, type ) \
static inline trit##bits##_t trit##bits##_equ ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    trit##bits##_t msk = t##bits##_and ( t##bits##_and ( a_, b_ ), t##bits##_high ); \
\
    return t##bits##_or ( t##bits##_andnot ( t##bits##_xor ( a_, b_ ), t##bits##_right ( msk ) ), msk ); \
}

#define trit_msk( bits ) \
static inline trit##bits##_t trit##bits##_msk ( trit##bits##_t a_ ) { \
\
    trit##bits##_t msk = t##bits##_and ( a_, t##bits##_high ); \
\
    return t##bits##_or ( msk, t##bits##_right ( msk ) ); \
}

// Is b a sub-set of several a?.. The same as below, but passing in the mask
// to be used for checking several values against the same b...

#define trit_sub_msk( bits ) \
static inline bool trit##bits##_sub_msk ( trit##bits##_t a_, trit##bits##_t msk_ ) { \
\
    return t##bits##_test ( t##bits##_and ( a_, msk_ ), msk_ ); \
}

// Is b a sub-set of a?..

#define trit_sub( bits ) \
static inline bool trit##bits##_sub ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    return t##bits##_test ( t##bits##_and ( a_, trit##bits##_msk ( b_ ) ), b_ ); \
}

// Set trits in a to active trits in b...

#define trit_set( bits ) \
static inline trit##bits##_t trit##bits##_set ( trit##bits##_t a_, trit##bits##_t b_ ) { \
\
    return t##bits##_or ( t##bits##_andnot ( trit##bits##_msk ( b_ ), a_ ), b_ ); \
}

#define trit_max_andnot( bits ) \
static inline trit##bits##_t trit##bits##_max_andnot ( trit##bits##_t a_, trit##bits##_t b_, trit##bits##_t c_ ) { \
\
    return t##bits##_or ( b_, t##bits##_andnot ( c_, a_ ) ); \
}

#define trit_print_bits( bits, type ) \
static inline void trit##bits##_print_bits ( trit##bits##_t a_ ) { \
\
    for ( int32_t i = ( bits - 1 ); i > -1; --i ) { \
\
        type trit = trit##bits##_read ( a_, i ); \
\
        putchar ( ( int ) ( trit >> 1 ) + ( int ) ( 48 ) ); \
        putchar ( ( int ) ( trit & ( type ) 1 ) + ( int ) ( 48 ) ); \
        putchar ( ' ' ); \
    } \
}

#define trit_print( bits, type ) \
static inline void trit##bits##_print ( trit##bits##_t a_ ) { \
\
    for ( int32_t i = ( bits - 1 ); i > -1; --i ) { \
\
        type trit = trit##bits##_read ( a_, i ); \
\
        if ( trit >> 1 ) { \
\
            putchar ( ( int ) ( trit & ( type ) 1 ) + ( int ) ( 48 ) ); \
        } \
\
        else { \
\
            putchar ( '.' ); \
        } \
    } \
}


#define TRIT_SET( bits, type ) \
\
    trit_write ( bits, type ) \
    trit_neg ( bits, type ) \
    trit_min ( bits, type ) \
    trit_max ( bits, type ) \
    trit_imp ( bits, type ) \
    trit_xor ( bits, type ) \
    trit_equ ( bits, type ) \
    trit_msk( bits ) \
    trit_sub_msk( bits ) \
    trit_sub( bits ) \
    trit_set ( bits ) \
    trit_max_andnot ( bits ) \
    trit_print_bits ( bits, type ) \
    trit_print ( bits, type ) \
\
    trit_set_read ( bits, type ) \
    trit_set_write ( bits, type ) \
\
    trit_set_active ( bits ) \
    trit_set_indexed ( bits, type ) \
\
    trit_set_neg_inplace ( bits ) \
    trit_set_neg ( bits ) \
\
    TRIT_SET_BINARY ( bits, _min ) \
    TRIT_SET_BINARY ( bits, _max ) \
    TRIT_SET_BINARY ( bits, _imp ) \
    TRIT_SET_BINARY ( bits, _xor ) \
    TRIT_SET_BINARY ( bits, _equ ) \
    trit_set_sub ( bits ) \
    trit_set_test ( bits ) \
    TRIT_SET_BINARY ( bits, _set ) \
\
    TRIT_SET_TERNARY ( bits, _max_andnot ) \
\
    TRIT_SET_BINARY_INPLACE ( bits, _min ) \
    TRIT_SET_BINARY_INPLACE ( bits, _max ) \
    TRIT_SET_BINARY_INPLACE ( bits, _imp ) \
    TRIT_SET_BINARY_INPLACE ( bits, _xor ) \
    TRIT_SET_BINARY_INPLACE ( bits, _equ ) \
\
    trit_set_print_bits ( bits ) \
    trit_set_print ( bits )
