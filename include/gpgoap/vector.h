
#pragma once

#include <stddef.h> /* for size_t */
#include <stdlib.h> /* for malloc/realloc/free */
#include <string.h>
#include <assert.h> /* for assert */

/**
 * brief vector_set_capacity - For internal use, sets the capacity variable of the vector
 * param vec - the vector
 * param size - the new capacity to set
 * return void
 */
#define vector_set_capacity(vec, size)   \
do {                                     \
	if(vec) {                            \
		((size_t *)(vec))[-1] = (size_t) (size);  \
	}                                    \
} while(0)

/**
 * brief vector_set_size - For internal use, sets the size variable of the vector
 * param vec - the vector
 * param size - the new capacity to set
 * return void
 */
#define vector_set_size(vec, size)      \
do {                                    \
	if(vec) {                           \
		((size_t *)(vec))[-2] = (size_t)(size); \
	}                                   \
} while(0)

/**
 * brief vector_capacity - gets the current capacity of the vector
 * param vec - the vector
 * return the capacity as a size_t
 */
#define vector_capacity(vec) \
	((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

/**
 * brief vector_size - gets the current size of the vector
 * param vec - the vector
 * return the size as a size_t
 */
#define vector_size(vec) \
	((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

/**
 * brief vector_empty - returns non-zero if the vector is empty
 * param vec - the vector
 * return non-zero if empty, zero if non-empty
 */
#define vector_empty(vec) \
	(vector_size(vec) == 0)


#define vector_clear(vec) \
do {                                    \
	if(vec) {                           \
		((size_t *)(vec))[-2] = 0;      \
	}                                   \
} while(0)


/**
 * brief vector_grow - For internal use, ensures that the vector is at least <count> elements big
 * param vec - the vector
 * param size - the new capacity to set
 * return void
 */
#define vector_grow(vec, count)                                                                     \
do {                                                                                                \
	if(!(vec)) {                                                                                    \
		size_t *__p = (size_t*) malloc ((size_t)(count) * sizeof(*(vec)) + (sizeof(size_t) * 2));   \
		assert(__p);                                                                                \
		(vec) = (void *)(&__p[2]);                                                                  \
		vector_set_capacity((vec), (count));                                                        \
		vector_set_size((vec), 0);                                                                  \
	} else {                                                                                        \
		size_t *__p1 = &((size_t *)(vec))[-2];                                                      \
		size_t *__p2 = realloc(__p1, ((size_t)(count) * sizeof(*(vec))+ (sizeof(size_t) * 2)));     \
		assert(__p2);                                                                               \
		(vec) = (void *)(&__p2[2]);                                                                 \
		vector_set_capacity((vec), (count));                                                        \
	}                                                                                               \
} while(0)

#define vector_grow_by(vec, count) \
do { \
\
    size_t __vector_grow_by_size = vector_size ( ( vec ) ) + ( count ); \
\
\
	    vector_grow ( ( vec ), __vector_grow_by_size ); \
        vector_set_size ( ( vec ), __vector_grow_by_size ); \
\
} while ( 0 )

#define vector_grow_zero(vec, count) \
do {                                                                                    \
	if(!(vec)) {                                                                        \
		size_t *__p = (size_t*) calloc ((size_t)(count), sizeof(*(vec)) + (sizeof(size_t) * 2));\
		assert(__p);                                                                    \
		(vec) = (void *)(&__p[2]);                                                      \
		vector_set_capacity((vec), (count));                                            \
		vector_set_size((vec), 0);                                                      \
	} else {                                                                            \
		size_t *__p1 = &((size_t *)(vec))[-2];                                          \
		size_t *__p2 = realloc(__p1, ((size_t)(count) * sizeof(*(vec))+ (sizeof(size_t) * 2))); \
		assert(__p2);                                                                   \
		(vec) = (void *)(&__p2[2]);                                                     \
		vector_set_capacity((vec), (count));                                            \
        memset((void*)((vec)+vector_size(vec)), 0, ((count)-vector_size(vec))*sizeof(*(vec))); \
	}                                                                                   \
} while(0)

#define vector_grow_zero_by(vec, count) \
do { \
\
    size_t __vector_grow_zero_by_size = vector_size ( ( vec ) ) + ( count ); \
\
    if ( ( size_t ) ( count ) > vector_capacity ( ( vec ) ) ) { \
\
	    vector_grow_zero ( ( vec ), __vector_grow_zero_by_size ); \
        vector_set_size ( ( vec ), __vector_grow_zero_by_size ); \
    } \
\
} while ( 0 )

#define vector_init(vec, count) \
do { \
    (vec) = NULL; \
	vector_grow((vec), (count)); \
    vector_set_size((vec), (count)); \
    memset((void*)(vec), 0, (size_t)(count)*sizeof(*(vec))); \
} while(0)

#define vector_allocate(vec) \
do { \
    (vec) = NULL; \
	vector_grow((vec), 1); \
    vector_set_size((vec), (0)); \
    memset((void*)(vec), 0, sizeof(*(vec))); \
} while(0)



/**
 * brief vector_pop_back - removes the last element from the vector
 * param vec - the vector
 * return void
 */
#define vector_pop_back(vec) \
do {                                              \
	vector_set_size((vec), vector_size(vec) - 1); \
} while(0)

/**
 * brief vector_free - frees all memory associated with the vector
 * param vec - the vector
 * return void
 */
#define vector_free(vec) \
do { \
	if(vec) {                                \
		size_t *p1 = &((size_t *)(vec))[-2]; \
		free(p1);                            \
	}                                        \
} while(0)

/**
 * brief vector_begin - returns an iterator to first element of the vector
 * param vec - the vector
 * return a pointer to the first element
 */
#define vector_begin(vec) \
	(vec)

/**
 * brief vector_end - returns an iterator to one past the last element of the vector
 * param vec - the vector
 * return a pointer to one past the last element
 */
#define vector_end(vec) \
	&((vec)[vector_size(vec)])

 /**
 * brief vector_end - returns an iterator to the last element of the vector
 * param vec - the vector
 * return a pointer to the last element
 */
#define vector_back(vec) \
	((vec)[vector_size(vec)-1])

/**
 * brief vector_push_back - adds an element to the end of the vector
 * param vec - the vector
 * param value - the value to add
 * return void
 */

#define vector_push_back(vec, value) \
do {                                                        \
	size_t __cap = vector_capacity(vec);                    \
	if(__cap <= vector_size(vec)) {                         \
		vector_grow((vec), !__cap ? __cap + 1 : __cap * 2); \
	}                                                       \
	vec[vector_size(vec)] = (value);                        \
	vector_set_size((vec), vector_size(vec) + 1);           \
} while(0)


#define vector_emplace_back(vec, func) \
do {                                                        \
	size_t __cap = vector_capacity(vec);                    \
	if(__cap <= vector_size(vec)) {                         \
		vector_grow((vec), !__cap ? __cap + 1 : __cap * 2); \
	}                                                       \
	func( ((vec) + vector_size(vec)) );                       \
	vector_set_size((vec), vector_size(vec) + 1);           \
} while(0)


#define vector_push_back_default(vec) \
do {                                                        \
	size_t __cap = vector_capacity(vec);                    \
	if(__cap <= vector_size(vec)) {                         \
		vector_grow((vec), !__cap ? __cap + 1 : __cap * 2); \
	}                                                       \
	vector_set_size((vec), vector_size(vec) + 1);           \
} while(0)


#define vector_reverse(vec) \
do { \
    size_t size = vector_size((vec)) - 1; \
    char buf [ sizeof(*(vec)) ];\
\
    for ( size_t i = size ; i > size / 2; i-- ) { \
\
        if ( i != ( size - i ) ) { \
\
            memcpy ( buf, (vec) + i, sizeof(*(vec)) ); \
            *( ( vec ) + i ) = *( (vec) + ( size - i ) ); \
            memcpy ( (vec) + ( size - i ), buf, sizeof(*(vec)) ); \
        } \
        else { \
            break; \
        } \
    } \
} while(0)
