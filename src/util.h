#ifndef _util_h
#define _util_h

/**
 * C's mod ('%') operator is mathematically correct, but it may return
 * a negative remainder even when both inputs are nonnegative.  This
 * function always returns a nonnegative remainder (x mod m), as long
 * as x and m are both positive.  This is helpful for computing
 * toroidal boundary conditions.
 */
static inline int 
mod (int x, int m)
{
  return (x < 0) ? ((x % m) + m) : (x % m);
}

/**
 * Given neighbor count and current state, return zero if cell will be
 * dead, or nonzero if cell will be alive, in the next round.
 */
static inline char 
alivep (char count, char state)
{
  return (! state && (count == (char) 3)) ||
    (state && (count == 2 || count ==3));
}

#define FOREACH(i, lim) for (unsigned i = 0; i < lim; ++i)

// #define ASSERTIONS_ENABLED
#ifdef ASSERTIONS_ENABLED
    #include "stdlib.h"
    #include "stdio.h"
    #define ASSERT(x) if (!(x)) \
    { \
        printf("Assertion failed: %s, %s:%d\n", #x, __FUNCTION__, __LINE__); \
        abort(); \
    }
#else
    #define ASSERT(x)
#endif

#define DEBUGGING_ENABLED
#ifdef DEBUGGING_ENABLED
    #define DEBUG(...) printf(__VA_ARGS__)
#else
    #define DEBUG(...)
#endif

#define TODO(s) \
    printf(s); \
    ASSERT(0)

#endif /* _util_h */
