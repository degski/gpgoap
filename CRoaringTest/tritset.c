
#include "tritset.h"

#include <stdio.h>
#include <string.h>


static inline int32_t tritset_lower_bound_impl ( tritset_t vec, sm_t value, bool ( *less ) ( sm_t a, sm_t b ) ) {

    int32_t count = ( int32_t ) vector_size ( vec );

    if ( 0 == count ) {

        return 0;
    }

    else if ( 1 == count ) {

        return less ( vec [ 0 ], value );
    }

    else if ( 2 == count ) {

        return less ( vec [ 0 ], value ) + less ( vec [ 1 ], value );
    }

    else if ( 3 == count ) {

        return less ( vec [ 0 ], value ) + less ( vec [ 1 ], value ) + less ( vec [ 2 ], value );
    }

    else if ( 4 == count ) {

        return less ( vec [ 0 ], value ) + less ( vec [ 1 ], value ) + less ( vec [ 2 ], value ) + less ( vec [ 3 ], value );
    }

    else if ( 5 == count ) {

        return less ( vec [ 0 ], value ) + less ( vec [ 1 ], value ) + less ( vec [ 2 ], value ) + less ( vec [ 3 ], value ) + less ( vec [ 4 ], value );
    }

    else if ( 6 == count ) {

        return less ( vec [ 0 ], value ) + less ( vec [ 1 ], value ) + less ( vec [ 2 ], value ) + less ( vec [ 3 ], value ) + less ( vec [ 4 ], value ) + less ( vec [ 5 ], value );
    }

    else {

        int32_t step;

        tritset_t it, first = vec;

        while ( count > 0 ) {

            it = first;

            step = count >> 1;

            it += step;

            if ( less ( *it, value ) ) {

                first = ++it;
                count -= step + 1;
            }

            else {

                count = step;
            }
        }

        return ( int32_t ) ( first - vec );
    }
}


static inline bool compare_sm ( sm_t a, sm_t b ) {

    return sm_magnitude ( a ) < sm_magnitude ( b );
}


smu_t tritset_read ( tritset_t vec, int32_t idx ) {

    assert ( idx >= 0 );
    assert ( idx < ( int32_t ) ( 1u << ( ( sizeof ( sm_t ) << 3 ) - 1u ) ) ); // Too high...

    const int32_t k = tritset_lower_bound_impl ( vec, ( sm_t ) idx, compare_sm );

    return k == ( int32_t ) vector_size ( vec ) ? TRIT_INDETERMINATE : ( ( smu_t ) ( TRIT_TRUE - sm_negative ( vec [ k ] ) ) );
}


tritset_t tritset_write ( tritset_t vec, int32_t idx, bool value ) {

    assert ( idx >=  0 );
    assert ( idx < ( int32_t ) ( 1u << ( ( sizeof ( sm_t ) << 3 ) - 1u ) ) ); // idx out of bounds...

    const int32_t size = ( int32_t ) vector_size ( vec );
    const int32_t k = tritset_lower_bound_impl ( vec, ( sm_t ) idx, compare_sm );

    if ( k < size ) { // k is valid idx...

        if ( sm_magnitude ( vec [ k ] ) != idx ) { // Is it not the same magnitude (value) -> grow, insert...

            vector_grow_by ( vec, 1 );
            memmove ( vec + k + 1, vec + k, ( size_t ) ( size - k ) * sizeof ( *( vec ) ) );

        } // else replace...
    }

    else { // k not a valid idx, points to one past the end -> grow, assign...

        vector_grow_by ( vec, 1 );
    }

    vec [ k ] = value ? ( sm_t ) idx : sm_negate ( ( sm_t ) idx );

    return vec;
}


tritset_t tritset_insert ( tritset_t vec, sm_t value ) {

    const sm_t v = sm_magnitude ( value );

    const int32_t size = ( int32_t ) vector_size ( vec );
    const int32_t k = tritset_lower_bound_impl ( vec, v, compare_sm );

    if ( k < size ) { // k is valid idx...

        if ( sm_magnitude ( vec [ k ] ) != v ) { // Is it not the same magnitude (value) -> grow, insert...

            vector_grow_by ( vec, 1 );
            memmove ( vec + k + 1, vec + k, ( size_t ) ( size - k ) * sizeof ( *( vec ) ) );

        } // else replace...
    }

    else { // k not a valid idx, points to one past the end -> grow, assign...

        vector_grow_by ( vec, 1 );
    }

    vec [ k ] = value;

    return vec;
}


tritset_t tritset_delete ( tritset_t vec, sm_t a ) {

    size_t size = vector_size ( vec );

    tritset_t p = vec + tritset_lower_bound_impl ( vec, sm_magnitude ( a ), compare_sm );

    if ( *p == a ) {

        vector_set_size ( vec, size - 1 );
        memmove ( p, p + 1, ( size - ( size_t ) ( ( p + 1 ) - vec ) ) * sizeof ( *( vec ) ) );
    }

    return vec;
}


bool tritset_sub ( tritset_t a_, tritset_t b_ ) {

    const tritset_t la = a_ + vector_size ( a_ ), lb = b_ + vector_size ( b_ );
    tritset_t a = a_, b = b_;

    while ( a < la and b < lb ) {

        a += *a == *b;
        b += 1;
    }

    return a == la;
}


tritset_t tritset_man ( tritset_t a_, tritset_t b_, tritset_t c_ ) { // tritset_t parent, tritset_t ante, tritset_t post

    tritset_t vec = NULL;
    vector_init ( vec, vector_size ( b_ ) );
    memcpy ( vec, b_, vector_size ( b_ ) * sizeof ( sm_t ) );

    tritset_t v = vec, a = a_, la = a_ + vector_size ( a_ ), c = c_, lc = c_ + vector_size ( c_ );

    while ( a < la and c < lc ) {

        if ( *a != *c ) {

            while ( sm_magnitude ( *a ) > *v ) ++v;

            vector_grow_by ( vec, 1u );
            memmove ( v + 1u, v, ( size_t ) ( vec + vector_size ( vec ) - v ) * sizeof ( sm_t ) );
            *v = *a;

        } else {

            ++c;
        }

        ++a;
    }

    for ( ; a < la; ++a ) { // add the rest of parent...

        while ( sm_magnitude ( *a ) > *v ) ++v;

        vector_grow_by ( vec, 1u );
        memmove ( v + 1u, v, ( size_t ) ( vec + vector_size ( vec ) - v ) * sizeof ( sm_t ) );
        *v = *a;
    }

    return vec;
}


bool tritset_equal ( tritset_t a_, tritset_t b_ ) {

    return vector_size ( a_ ) == vector_size ( b_ ) ? memcmp ( a_, b_, vector_size ( a_ ) * sizeof ( smu_t ) ) == 0 : false;
}


static void sm_print ( sm_t v_ ) {

    printf ( "%c%i", sm_negative ( v_ ) ? '-' : '+', ( int32_t ) sm_magnitude ( v_ ) );
}

static void sm_print_ls ( sm_t v_ ) {

    printf ( " %c%i", sm_negative ( v_ ) ? '-' : '+', ( int32_t ) sm_magnitude ( v_ ) );
}

void tritset_print ( tritset_t vec_ ) {

    const int32_t li = ( int32_t ) vector_size ( vec_ );

    if ( li ) {

        sm_print ( vec_ [ 0 ] );

        for ( int32_t i = 1; i < li; ++i ) {

            sm_print_ls ( vec_ [ i ] );
        }

        putchar ( '\n' );
    }
}


// Develop earlier versions...


static bool tritset_sub2 ( tritset_t vec1, tritset_t vec2 ) { // To be improved, see below...

    int32_t v1_size = ( int32_t ) vector_size ( vec1 );

    for ( int32_t i = 0, li = ( int32_t ) vector_size ( vec2 ); i < li; ++i ) {

        int32_t k = tritset_lower_bound_impl ( vec1, sm_magnitude ( vec2 [ i ] ), compare_sm );

        if ( ( k == v1_size ) or ( sm_sign ( vec1 [ k ] ) != sm_sign ( vec2 [ i ] ) ) ) {

            return false;
        }
    }

    return true;
}


static tritset_t tritset_man2 ( tritset_t parent, tritset_t ante, tritset_t post ) {

    const int32_t lpa = ( int32_t ) vector_size ( parent ), lan = ( int32_t ) vector_size ( ante ), lpo = ( int32_t ) vector_size ( post );
    int32_t pa = 0, po = 0;

    tritset_t vec = NULL;
    vector_allocate ( vec );

    while ( pa < lpa and po < lpo ) {

        if ( parent [ pa ] != post [ po ] ) {

            vector_push_back ( vec, parent [ pa ] );
        }

        else {

            ++po;
        }

        ++pa;
    }

    for ( ; pa < lpa; ++pa ) { // add the rest of parent...

        vector_push_back ( vec, parent [ pa ] );
    }

    for ( int32_t an = 0; an < lan; ++an ) {

        vec = tritset_insert ( vec, ante [ an ] );
    }

    return vec;
}
