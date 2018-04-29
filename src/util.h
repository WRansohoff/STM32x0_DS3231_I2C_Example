#ifndef _VVC_UTIL_H
#define _VVC_UTIL_H

#include <stdint.h>
#ifdef VVC_F0
  #include "stm32f031x6.h"
#elif  VVC_L0
  #include "stm32l031xx.h"
#endif

inline uint8_t dec_to_bcd(uint8_t dec);
inline void i2c_start(void);
inline void i2c_stop(void);
void    i2c_write_byte(uint8_t dat);
uint8_t i2c_read_byte(void);
uint8_t i2c_read_register(uint8_t reg_addr);

#endif
