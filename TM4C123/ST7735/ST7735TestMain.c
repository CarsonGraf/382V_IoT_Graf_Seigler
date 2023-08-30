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

// Adafruit ST7735 use INITR_REDTAB
// HiLetgo ST7735, use INITR_BLACKTAB
int main1(void){
  PLL_Init(Bus80MHz);               // set system clock to 80 MHz
  ST7735_InitR(INITR_BLACKTAB);
  ST7735_OutString("hello world");
  while(1){
  }
}



int main2(void){ int i; // main 2
  PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
  ST7735_InitR(INITR_BLACKTAB); //
  ST7735_FillRect(20,20,10,10,ST7735_YELLOW);
  for(i=0;i<80;i++){
    ST7735_FillRect(i,2*i,2,2,ST7735_MAGENTA);
  }
  while(1){
  }
}

int main3(void){uint32_t j; // main 3
  PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
  ST7735_InitR(INITR_BLACKTAB);
  ST7735_OutString("Graphics test\n");
  ST7735_OutString("cubic function\n");
  ST7735_PlotClear(0,4095);  // range from 0 to 4095
  for(j=0;j<128;j++){
    ST7735_PlotPoints(j*j/2+900-(j*j/256)*j,32*j); // cubic,linear
    ST7735_PlotNext();
  }   // called 128 times
  while(1){
  }
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";

const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
//********ST7735_OutPhrase*****************
// Print a string of characters to the ST7735 LCD.
// Position determined by ST7735_SetCursor command
// Color set by ST7735_SetTextColor
// The string will not automatically wrap.
// inputs: Phrase type
// outputs: none
void ST7735_OutPhrase(phrase_t message){
  const char *ptr = Phrases[message][myLanguage];
  while(*ptr){
    ST7735_OutChar(*ptr);
    ptr = ptr + 1;
  }
}  
void ST7735_LanguageTest(Language_t l){
  Language_t old = myLanguage;
  myLanguage = l;
  ST7735_OutPhrase(LANGUAGE); ST7735_OutChar(' ');
  ST7735_OutPhrase(HELLO); ST7735_OutChar(' ');
  ST7735_OutPhrase(GOODBYE); ST7735_OutChar(13);
  myLanguage = old;
}
void ST7735_PhraseTest(phrase_t message){
  Language_t old = myLanguage;
  myLanguage = English;
  ST7735_OutPhrase(LANGUAGE); ST7735_OutChar(' ');
  ST7735_OutPhrase(message); ST7735_OutChar(13);
  myLanguage = Spanish;
  ST7735_OutPhrase(LANGUAGE); ST7735_OutChar(' ');
  ST7735_OutPhrase(message); ST7735_OutChar(13);
  myLanguage = Portuguese;
  ST7735_OutPhrase(LANGUAGE); ST7735_OutChar(' ');
  ST7735_OutPhrase(message); ST7735_OutChar(13);
  myLanguage = French;
  ST7735_OutPhrase(LANGUAGE); ST7735_OutChar(' ');
  ST7735_OutPhrase(message); ST7735_OutChar(13);
  myLanguage = old;
}  
void Pause(void){
  while(PF4){ Clock_Delay1ms(10);}
  while(PF4 == 0){ Clock_Delay1ms(10);}
}
int main(void){char l; // main 4
  PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
  LaunchPad_Init();
  ST7735_InitR(INITR_BLACKTAB);
  ST7735_OutString("LanguageTest\n");
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
      ST7735_OutString((char *)Phrases[LANGUAGE][myL]);ST7735_OutChar(' ');
      ST7735_OutString((char *)Phrases[myPhrase][myL]);ST7735_OutChar(13);
    }
  }
  Pause();
  l = 128;
  ST7735_OutString("Extended ASCII\n");
  ST7735_OutPhrase(HELLO);
  while(1){
    ST7735_FillScreen(0);
    for(int i=0;i<16;i++){
      ST7735_SetCursor(0,i);
      ST7735_OutUDec(l); 
      ST7735_OutChar(' '); 
      ST7735_OutChar(l); 
      l++;
    }
    Pause();
  }  
}

