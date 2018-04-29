#include "util.h"

inline uint8_t dec_to_bcd(uint8_t dec) {
  return ((((dec / 10) & 0xF) << 4) |
           ((dec % 10) & 0xF));
}

inline void i2c_start(void) {
  // Send 'Start' condition, and wait for acknowledge.
  I2C1->CR2 |=  (I2C_CR2_START);
  while ((I2C1->CR2 & I2C_CR2_START)) {}
}

inline void i2c_stop(void) {
  // Send 'Stop' condition, and wait for acknowledge.
  I2C1->CR2 |=  (I2C_CR2_STOP);
  while ((I2C1->CR2 & I2C_CR2_STOP)) {}
  // Reset the ICR ('Interrupt Clear Register') event flag.
  I2C1->ICR |=  (I2C_ICR_STOPCF);
  while ((I2C1->ICR & I2C_ICR_STOPCF)) {}
}

void i2c_write_byte(uint8_t dat) {
  I2C1->TXDR = (I2C1->TXDR & 0xFFFFFF00) | dat;
  // Wait for one of these ISR bits:
  // 'TXIS' ("ready for next byte")
  // 'TC'   ("transfer complete")
  while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_TC))) {}
  // (Also of interest: 'TXE' ("TXDR register is empty") and
  //  'TCR' ("transfer complete, and 'RELOAD' is set."))
}

uint8_t i2c_read_byte(void) {
  // Wait for a byte of data to be available, then read it.
  while (!(I2C1->ISR & I2C_ISR_RXNE)) {}
  return (I2C1->RXDR & 0xFF);
}

uint8_t i2c_read_register(uint8_t reg_addr) {
  // Set '1 byte to send.'
  I2C1->CR2 &= ~(I2C_CR2_NBYTES);
  I2C1->CR2 |=  (0x01 << I2C_CR2_NBYTES_Pos);
  // Start the I2C write transmission.
  i2c_start();
  // Send the register address.
  i2c_write_byte(reg_addr);
  // Stop the I2C write transmission.
  i2c_stop();
  // Set '1 byte to receive.'
  I2C1->CR2 &= ~(I2C_CR2_NBYTES);
  I2C1->CR2 |=  (0x01 << I2C_CR2_NBYTES_Pos);
  // Set 'read' I2C direction.
  I2C1->CR2 |=  (I2C_CR2_RD_WRN);
  // Start the I2C read transmission.
  i2c_start();
  // Read the transmitted data.
  uint8_t read_result = i2c_read_byte();
  // Stop the I2C read transmission.
  i2c_stop();
  // Set 'write' I2C direction again.
  I2C1->CR2 &= ~(I2C_CR2_RD_WRN);

  // Return the read value.
  return read_result;
}
