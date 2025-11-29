/*
 * DHT11 Temperature and Humidity Sensor Driver
 * Header file for 8051 microcontroller
 */

#ifndef DHT11_H
#define DHT11_H

#include <reg51.h>

// DHT11 Pin Definition
#define DHT11_PORT P1
#define DHT11_BIT 7
#define DHT11_PIN P1_7

// DHT11 Timing Constants (in microseconds)
#define DHT11_START_TIME 18000      // 18ms start signal
#define DHT11_RESPONSE_TIME 80      // 80us response
#define DHT11_DATA_ZERO 26          // Data bit 0: 26us
#define DHT11_DATA_ONE 70           // Data bit 1: 70us

// Function Declarations
void DHT11_Init(void);
unsigned char DHT11_Read(unsigned char *temp, unsigned char *humidity);
void DHT11_Start(void);
unsigned char DHT11_CheckResponse(void);
unsigned char DHT11_ReadBit(void);
unsigned char DHT11_ReadByte(void);
void DHT11_Delay_us(unsigned int us);
void DHT11_Delay_ms(unsigned int ms);

#endif // DHT11_H
