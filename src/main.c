#include "main.h"

/**
 * Main program.
 */
int main(void) {
  // Enable the I2C1 peripheral in 'RCC_APB1ENR'.
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  #ifdef VVC_F0
    // Enable the GPIOB peripheral in 'RCC_AHBENR'.
    RCC->AHBENR   |= RCC_AHBENR_GPIOBEN;
  #elif  VVC_L0
    RCC->IOPENR   |= RCC_IOPENR_IOPBEN;
  #endif
  // Initialize the GPIOB pins.
  // For I2C1, use AF1.
  GPIOB->AFR[SCL_PIN/8] &= ~(0xF << (SCL_PIN*4));
  GPIOB->AFR[SCL_PIN/8] |=  (0x1 << (SCL_PIN*4));
  GPIOB->AFR[SDA_PIN/8] &= ~(0xF << (SDA_PIN*4));
  GPIOB->AFR[SDA_PIN/8] |=  (0x1 << (SDA_PIN*4));
  // B6/7 should be set to 'alt' mode, open-drain, with pull-up.
  GPIOB->MODER  &= ~(0x3 << (SCL_PIN*2));
  GPIOB->MODER  |=  (0x2 << (SCL_PIN*2));
  GPIOB->PUPDR  &= ~(0x3 << (SCL_PIN*2));
  GPIOB->PUPDR  |=  (0x1 << (SCL_PIN*2));
  GPIOB->OTYPER |=  (0x1 << SCL_PIN);
  GPIOB->MODER  &= ~(0x3 << (SDA_PIN*2));
  GPIOB->MODER  |=  (0x2 << (SDA_PIN*2));
  GPIOB->PUPDR  &= ~(0x3 << (SDA_PIN*2));
  GPIOB->PUPDR  |=  (0x1 << (SDA_PIN*2));
  GPIOB->OTYPER |=  (0x1 << SDA_PIN);
  // B3 is connected to an LED on the 'Nucleo' board.
  //    It should be set to push-pull low-speed output.
  GPIOB->MODER  &= ~(0x3 << (LED_PIN*2));
  GPIOB->MODER  |=  (0x1 << (LED_PIN*2));
  GPIOB->OTYPER &= ~(1 << LED_PIN);
  GPIOB->PUPDR  &= ~(0x3 << (LED_PIN*2));

  // Initialize the I2C1 peripheral.
  // First, disable the peripheral.
  I2C1->CR1     &= ~(I2C_CR1_PE);
  // Clear some 'CR1' bits.
  I2C1->CR1     &= ~( I2C_CR1_DNF    |
                      I2C_CR1_ANFOFF |
                      I2C_CR1_SMBHEN |
                      I2C_CR1_SMBDEN );
  // Clear some 'CR2' bits.
  I2C1->CR2     &= ~( I2C_CR2_RD_WRN  |
                      I2C_CR2_NACK    |
                      I2C_CR2_RELOAD  |
                      I2C_CR2_AUTOEND );
  // Clear all 'ICR' flags.
  I2C1->ICR     |=  ( I2C_ICR_ADDRCF   |
                      I2C_ICR_NACKCF   |
                      I2C_ICR_STOPCF   |
                      I2C_ICR_BERRCF   |
                      I2C_ICR_ARLOCF   |
                      I2C_ICR_OVRCF    |
                      I2C_ICR_PECCF    |
                      I2C_ICR_TIMOUTCF |
                      I2C_ICR_ALERTCF  );
  // Configure I2C timing.
  // Reset all but the reserved bits.
  I2C1->TIMINGR &=  (0x0F000000);
  // (100KHz @48MHz core clock, according to an application note)
  I2C1->TIMINGR |=  (0xB0420F13);
  // Enable the peripheral. (PE = 'Peripheral Enable')
  I2C1->CR1     |=  I2C_CR1_PE;

  // Set the DS3231's I2C address.
  I2C1->CR2     &= ~(I2C_CR2_SADD);
  I2C1->CR2     |=  (0xD0 << I2C_CR2_SADD_Pos);
  // Read the 'Control/Status' register. (Offset 0x0F)
  uint8_t ds3231_status = i2c_read_register(0x0F);
  // Ensure that the 'oscillator stop flag' is set to 0. (bit 7)
  // (If it was set to 1, that means that power was removed
  //  from the chip since it was last turned on, so the
  //  time is probably invalid.)
  ds3231_status &= 0x7F;
  // Write two bytes; the register offset, and its value.
  I2C1->CR2 &= ~(I2C_CR2_NBYTES);
  I2C1->CR2 |=  (0x02 << I2C_CR2_NBYTES_Pos);
  i2c_start();
  i2c_write_byte(0x0F);
  i2c_write_byte(ds3231_status);
  i2c_stop();

  // Optional: If this is the first time using a clock,
  // set an initial time. (Ideally the user could do this)
  //#define VVC_SET_INITIAL_TIME 1
  #undef VVC_SET_INITIAL_TIME
  #ifdef VVC_SET_INITIAL_TIME
    const uint8_t seconds = 0;
    const uint8_t minutes = 30;
    const uint8_t hours = 14;
    const uint8_t day = 29;
    const uint8_t month = 4;
    const uint8_t year = 18;
    const uint8_t weekday = 7;
    // We'll write 7 registers; one for each of the above.
    // (Plus 1 for the initial register offset, 0x00).
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);
    I2C1->CR2 |=  (0x08 << I2C_CR2_NBYTES_Pos);
    i2c_start();
    i2c_write_byte(0x00);
    // The registers are written and read in 'BCD' format.
    i2c_write_byte(dec_to_bcd(seconds));
    i2c_write_byte(dec_to_bcd(minutes));
    i2c_write_byte(dec_to_bcd(hours) | 0x40);
    i2c_write_byte(weekday);
    i2c_write_byte(dec_to_bcd(day));
    i2c_write_byte(dec_to_bcd(month));
    i2c_write_byte(dec_to_bcd(year));
    i2c_stop();
  #endif

  // Optional: Write some test data to the EEPROM.
  //#define VVC_SET_EEPROM 1
  #undef VVC_SET_EEPROM
  #ifdef VVC_SET_EEPROM
  // Set the AT24C32's I2C address.
  I2C1->CR2     &= ~(I2C_CR2_SADD);
  I2C1->CR2     |=  (0xAE << I2C_CR2_SADD_Pos);
  // Write 16 bytes total; 2 address, plus the string.
  I2C1->CR2 &= ~(I2C_CR2_NBYTES);
  I2C1->CR2 |=  (16 << I2C_CR2_NBYTES_Pos);
  // Write to address 0.
  i2c_start();
  i2c_write_byte(0x00);
  i2c_write_byte(0x00);
  // Write 'Hello, world!'
  i2c_write_byte('H');
  i2c_write_byte('e');
  i2c_write_byte('l');
  i2c_write_byte('l');
  i2c_write_byte('o');
  i2c_write_byte(',');
  i2c_write_byte(' ');
  i2c_write_byte('w');
  i2c_write_byte('o');
  i2c_write_byte('r');
  i2c_write_byte('l');
  i2c_write_byte('d');
  i2c_write_byte('!');
  i2c_write_byte('\0');
  i2c_stop();
  // Set the DS3231's I2C address again.
  I2C1->CR2     &= ~(I2C_CR2_SADD);
  I2C1->CR2     |=  (0xD0 << I2C_CR2_SADD_Pos);
  #endif

  // Main loop.
  uint8_t cur_s = 0;
  uint8_t cur_m = 0;
  uint8_t cur_h = 0;
  uint8_t cur_D = 0;
  uint8_t cur_M = 0;
  uint8_t cur_Y = 0;
  uint8_t eeprom_i = 0;
  unsigned char eeprom_str[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  while (1) {
    // Read the current time/date.
    cur_s = i2c_read_register(0x00);
    cur_m = i2c_read_register(0x01);
    cur_h = i2c_read_register(0x02) & 0x3F;
    cur_D = i2c_read_register(0x04);
    cur_M = i2c_read_register(0x05);
    cur_Y = i2c_read_register(0x06);

    // Turn the board's LED on every tenth second.
    if ((cur_s & 0x0F) == 0) {
      GPIOB->ODR |=  (1 << LED_PIN);
    }
    else {
      GPIOB->ODR &= ~(1 << LED_PIN);
    }

    // (For testing EEPROMs on 'ZS-042' boards.)
    //#define VVC_READ_EEPROM 1
    #undef VVC_READ_EEPROM
    #ifdef VVC_READ_EEPROM
    // Read the first 16 bytes of EEPROM data.
    // Set the AT24C32's I2C address.
    I2C1->CR2     &= ~(I2C_CR2_SADD);
    I2C1->CR2     |=  (0xAE << I2C_CR2_SADD_Pos);
    // Write the EEPROM address; 0x000000.
    I2C1->CR2     &= ~(I2C_CR2_NBYTES);
    I2C1->CR2     |=  (2 << I2C_CR2_NBYTES_Pos);
    i2c_start();
    i2c_write_byte(0x00);
    i2c_write_byte(0x00);
    i2c_stop();
    // Read 16 bytes.
    I2C1->CR2 &= ~(I2C_CR2_NBYTES);
    I2C1->CR2 |=  (16 << I2C_CR2_NBYTES_Pos);
    // Set 'read' I2C direction.
    I2C1->CR2 |=  (I2C_CR2_RD_WRN);
    i2c_start();
    for (eeprom_i = 0; eeprom_i < 16; ++eeprom_i) {
      eeprom_str[eeprom_i] = i2c_read_byte();
    }
    i2c_stop();
    // Set 'write' I2C direction again.
    I2C1->CR2 &= ~(I2C_CR2_RD_WRN);
    // Set the DS3231's I2C address again.
    I2C1->CR2     &= ~(I2C_CR2_SADD);
    I2C1->CR2     |=  (0xD0 << I2C_CR2_SADD_Pos);
    #endif
  }
}
