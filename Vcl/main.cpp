
// #include <autotimer.hpp>

#include <ciso646>

#include "emmintrin.h"
#include "immintrin.h"

#include <iostream.hpp> // <iostream> + nl, sp etc. defined...


template<int N>
inline __m256i shift_left ( __m256i A ) {

    if constexpr ( N == 0 ) return A;
    else if constexpr ( N <  16 ) return _mm256_alignr_epi8 ( A, _mm256_permute2x128_si256 ( A, A, _MM_SHUFFLE ( 0, 0, 2, 0 ) ), ( uint8_t ) ( 16 - N ) );
    else if constexpr( N == 16 ) return _mm256_permute2x128_si256 ( A, A, _MM_SHUFFLE ( 0, 0, 2, 0 ) );
    else return _mm256_slli_si256 ( _mm256_permute2x128_si256 ( A, A, _MM_SHUFFLE ( 0, 0, 2, 0 ) ), ( uint8_t ) ( N - 16 ) );
}

void print ( const size_t n ) {

    size_t i = 0x8000000000000000;

    while ( i ) {

        putchar ( ( int ) ( ( n & i ) > 0 ) + ( int ) ( 48 ) );
        i >>= 1;
        putchar ( ( int ) ( ( n & i ) > 0 ) + ( int ) ( 48 ) );
        i >>= 1;

        // putchar ( ' ' );
    }
}


void print_256 ( size_t * c );

int main0 ( ) {

    __m256i a = _mm256_set_epi64x ( 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF );
    __m256i m = _mm256_set_epi64x ( 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF );

    __m256i b = _mm256_slli_epi64 ( a, 90 );
    __m256i shifted_out_bits = _mm256_srli_epi64 ( a, 128 - 90 );

    __m256i d = _mm256_permute4x64_epi64 ( shifted_out_bits, _MM_SHUFFLE ( 2, 1, 0, 0 ) );
    __m256i e = _mm256_and_si256 ( d, m );

    __m256i f = _mm256_or_si256 ( b, _mm256_and_si256 ( d, m ) );

    print_256 ( ( size_t * ) &a );

    print_256 ( ( size_t * ) &b );
    print_256 ( ( size_t * ) &shifted_out_bits );

    print_256 ( ( size_t * ) &d );
    print_256 ( ( size_t * ) &e );
    print_256 ( ( size_t * ) &f );

    return 0;
}


void print_256 ( size_t * c ) {

    print ( c [ 3 ] ); print ( c [ 2 ] ); print ( c [ 1 ] ); print ( c [ 0 ] ); putchar ( '\n' );
}


void print_u8 ( __m256i x ) {

    union {

        __m256i v;
        uint8_t s [ 32 ];

    } a;

    a.v = x;

    for ( int c = 0; c < 32; c++ ) {

        printf ( "%2i ", a.s [ 31 - c ] );
    }

    putchar ( '\n' );
}


__m256i init_u8 ( ) {

    union {

        __m256i x;
        char buffer [ 32 ];

    } a;

    for ( int c = 0; c < 32; c++ ) {

        a.buffer [ 31 - c ] = ( char ) c;
    }

    return a.x;
}






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

    if ( n < 128 ) return n <  64 ? right_rotate_000_063 ( a, n      ) : right_rotate_064_127 ( a, n % 64 );
    else           return n < 192 ? right_rotate_128_191 ( a, n % 64 ) : right_rotate_192_255 ( a, n % 64 );
}


static inline __m256i _mm256_mi_si256 ( uint64_t val, int imm ) {

    switch ( imm / 64 ) {

        case  0: return _mm256_set_epi64x ( 0x00, 0x00, 0x00, val << ( imm % 64 ) );
        case  1: return _mm256_set_epi64x ( 0x00, 0x00, val << ( imm % 64 ), 0x00 );
        case  2: return _mm256_set_epi64x ( 0x00, val << ( imm % 64 ), 0x00, 0x00 );
        case  3: return _mm256_set_epi64x ( val << ( imm % 64 ), 0x00, 0x00, 0x00 );

        default: return _mm256_set_epi64x ( 0x00, 0x00, 0x00, 0x00 ); // should not happen
    }
}



struct left_t {

    __m256i perm_l;
    __m256i perm_r;

    __m128i shift_l;
    __m128i shift_r;

    __m256i mask;
};

//  Precompute all the necessary values.

void init_left ( left_t * l, int n ) {

    l->perm_l = _mm256_sub_epi32 ( _mm256_setr_epi32 ( 0, 1, 2, 3, 4, 5, 6, 7 ), _mm256_set1_epi32 ( n / 32 ) );
    l->perm_r = _mm256_sub_epi32 ( l->perm_l, _mm256_set1_epi32 ( 1 ) );

    n %= 32;

    l->shift_l = _mm_cvtsi32_si128 ( n );
    l->shift_r = _mm_cvtsi32_si128 ( 32 - n );

    __m256i maskL = _mm256_cmpgt_epi32 ( _mm256_setzero_si256 ( ), l->perm_l );
    __m256i maskR = _mm256_cmpgt_epi32 ( _mm256_setzero_si256 ( ), l->perm_r );

    l->mask = _mm256_or_si256 ( maskL, _mm256_srl_epi32 ( maskR, l->shift_r ) );
}


__m256i rotate ( left_t * l, __m256i x ) {

    __m256i L = _mm256_permutevar8x32_epi32 ( x, l->perm_l );
    __m256i R = _mm256_permutevar8x32_epi32 ( x, l->perm_r );

    L = _mm256_sll_epi32 ( L, l->shift_l );
    R = _mm256_srl_epi32 ( R, l->shift_r );

    return _mm256_or_si256 ( L, R );
}
__m256i shift ( left_t * l, __m256i x ) {

    return _mm256_andnot_si256 ( l->mask, rotate ( l, x ) );
}



int main ( ) {

    __m256i a = init_u8 ( );

    // __m256i b = right_shift_128_191 ( a, 8 );

    //print_256 ( ( size_t * ) &a );
    //print_256 ( ( size_t * ) &b );

    //__m256i d = left_shift_64 ( c, n % 64 );

//print_256 ( ( size_t * ) &c );

    //print_256 ( ( size_t * ) &e );

    //putchar ( '\n' );
    //print_256 ( ( size_t * ) &e );
    //print_256 ( ( size_t * ) &f );


   for ( int i = 0; i < 256; ++i ) {

       __m256i b = _mm256_rli_si256 ( a, i );

       print_256 ( ( size_t * ) &b );
    }

   for ( int i = 0; i < 256; ++i ) {

       __m256i b = _mm256_sli_si256 ( a, i );

       print_256 ( ( size_t * ) &b );
   }

   for ( int i = 0; i < 256; ++i ) {

       __m256i b = _mm256_rri_si256 ( a, i );

       print_256 ( ( size_t * ) &b );
   }

   for ( int i = 0; i < 256; ++i ) {

       __m256i b = _mm256_sri_si256 ( a, i );

       print_256 ( ( size_t * ) &b );
   }

	return 0;
}














#define READ( a_, idx_ ) case idx_: return ( size_t ) ( _mm256_extract_epi8 ( ( a_ ), ( idx_ ) / 4 ) >> ( ( ( idx_ ) % 4 ) << 1 ) ) & 0x0000000000000003;

static inline size_t trit128_read ( __m256i a_, int32_t idx_ ) {

    switch ( idx_ ) {

        READ ( a_, 0 );
        READ ( a_, 1 );
        READ ( a_, 2 );
        READ ( a_, 3 );
        READ ( a_, 4 );
        READ ( a_, 5 );
        READ ( a_, 6 );
        READ ( a_, 7 );
        READ ( a_, 8 );
        READ ( a_, 9 );
        READ ( a_, 10 );
        READ ( a_, 11 );
        READ ( a_, 12 );
        READ ( a_, 13 );
        READ ( a_, 14 );
        READ ( a_, 15 );
        READ ( a_, 16 );
        READ ( a_, 17 );
        READ ( a_, 18 );
        READ ( a_, 19 );
        READ ( a_, 20 );
        READ ( a_, 21 );
        READ ( a_, 22 );
        READ ( a_, 23 );
        READ ( a_, 24 );
        READ ( a_, 25 );
        READ ( a_, 26 );
        READ ( a_, 27 );
        READ ( a_, 28 );
        READ ( a_, 29 );
        READ ( a_, 30 );
        READ ( a_, 31 );
        READ ( a_, 32 );
        READ ( a_, 33 );
        READ ( a_, 34 );
        READ ( a_, 35 );
        READ ( a_, 36 );
        READ ( a_, 37 );
        READ ( a_, 38 );
        READ ( a_, 39 );
        READ ( a_, 40 );
        READ ( a_, 41 );
        READ ( a_, 42 );
        READ ( a_, 43 );
        READ ( a_, 44 );
        READ ( a_, 45 );
        READ ( a_, 46 );
        READ ( a_, 47 );
        READ ( a_, 48 );
        READ ( a_, 49 );
        READ ( a_, 50 );
        READ ( a_, 51 );
        READ ( a_, 52 );
        READ ( a_, 53 );
        READ ( a_, 54 );
        READ ( a_, 55 );
        READ ( a_, 56 );
        READ ( a_, 57 );
        READ ( a_, 58 );
        READ ( a_, 59 );
        READ ( a_, 60 );
        READ ( a_, 61 );
        READ ( a_, 62 );
        READ ( a_, 63 );
        READ ( a_, 64 );
        READ ( a_, 65 );
        READ ( a_, 66 );
        READ ( a_, 67 );
        READ ( a_, 68 );
        READ ( a_, 69 );
        READ ( a_, 70 );
        READ ( a_, 71 );
        READ ( a_, 72 );
        READ ( a_, 73 );
        READ ( a_, 74 );
        READ ( a_, 75 );
        READ ( a_, 76 );
        READ ( a_, 77 );
        READ ( a_, 78 );
        READ ( a_, 79 );
        READ ( a_, 80 );
        READ ( a_, 81 );
        READ ( a_, 82 );
        READ ( a_, 83 );
        READ ( a_, 84 );
        READ ( a_, 85 );
        READ ( a_, 86 );
        READ ( a_, 87 );
        READ ( a_, 88 );
        READ ( a_, 89 );
        READ ( a_, 90 );
        READ ( a_, 91 );
        READ ( a_, 92 );
        READ ( a_, 93 );
        READ ( a_, 94 );
        READ ( a_, 95 );
        READ ( a_, 96 );
        READ ( a_, 97 );
        READ ( a_, 98 );
        READ ( a_, 99 );
        READ ( a_, 100 );
        READ ( a_, 101 );
        READ ( a_, 102 );
        READ ( a_, 103 );
        READ ( a_, 104 );
        READ ( a_, 105 );
        READ ( a_, 106 );
        READ ( a_, 107 );
        READ ( a_, 108 );
        READ ( a_, 109 );
        READ ( a_, 110 );
        READ ( a_, 111 );
        READ ( a_, 112 );
        READ ( a_, 113 );
        READ ( a_, 114 );
        READ ( a_, 115 );
        READ ( a_, 116 );
        READ ( a_, 117 );
        READ ( a_, 118 );
        READ ( a_, 119 );
        READ ( a_, 120 );
        READ ( a_, 121 );
        READ ( a_, 122 );
        READ ( a_, 123 );
        READ ( a_, 124 );
        READ ( a_, 125 );
        READ ( a_, 126 );
    default:
        READ ( a_, 127 );
    }
}

static inline void trit128_print_bits ( const __m256i a_ ) {

    // for ( int32_t i = 127; i > -1; --i ) {
    for ( int32_t i = 0; i < 128; ++i ) {

        const size_t trit = trit128_read ( a_, i );

        putchar ( ( int ) ( trit >> 1 ) + ( int ) ( 48 ) );
        putchar ( ( int ) ( trit & 0x0000000000000001 ) + ( int ) ( 48 ) );
        putchar ( ' ' );
    }

    putchar ( '\n' );
}
