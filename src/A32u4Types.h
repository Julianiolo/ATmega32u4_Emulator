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


#define MCU_INLINE 



#endif