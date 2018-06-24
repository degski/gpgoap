
#pragma once

#include "trit.h"

#include <immintrin.h> // AVX2

typedef __m256i trit128_t;

static inline trit128_t trit128_mask ( int32_t idx_, uint64_t value_ ) {

    switch ( idx_ / 64 ) {

    case  0: return _mm256_set_epi64x ( 0, 0, 0, ( int64_t ) ( value_ << ( idx_ % 64 ) ) );
    case  1: return _mm256_set_epi64x ( 0, 0, ( int64_t ) ( value_ << ( idx_ % 64 ) ), 0 );
    case  2: return _mm256_set_epi64x ( 0, ( int64_t ) ( value_ << ( idx_ % 64 ) ), 0, 0 );
    case  3: return _mm256_set_epi64x ( ( int64_t ) ( value_ << ( idx_ % 64 ) ), 0, 0, 0 );

    default: {

            printf ( "should not happen\n" );
            return _mm256_set_epi64x ( 0, 0, 0, 0 ); // should not happen
        }
    }
}

static inline uint64_t trit128_read ( trit128_t a_, int32_t idx_ ) {

    return ( uint64_t ) ( ( ( uint8_t * ) &a_ ) [ idx_ / 4 ] >> ( ( ( idx_ ) % 4 ) << 1 ) ) & TRIT32_2L;
}

#define t128_high _mm256_set1_epi64x ( ( long long ) TRIT32_HB )
#define t128_low _mm256_set1_epi64x ( ( long long ) TRIT32_LB )
#define t128_ones _mm256_xor_si256 ( _mm256_setzero_si256 ( ), _mm256_setzero_si256 ( ) )
#define t128_zeros _mm256_setzero_si256 ( )
#define t128_mask( i ) trit128_mask ( ( i ), TRIT32_2L )
#define t128_set( i, v ) trit128_mask ( ( i ), ( v ) )
#define t128_left( a ) _mm256_slli_epi64 ( ( a ), 1 )
#define t128_right( a ) _mm256_srli_epi64 ( ( a ), 1 )
#define t128_not( a ) _mm256_xor_si256 ( ( a ), t128_ones )
#define t128_andnot( a, b ) _mm256_andnot_si256 ( ( a ), ( b ) )
#define t128_and( a, b ) _mm256_and_si256 ( ( a ), ( b ) )
#define t128_or( a, b ) _mm256_or_si256 ( ( a ), ( b ) )
#define t128_xor( a, b ) _mm256_xor_si256 ( ( a ), ( b ) )
#define t128_test( a, b ) ( ! _mm256_testz_si256 ( ( a ), ( b ) ) )

TRIT_SET ( 128, uint64_t )


//------------------------------------------------------------------------------------------------------------


static inline __m256i left_shift_000_063 ( __m256i a, int n ) { // 6

    return _mm256_or_si256 ( _mm256_slli_epi64 ( a, n ), _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), _mm256_permute4x64_epi64 ( _mm256_srli_epi64 ( a, 64 - n ), _MM_SHUFFLE ( 2, 1, 0, 0 ) ), _MM_SHUFFLE ( 3, 3, 3, 0 ) ) );
}

static inline __m256i left_shift_064_127 ( __m256i a, int n ) { // 7

    __m256i b = _mm256_slli_epi64 ( a, n );
    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 2, 1, 0, 0 ) );

    __m256i c = _mm256_srli_epi64 ( a, 64 - n );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 1, 0, 0, 0 ) );

    __m256i f = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), d, _MM_SHUFFLE ( 3, 3, 3, 0 ) );
    __m256i g = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), e, _MM_SHUFFLE ( 3, 3, 0, 0 ) ); // 6

    return _mm256_or_si256 ( f, g );
}

static inline __m256i left_shift_128_191 ( __m256i a, int n ) { // 7

    __m256i b = _mm256_slli_epi64 ( a, n );
    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 1, 0, 0, 0 ) );

    __m256i c = _mm256_srli_epi64 ( a, 64 - n );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 1, 0, 0, 0 ) );

    __m256i f = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), d, _MM_SHUFFLE ( 3, 3, 0, 0 ) );
    __m256i g = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), e, _MM_SHUFFLE ( 3, 0, 0, 0 ) );

    return _mm256_or_si256 ( f, g );
}

static inline __m256i left_shift_192_255 ( __m256i a, int n ) { // 5

    return _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), _mm256_slli_epi64 ( _mm256_permute4x64_epi64 ( a, _MM_SHUFFLE ( 0, 0, 0, 0 ) ), n ), _MM_SHUFFLE ( 3, 0, 0, 0 ) );
}

static inline __m256i _mm256_sli_si256 ( __m256i a, int n ) {

    if ( n < 128 ) return n <  64 ? left_shift_000_063 ( a, n ) : left_shift_064_127 ( a, n % 64 );
    else           return n < 192 ? left_shift_128_191 ( a, n % 64 ) : left_shift_192_255 ( a, n % 64 );
}


static inline __m256i right_shift_000_063 ( __m256i a, int n ) { // 6

    return _mm256_or_si256 ( _mm256_srli_epi64 ( a, n ), _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), _mm256_permute4x64_epi64 ( _mm256_slli_epi64 ( a, 64 - n ), _MM_SHUFFLE ( 0, 3, 2, 1 ) ), _MM_SHUFFLE ( 0, 3, 3, 3 ) ) );
}

static inline __m256i right_shift_064_127 ( __m256i a, int n ) { // 7

    __m256i b = _mm256_srli_epi64 ( a, n );
    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 3, 3, 2, 1 ) );

    __m256i c = _mm256_slli_epi64 ( a, 64 - n );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 3, 3, 3, 2 ) );

    __m256i f = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), d, _MM_SHUFFLE ( 0, 3, 3, 3 ) );
    __m256i g = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), e, _MM_SHUFFLE ( 0, 0, 3, 3 ) );

    return _mm256_or_si256 ( f, g );
}

static inline __m256i right_shift_128_191 ( __m256i a, int n ) { // 7

    __m256i b = _mm256_srli_epi64 ( a, n );
    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 3, 2, 3, 2 ) );

    __m256i c = _mm256_slli_epi64 ( a, 64 - n );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 3, 2, 1, 3 ) );

    __m256i f = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), d, _MM_SHUFFLE ( 0, 0, 3, 3 ) );
    __m256i g = _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), e, _MM_SHUFFLE ( 0, 0, 0, 3 ) );

    return _mm256_or_si256 ( f, g );
}

static inline __m256i right_shift_192_255 ( __m256i a, int n ) { // 5

    return _mm256_blend_epi32 ( _mm256_setzero_si256 ( ), _mm256_srli_epi64 ( _mm256_permute4x64_epi64 ( a, _MM_SHUFFLE ( 0, 0, 0, 3 ) ), n ), _MM_SHUFFLE ( 0, 0, 0, 3 ) );
}

static inline __m256i _mm256_sri_si256 ( __m256i a, int n ) {

    if ( n < 128 ) return n <  64 ? right_shift_000_063 ( a, n ) : right_shift_064_127 ( a, n % 64 );
    else           return n < 192 ? right_shift_128_191 ( a, n % 64 ) : right_shift_192_255 ( a, n % 64 );
}


static inline __m256i left_rotate_000_063 ( __m256i a, int n ) { // 5

    return _mm256_or_si256 ( _mm256_slli_epi64 ( a, n ), _mm256_permute4x64_epi64 ( _mm256_srli_epi64 ( a, 64 - n ), _MM_SHUFFLE ( 2, 1, 0, 3 ) ) );
}

static inline __m256i left_rotate_064_127 ( __m256i a, int n ) { // 6

    __m256i b = _mm256_slli_epi64 ( a, n );
    __m256i c = _mm256_srli_epi64 ( a, 64 - n );

    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 2, 1, 0, 3 ) );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 1, 0, 3, 2 ) );

    return _mm256_or_si256 ( d, e );
}

static inline __m256i left_rotate_128_191 ( __m256i a, int n ) { // 6

    __m256i b = _mm256_slli_epi64 ( a, n );
    __m256i c = _mm256_srli_epi64 ( a, 64 - n );

    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 1, 0, 3, 2 ) );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 0, 3, 2, 1 ) );

    return _mm256_or_si256 ( d, e );
}

static inline __m256i left_rotate_192_255 ( __m256i a, int n ) { // 5

    return _mm256_or_si256 ( _mm256_srli_epi64 ( a, 64 - n ), _mm256_permute4x64_epi64 ( _mm256_slli_epi64 ( a, n ), _MM_SHUFFLE ( 0, 3, 2, 1 ) ) );
}

static inline __m256i _mm256_rli_si256 ( __m256i a, int n ) {

    if ( n < 128 ) return n <  64 ? left_rotate_000_063 ( a, n ) : left_rotate_064_127 ( a, n % 64 );
    else           return n < 192 ? left_rotate_128_191 ( a, n % 64 ) : left_rotate_192_255 ( a, n % 64 );
}


static inline __m256i right_rotate_000_063 ( __m256i a, int n ) { // 5

    return _mm256_or_si256 ( _mm256_srli_epi64 ( a, n ), _mm256_permute4x64_epi64 ( _mm256_slli_epi64 ( a, 64 - n ), _MM_SHUFFLE ( 0, 3, 2, 1 ) ) );
}

static inline __m256i right_rotate_064_127 ( __m256i a, int n ) { // 6

    __m256i b = _mm256_srli_epi64 ( a, n );
    __m256i c = _mm256_slli_epi64 ( a, 64 - n );

    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 0, 3, 2, 1 ) );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 1, 0, 3, 2 ) );

    return _mm256_or_si256 ( d, e );
}

static inline __m256i right_rotate_128_191 ( __m256i a, int n ) { // 6

    __m256i b = _mm256_srli_epi64 ( a, n );
    __m256i c = _mm256_slli_epi64 ( a, 64 - n );

    __m256i d = _mm256_permute4x64_epi64 ( b, _MM_SHUFFLE ( 1, 0, 3, 2 ) );
    __m256i e = _mm256_permute4x64_epi64 ( c, _MM_SHUFFLE ( 2, 1, 0, 3 ) );

    return _mm256_or_si256 ( d, e );
}

static inline __m256i right_rotate_192_255 ( __m256i a, int n ) { // 5

    return _mm256_or_si256 ( _mm256_slli_epi64 ( a, 64 - n ), _mm256_permute4x64_epi64 ( _mm256_srli_epi64 ( a, n ), _MM_SHUFFLE ( 2, 1, 0, 3 ) ) );
}

static inline __m256i _mm256_rri_si256 ( __m256i a, int n ) {

    if ( n < 128 ) return n <  64 ? right_rotate_000_063 ( a, n ) : right_rotate_064_127 ( a, n % 64 );
    else           return n < 192 ? right_rotate_128_191 ( a, n % 64 ) : right_rotate_192_255 ( a, n % 64 );
}


static inline __m256i _mm256_mi_si256 ( uint64_t val, int imm ) {

    switch ( imm / 64 ) {

        case  0: return _mm256_set_epi64x ( 0x00, 0x00, 0x00, ( int64_t ) ( val << ( imm % 64 ) ) );
        case  1: return _mm256_set_epi64x ( 0x00, 0x00, ( int64_t ) ( val << ( imm % 64 ) ), 0x00 );
        case  2: return _mm256_set_epi64x ( 0x00, ( int64_t ) ( val << ( imm % 64 ) ), 0x00, 0x00 );
        case  3: return _mm256_set_epi64x ( ( int64_t ) ( val << ( imm % 64 ) ), 0x00, 0x00, 0x00 );

        default: return _mm256_set_epi64x ( 0x00, 0x00, 0x00, 0x00 ); // should not happen
    }
}


//------------------------------------------------------------------------------------------------------------
