// ST7735TestMain.c
// Runs on LM4F120/TM4C123
// Test the functions in ST7735.c by printing basic
// patterns to the LCD.
//    16-bit color, 128 wide by 160 high LCD
// Daniel Valvano
// March 30, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// hardware connections
// **********ST7735 TFT and SDC*******************
// ST7735
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// **********wide.hk ST7735R with ADXL345 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// SDA  – (NC) I2C data for ADXL345 accelerometer
// SCL  – (NC) I2C clock for ADXL345 accelerometer
// SDO  – (NC) I2C alternate address for ADXL345 accelerometer
// Backlight + - Light, backlight connected to +3.3 V

// **********wide.hk ST7735R with ADXL335 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// X– (NC) analog input X-axis from ADXL335 accelerometer
// Y– (NC) analog input Y-axis from ADXL335 accelerometer
// Z– (NC) analog input Z-axis from ADXL335 accelerometer
// Backlight + - Light, backlight connected to +3.3 V

// **********HiLetgo ST7735 TFT and SDC (SDC not tested)*******************
// ST7735
// LED-   (pin 16) TFT, connected to ground
// LED+   (pin 15) TFT, connected to +3.3 V
// SD_CS  (pin 14) SDC, chip select
// MOSI   (pin 13) SDC, MOSI
// MISO   (pin 12) SDC, MISO
// SCK    (pin 11) SDC, serial clock
// CS     (pin 10) TFT, PA3 (SSI0Fss)
// SCL    (pin 9)  TFT, SCK  PA2 (SSI0Clk)
// SDA    (pin 8)  TFT, MOSI PA5 (SSI0Tx)
// A0     (pin 7)  TFT, Data/Command PA6 (GPIO), high for data, low for command
// RESET  (pin 6)  TFT, to PA7 (GPIO)
// NC     (pins 3,4,5) not connected
// VCC    (pin 2)  connected to +3.3 V
// GND    (pin 1)  connected to ground

#include <stdio.h>
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/ST7735.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"
#include "../inc/ultrasound.h"
#include "../inc/Timer4A.h"

uint32_t Counts;
// Interrupt service routine
// Executed every 12.5ns*(period)
void StartMeasurement(void){
  PF3 ^= 0x08;
  WTimer2_StartHCSR04();
  Counts = Counts + 1;
}

uint32_t cycles, distance;
int main(void){ uint32_t t=0,time;
  DisableInterrupts();
  PLL_Init(Bus80MHz);     // set system clock to 80 MHz
  LaunchPad_Init();
  Counts = 0;
  ST7735_InitR(INITR_BLACKTAB);
  ST7735_OutString("Ultrasound\n");
  ST7735_SetCursor(0,1); ST7735_OutUDec3(distance); ST7735_OutString(" mm");
  ST7735_PlotClear(0,1000);  // range from 0 to 1000 mm, 1m
  WTimer2_Init(2);
  Timer4A_Init(&StartMeasurement,1600000,2);    // 50 Hz
  EnableInterrupts();
  while(1){
    if(time = WTimer2_Read()){
      cycles = time;
      PF2 ^= 0x04;
      distance = Cycles2millimeter(cycles);
      if(distance <= 1000){
        t++;
        ST7735_PlotPoint(distance);
        ST7735_PlotNextErase();
        if(t >= 50){
          t = 0; // once a second
          ST7735_SetCursor(0,1); ST7735_OutUDec3(distance);
        }
      }
    }
  }
}



