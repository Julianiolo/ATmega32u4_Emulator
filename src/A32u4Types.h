#ifndef __A32U4TYPES_H__
#define __A32U4TYPES_H__

#include <stdint.h>
#include <cinttypes> // for PRId64 and similar
#include <climits> // for SIZE_MAX

typedef uint8_t regind_t;
typedef uint8_t reg_t;
typedef uint16_t addrmcu_t;
typedef uint16_t pc_t;
typedef uint16_t sizemcu_t;

#define ADDRMCU_T_MAX 0xFFFF

#define MCU_PRIuSIZEMCU PRIu16
#define MCU_PRIxSIZEMCU PRIx16
#define MCU_PRIuADDR PRIu16
#define MCU_PRIxADDR PRIx16
#define MCU_PRIuPC PRIu16
#define MCU_PRIxPC PRIx16

// Print size_t macros
#if SIZE_MAX == 0xffffull
    #define MCU_PRIdSIZE PRId16
    #define MCU_PRIuSIZE PRIu16
    #define MCU_PRIxSIZE PRIx16
#elif SIZE_MAX == 0xffffffffull
    #define MCU_PRIdSIZE PRId32
    #define MCU_PRIuSIZE PRIu32
    #define MCU_PRIxSIZE PRIx32
#elif SIZE_MAX == 0xffffffffffffffffull
    #define MCU_PRIdSIZE PRId64
    #define MCU_PRIuSIZE PRIu64
    #define MCU_PRIxSIZE PRIx64
#else
    #define MCU_PRIdSIZE "d"
    #define MCU_PRIuSIZE "u"
    #define MCU_PRIxSIZE "x"
    #error
#endif


#define MCU_INLINE 


#define MCU_ARR_SIZE(a) (sizeof(a)/sizeof(a[0]))


inline void __assertion_failed__(const char* file, int line) {
    printf("Assertion Failed! %s:%d\n", file, line);
    abort();
}
#ifdef _DEBUG
    #define MCU_ASSERT(x) do {\
        if(!(x)){\
            __assertion_failed__(__FILE__, __LINE__);\
        }\
    } while(0)
#else
    #define MCU_ASSERT(x)
#endif


#ifdef _MSC_VER
    #define MCU_STATIC_ASSERT(x) static_assert(x,"")
#else
    #define MCU_STATIC_ASSERT(x) static_assert(x)
#endif
#define MCU_STATIC_ASSERT_MSG(x,msg) static_assert(x,msg)


#if __cplusplus >= 201703L
    #define MCU_FALLTHROUGH [[fallthrough]]
#else
    #define MCU_FALLTHROUGH // fall through
#endif


#define MCU_UNUSED(x) do { (void)(x); } while(0)

#endif