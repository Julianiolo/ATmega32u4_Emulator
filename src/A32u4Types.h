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



#ifdef _MSC_VER
    #define MCU_INLINE
#else
    #define MCU_INLINE
#endif


#ifdef _DEBUG
    #define MCU_ASSERT(x) do {\
        if(!(x)){\
            printf("Assertion Failed! %s:%d\n", __FILE__, __LINE__);\
            abort();\
        }\
    } while(0)
#else
    #define MCU_ASSERT(x)
    //#error
#endif
#ifdef _MSC_VER
    #define MCU_STATIC_ASSERT(x) static_assert(x,"")
#else
    #define MCU_STATIC_ASSERT(x) static_assert(x)
#endif
    #define MCU_STATIC_ASSERT_MSG(x,msg) static_assert(x,msg)

#define MCU_FALLTHROUGH [[fallthrough]]
#define MCU_UNUSED(x) do { (void)(x); } while(0)

#endif