/*
 * IoT Home Automation System - 8051 Microcontroller Firmware
 * File: 8051_Main_Code.c
 * Description: Main firmware for temperature and humidity monitoring
 * MCU: AT89C51 / 89V51RD2
 * Crystal: 11.0592 MHz
 * UART Baud Rate: 9600 bps
 * Created: 2025
 */

#include <reg51.h>
#include "DHT11.h"
#include "UART.h"

// Define Pins
#define DHT11_PIN P1_7
#define LED_RED P1_0
#define LED_GREEN P1_1
#define BUTTON_PIN P1_2

// Global Variables
unsigned char temperature = 0;
unsigned char humidity = 0;
unsigned char sensor_status = 0;
bit data_ready_flag = 0;

/*
 * Function: SystemInit()
 * Description: Initialize system - UART, Timer, and sensors
 * Parameters: None
 * Return: None
 */
void SystemInit(void) {
    UART_Init(9600);              // Initialize UART at 9600 bps
    DHT11_Init();                  // Initialize DHT11 sensor
    
    // Configure GPIO
    LED_RED = 0;                   // Turn off red LED initially
    LED_GREEN = 1;                 // Turn on green LED (system ready)
    
    // Enable interrupts
    EA = 1;                        // Global interrupt enable
    ES = 1;                        // UART interrupt enable
    ET0 = 1;                       // Timer 0 interrupt enable
    
    UART_SendString("IoT Home Automation System Initialized\r\n");
    UART_SendString("System: Ready\r\n");
}

/*
 * Function: Timer0_Init()
 * Description: Initialize Timer0 for 5-second polling interval
 * Parameters: None
 * Return: None
 */
void Timer0_Init(void) {
    TMOD = 0x01;                   // Timer0 in Mode 1 (16-bit)
    TL0 = 0xB0;                    // Load timer value for 5ms interval
    TH0 = 0x3C;                    // (65536 - 5000) * 1.085 microseconds
    TR0 = 1;                        // Start Timer0
}

/*
 * Function: Timer0_ISR() - Interrupt Service Routine
 * Description: Timer0 interrupt handler for periodic sensor reading
 * Called every 5ms
 */
void Timer0_ISR(void) interrupt 1 {
    static unsigned int timer_count = 0;
    
    TL0 = 0xB0;                    // Reload timer
    TH0 = 0x3C;
    
    timer_count++;
    
    // Read sensor every 1000 interrupts (5 seconds)
    if (timer_count >= 1000) {
        timer_count = 0;
        data_ready_flag = 1;       // Set flag to read sensor
    }
}

/*
 * Function: ReadSensor()
 * Description: Read temperature and humidity from DHT11
 * Parameters: None
 * Return: 1 if successful, 0 if error
 */
unsigned char ReadSensor(void) {
    unsigned char result;
    
    // Attempt to read from DHT11
    result = DHT11_Read(&temperature, &humidity);
    
    if (result == 1) {
        sensor_status = 1;         // Sensor is operational
        LED_GREEN = 1;             // Turn on green LED
        LED_RED = 0;               // Turn off red LED
        return 1;
    } else {
        sensor_status = 0;         // Sensor error
        LED_RED = 1;               // Turn on red LED
        LED_GREEN = 0;             // Turn off green LED
        return 0;
    }
}

/*
 * Function: SendSensorData()
 * Description: Transmit sensor data via UART
 * Format: TEMP:XX HUM:XX STATUS:X\r\n
 * Parameters: None
 * Return: None
 */
void SendSensorData(void) {
    unsigned char temp_int, temp_frac;
    unsigned char hum_int, hum_frac;
    
    // Separate integer and fractional parts (example)
    temp_int = temperature / 10;
    temp_frac = temperature % 10;
    hum_int = humidity / 10;
    hum_frac = humidity % 10;
    
    // Send formatted data
    UART_SendString("TEMP:");
    UART_SendByte(temp_int + 0x30);     // Send tens digit
    UART_SendByte(temp_frac + 0x30);    // Send ones digit
    
    UART_SendString(" HUM:");
    UART_SendByte(hum_int + 0x30);
    UART_SendByte(hum_frac + 0x30);
    
    UART_SendString(" STATUS:");
    UART_SendByte(sensor_status + 0x30);
    
    UART_SendString(" \r\n");
}

/*
 * Function: ProcessCommand()
 * Description: Process received UART commands
 * Commands: 'R' - Read sensor, 'S' - System status, 'C' - Calibrate
 * Parameters: unsigned char cmd - Command character
 * Return: None
 */
void ProcessCommand(unsigned char cmd) {
    switch(cmd) {
        case 'R':
            UART_SendString("Reading Sensor...\r\n");
            ReadSensor();
            SendSensorData();
            break;
            
        case 'S':
            UART_SendString("System Status: OK\r\n");
            UART_SendString("Sensor Status: ");
            UART_SendByte(sensor_status + 0x30);
            UART_SendString("\r\n");
            break;
            
        case 'C':
            UART_SendString("System Calibrating...\r\n");
            DHT11_Init();
            UART_SendString("Calibration Complete\r\n");
            break;
            
        default:
            UART_SendString("Unknown Command\r\n");
            break;
    }
}

/*
 * Function: main()
 * Description: Main program loop
 */
void main(void) {
    unsigned char received_byte;
    
    SystemInit();                  // Initialize system
    Timer0_Init();                 // Initialize timer for periodic reads
    
    while(1) {
        // Check if it's time to read sensor
        if (data_ready_flag) {
            data_ready_flag = 0;
            ReadSensor();
            SendSensorData();
        }
        
        // Check for incoming UART data
        if (RI) {
            RI = 0;
            received_byte = SBUF;
            ProcessCommand(received_byte);
        }
    }
}

/*
 * Function: UART_ISR() - UART Interrupt Service Routine
 * Description: Handle UART transmit/receive interrupts
 */
void UART_ISR(void) interrupt 4 {
    if (RI) {
        RI = 0;
        // Data received - processed in main loop
    }
    if (TI) {
        TI = 0;
        // Transmission complete
    }
}

// End of File
