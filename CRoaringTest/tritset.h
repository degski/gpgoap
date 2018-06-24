
#pragma once

#include "../include/gpgoap/vector.h"

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>


#define TRIT_INDETERMINATE  0u // and 1u
#define TRIT_UNUSED         1u
#define TRIT_FALSE          2u
#define TRIT_TRUE           3u
#define TRIT_MASK           3u
#define TRIT_LB             1u
#define TRIT_HB             2u


typedef uint8_t sm_t;
typedef uint8_t smu_t; // the underlying type of sm_t...
typedef sm_t * tritset_t;


// SM/sm stands for Signed Magnitude...

#define SM_MSB 0x80

static inline bool sm_negative ( sm_t v ) { return v & SM_MSB; }
static inline bool sm_positive ( sm_t v ) { return not ( sm_negative ( v ) ); }
static inline int32_t sm_sign ( sm_t v ) { return ( int32_t ) not ( v & SM_MSB ) * 2 - 1; }
static inline smu_t sm_magnitude ( sm_t v ) { return ( smu_t ) ( v & ~SM_MSB ); }
static inline smu_t sm_abs ( sm_t v ) { return ( smu_t ) ( v & ~SM_MSB ); }
static inline sm_t sm_negate ( sm_t v ) { return v ^ SM_MSB; }


smu_t tritset_read ( tritset_t vec, int32_t idx );
tritset_t tritset_write ( tritset_t vec, int32_t idx, bool value );
tritset_t tritset_insert ( tritset_t vec, sm_t value );
tritset_t tritset_delete ( tritset_t vec, sm_t a );
bool tritset_sub ( tritset_t vec1 , tritset_t vec2 );
tritset_t tritset_man ( tritset_t parent, tritset_t ante, tritset_t post );
bool tritset_equal ( tritset_t a, tritset_t b );
void tritset_print ( tritset_t vec );
