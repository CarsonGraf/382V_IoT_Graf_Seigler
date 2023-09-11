// Seeed_HC12main.c
// Runs on TM4C123
// Lab 1 start project for ECE382V, Technology for Embedded IoT
// Use the SysTick timer to request interrupts at a particular period.
// Jonathan Valvano
// Aug 2, 2023

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2020
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2023, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// built-in LED connected to PF1,PF2,PF3
// PF3 is an output to profiling scope/logic analyzer for ISR
// UART output to PC for debugging
//   U0Rx (VCP receive) connected to PA0
//   U0Tx (VCP transmit) connected to PA1

// HC12 pin connections
//   5 SET to GPIO      from LaunchPad to HC12  PB4 (GPIO output){TM4C123}
//   4 SO to serial RxD from HC12 to LaunchPadU1Rx PB0 is RxD (input to TM4C123)
//   3 SI to serial TxD from LaunchPad to HC12  U1Tx PB1 is TxD (output of TM4C123)
//   2 GND
//   1 VCC 1N4004 diode to +5V Vin
//       plus side of diode on +5V
//       stripe side of the diode on HC12 VCC
//   Power pin, the requirements of 3.2V to 5.5V
//   DC power supply, the supply current is not less
//   than 200mA. Note: If the module is to work
//   for a long time in the transmit state, it is
//   recommended that the power supply voltage
//   of more than 4.5V when connected to a
//   1N4007/1N4004 diode, to avoid the module built-in
//   LDO fever.
// SSD1306 LCD connections to I2C0
//   GND    ground
//   VIN    3.3V
//   SDA    SSD1306 SDA <> PB3 I2C0 data
//   SCL    SSD1306 SCL <- PB2 I2C0 clock with 1.5k pullup to 3.3V
// Four positive logic switch inputs
//   DOWN   PE0
//   LEFT   PE1
//   RIGHT  PE2
//   UP     PE3

#include <stdint.h>
#include <stdio.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/UART.h"
#include "../inc/UART1int.h"
#include "../inc/SSD1306.h"
#include "../inc/SysTickInts.h"
#include "../inc/PLL.h"
#include "../inc/LaunchPad.h"
#include "../inc/CortexM.h"
#define PE3  (*((volatile uint32_t *)0x40024020))   // UP
#define PE2  (*((volatile uint32_t *)0x40024010))   // RIGHT
#define PE1  (*((volatile uint32_t *)0x40024008))   // LEFT
#define PE0  (*((volatile uint32_t *)0x40024004))   // DOWN
#define PA4  (*((volatile uint32_t *)0x40004040))   // GREEN
#define PA3  (*((volatile uint32_t *)0x40004020))   // YELLOW
#define PA2  (*((volatile uint32_t *)0x40004010))   // RED
void Switch_Init(void);
uint8_t Switch_Input(void);
void LED_Init();
void LED_Output(uint32_t data);
volatile uint32_t Time,MainCount;
// PortB at 0x400053FC
// PB4 bit-specific address is 0x40005040
#define SET (*((volatile uint32_t *)0x40005040))

void SysTick_Handler(void){
  PF3 ^= 0x08;       // toggle PF3
  PF3 ^= 0x08;       // toggle PF3
  Time = Time + 1;
  PF3 ^= 0x08;       // toggle PF3
}

void HC12_ReadAllInput(void){uint8_t in;
// flush receiver buffer
  in = UART1_InCharNonBlock();
  while(in){
    UART_OutChar(in);
    in = UART1_InCharNonBlock();
  }
}

char HC12data;

void HC12_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x00000002;  // activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};// allow time for clock to stabilize
  GPIO_PORTB_DIR_R |= 0x10;        // PB4 output
  GPIO_PORTB_DEN_R |= 0x10;        // enable digital I/O on PB4
  SET = 0;

  UART1_Init9600();    // serial port to HC12, 9600 baud
  HC12data = '1';
  //************ configure the HC12 module**********************
  SET = 0;       // enter AT command mode
  Clock_Delay1ms(40);
  UART1_OutString("AT+B9600\n");  // UART baud rate set to 9600
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+C007\n");   // channel 7 selected (001 to 100 valid)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+P8\n");    // highest power level (1 to 8)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+RF\n");    // read FU transmission mode (FU3)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+V\n");    // read firmware
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  SET = 0x10;  // exit AT command mode
  Clock_Delay1ms(200);
  HC12_ReadAllInput(); // remove any buffered input
  //************ configuration ended***********************
  printf("\nRF_XMT initialization done\n");
}

/**
 * main.c
 */


void main(void){
    int counter = 0;
    int wait_count = 0;
    char rx_string[32] = {0};
    char tx_counter = 1;
    char rx_counter = 1;
    char clear_to_send = 0;

    PLL_Init(Bus80MHz);      // running on crystal
    Time = MainCount = 0;
    SysTick_Init(800000);   // set up SysTick for 100 Hz interrupts
    LaunchPad_Init();       // PF4-PF0 LaunchPad
    Switch_Init();          // PE3-PE0 switches
    LED_Init();             // PF4-PF2 LEDs
    PA3 = 0x08;             // yellow on
    Output_Init();          // serial port to PC for debugging
    SSD1306_Init(SSD1306_SWITCHCAPVCC);
    SSD1306_OutClear();
    SSD1306_SetCursor(0,0);
    SSD1306_OutString("TX:\n");
    SSD1306_SetCursor(0,2);
    SSD1306_OutString("RX:\n");
    EnableInterrupts();
    HC12_Init();
    while(1)
    {
        WaitForInterrupt();
        MainCount++;

        if(!(Time%100))
        {
            if(clear_to_send || (wait_count >= 5))
            {
                UART1_OutChar('S');
                UART1_OutChar(tx_counter);
                SSD1306_SetCursor(0,1);
                SSD1306_OutUDec(tx_counter);
                clear_to_send = 0;
                wait_count = 0;
            }
            else
            {
                wait_count += 1;
            }
        }

        char in = 0;
        while(in = UART1_InCharNonBlock())
        {
            rx_string[counter] = in;
            counter++;
        }

        switch(counter)
        {
            case 0 :
            case 1 :
            break;
            case 2 :
                switch(rx_string[0])
                {
                    case 'R' :
                        tx_counter = (tx_counter % 99) + 1;
                        clear_to_send = 1;
                    break;
                    case 'E' :
                        tx_counter = rx_string[1];
                        clear_to_send = 1;
                    break;
                    case 'S' :
                        if(rx_string[1] == rx_counter)
                        {
                            UART1_OutChar('R');
                            UART1_OutChar(rx_counter);
                            SSD1306_SetCursor(0,3);
                            SSD1306_OutUDec(rx_counter);
                            rx_counter = (rx_counter % 99) + 1;
                        }
                        else
                        {
                            UART1_OutChar('E');
                            UART1_OutChar(rx_counter);
                        }
                    break;
                    default :
                        UART1_OutChar('E');
                        UART1_OutChar(rx_counter);
                    break;
                }
                counter = 0;
            break;
            default :
                UART1_OutChar('E');
                UART1_OutChar(rx_counter);
                counter = 0;
            break;
        }
    }
}


void WaitSwitch(void)
{
  uint8_t ThisInput;
  do{// either button
    ThisInput = LaunchPad_Input(); // wait while release
  }while(ThisInput==0);
  do{// either button
    ThisInput = LaunchPad_Input(); // wait while touched
  }while(ThisInput);
}


void HC12_ResetFactorySetting(void){
  SSD1306_OutClear();
  SSD1306_SetCursor(0,0);
  SSD1306_OutString("ResetFactorySetting\n");
  printf("\nResetFactorySetting\n");
  printf("1) Remove power to HC12\n");
  SSD1306_OutString("Power off to HC12\n");
  SSD1306_OutString("Switch to continue\n");
  printf("Hit switch to continue\n");
  WaitSwitch();

  SYSCTL_RCGCGPIO_R |= 0x00000002;  // activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};// allow time for clock to stabilize
  GPIO_PORTB_DIR_R |= 0x10;        // PB4 output
  GPIO_PORTB_DEN_R |= 0x10;        // enable digital I/O on PB4
  SET = 0;

  UART1_Init9600();    // serial port to HC12, 9600 baud
  Clock_Delay1ms(40);
  printf("2) SET=0; Power on to HC12\n");
  SSD1306_OutString("Power on to HC12\n");
  SSD1306_OutString("Switch to continue\n");
  printf("Hit switch to continue\n");
  WaitSwitch();
  UART1_OutString("AT+B9600\n");  // UART baud rate set to 9600
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  //************ configuration ended***********************
  printf("Done\nDownload regular program to system\n");
  SSD1306_OutString("Done\n");
  SSD1306_OutString("Download regular\n");

}
// run this main2 to reset HC12 to factory reset
void main2(void){
  PLL_Init(Bus80MHz);      // running on crystal
  LaunchPad_Init();         // P1.0 is red LED on LaunchPad
  Output_Init();          // serial port to PC for debugging
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  //HC12_ResetFactorySetting();
  while(1){
  }
 }

//------------Switch_Init------------
// Initialize PE3-PE0 Switches
// Input: none
// Output: none
void Switch_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x00000010;     // activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){};// allow time for clock to stabilize
  GPIO_PORTE_DIR_R &= ~0x0F;           // PE3-PE0 in
  GPIO_PORTE_DEN_R |= 0x0F;            // enable digital I/O on PE3-PE0
}
//------------Switch_Input------------
// Input from Switches PE3-PE0,
// Input: none
// Output: 0x00 none
//         0x01 DOWN  pressed (from PE0)
//         0x02 LEFT  pressed (from PE1)
//         0x04 RIGHT pressed (from PE2)
//         0x08 UP    pressed (from PE3)
uint8_t Switch_Input(void){
  return GPIO_PORTE_DATA_R&0x0F; // PE3-PE0
}
//------------LED_Init------------
// Initialize PA4-PA2 LEDs
// Input: none
// Output: none
void LED_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x00000001;     // activate clock for Port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};// allow time for clock to stabilize
  GPIO_PORTA_DIR_R |= 0x1C;            // PA4-PA2 out
  GPIO_PORTA_DEN_R |= 0x1C;            // enable digital I/O on PA4-PA2
}
//------------LED_Output------------
// Output to LEDs PA4-PA2,
// Input: data
//         0x01 Red    to PA2
//         0x02 Yellow to PA3
//         0x04 Green  to PA4
// Output: none
void LED_Output(uint32_t data){
    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R&~0x1C)|((data<<2)&0x1C); // PE3-PE0
}

// our divisor is b1 1001 0011 because that's what I randomly picked lol
uint8_t crc_divisor_high = 0xc9;
uint8_t crc_divisor_bottom = 0x01;

/**
 * Generates an 8-bit CRC code for the given message using the divisor 0x193
 * Parameters:
 *      message: pointer to the message to generate a CRC for
 *      length: number of bytes in the message
 *
 * Returns:
 *      8-bit CRC of type uint_8
 */
uint8_t CRCGen(uint8_t* message, uint32_t length) {

    uint8_t remainder[length + 1];

    // for local copy of message so we can divide it without changing the original message
    for (int i = 0; i < length; i++)
        remainder[i] = message[i];

    remainder[length] = 0x00;

    uint32_t byte_idx = 0;
    while (byte_idx < length) { // we're done once we have all zeroes starting at bit 8 (bit 0 of the second to last byte)
        if (remainder[byte_idx] == 0) {
            byte_idx++;
            continue;
        }
        uint8_t shift = 0;
        while (!(remainder[byte_idx] & 0x80)) {
            remainder[byte_idx] = remainder[byte_idx] << 1;
            shift++;
        }
        remainder[byte_idx] = remainder[byte_idx] ^ crc_divisor_high;
        remainder[byte_idx] = remainder[byte_idx] >> shift;
        uint8_t xor_mask = crc_divisor_high << (8-shift);
        xor_mask = xor_mask + (crc_divisor_bottom << (7-shift));
        remainder[byte_idx + 1] = remainder[byte_idx + 1] ^ xor_mask;
    }
    return (remainder[length]);

}

/**
 * Checks if a message matches its CRC using the divisor 0x193
 * Parameters:
 *      message: pointer to the message whose CRC will be checked
 *      length: number of bytes in the message
 *
 * Returns:
 *      1 if the CRC matches the message and 0 otherwise
 *
 * IMPORTANT USAGE NOTE: make sure the message's CRC is included as the last byte of message, and
 *      that length counts the CRC byte
 */
uint8_t CRCCheck(uint8_t* message, uint32_t length) {

    uint8_t remainder[length];

    // for local copy of message so we can divide it without changing the original message
    for (int i = 0; i < length; i++)
        remainder[i] = message[i];

    uint32_t byte_idx = 0;
    while (byte_idx < (length - 1)) { // we're done once we have all zeroes starting at bit 8 (bit 0 of the second to last byte)
        if (remainder[byte_idx] == 0) {
            byte_idx++;
            continue;
        }
        uint8_t shift = 0;
        while (!(remainder[byte_idx] & 0x80)) {
            remainder[byte_idx] = remainder[byte_idx] << 1;
            shift++;
        }
        remainder[byte_idx] = remainder[byte_idx] ^ crc_divisor_high;
        remainder[byte_idx] = remainder[byte_idx] >> shift;
        uint8_t xor_mask = crc_divisor_high << (8-shift);
        xor_mask = xor_mask + (crc_divisor_bottom << (7-shift));
        remainder[byte_idx + 1] = remainder[byte_idx + 1] ^ xor_mask;
    }
    return remainder[length - 1] == 0 ? 1 : 0;
}
