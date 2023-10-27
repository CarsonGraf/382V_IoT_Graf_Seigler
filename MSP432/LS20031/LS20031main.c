// LS20031main.c
// Runs on MSP432
// This project will eventual interface the LS20031 GPS module
// Jonathan Valvano, Daniel Valvano
// September 14, 2022

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2022
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2022, Jonathan Valvano, All rights reserved.

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


// built-in LED1 connected to P1.0
// P1.0, P2.0 are an output to profiling scope/logic analyzer
// UCA2RXD (VCP receive) connected to P3.2
// UCA2TXD (VCP transmit) connected to P3.3
// LS20031 pin connection
// 5 Vcc to 3.3V
// 4 Rx P3.3 from LaunchPad to LS20031
// 3 Tx to J1.3 from LS20031 to LaunchPad (UCA2TXD){MSP432 P3.2}
// 2 GND
// 1 GND

#define TRUE 1
#define FALSE 0
#define MAX_PIXELS 118 // 128x128 screen minus 5 pixel border

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "msp.h"
#include "..\inc\CortexM.h"
#include "..\inc\SysTickInts.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Clock.h"
#include "..\inc\UART0.h"
#include "..\inc\UART1.h"
#include "..\inc\BSP.h"
volatile uint32_t Time,MainCount;
#define LEDOUT (*((volatile uint8_t *)(0x42098040)))
// P3->OUT is 8-bit port at 0x4000.4C22
// I/O address be 0x4000.0000+n, and let b represent the bit 0 to 7.
// n=0x4C22, b=0
// bit banded address is 0x4200.0000 + 32*n + 4*b
#define SET (*((volatile uint8_t *)(0x42098440)))
// every 10ms
void SysTick_Handler(void){
//  uint8_t in;
  LEDOUT ^= 0x01;       // toggle P1.0
  LEDOUT ^= 0x01;       // toggle P1.0
  Time = Time + 1;
  uint8_t ThisInput = LaunchPad_Input();   // either button
  if(ThisInput){
    if((Time%100) == 0){ // 1 Hz

    }
  }
  LEDOUT ^= 0x01;       // toggle P1.0
}

void LS20031_Init(uint32_t baud){
  UART1_InitB(baud);    // serial port to LS20031
 printf("\nLS20031 initialization done");
}

/**
 * main.c
 */
enum PacketState{
  INVALID,
  VALID,
  INCOMING
};


int updateMaxLat = FALSE;
int updateMinLat = FALSE;
float maxLat;
float minLat;
int updateMaxLong = FALSE;
int updateMinLong = FALSE;
float maxLong;
float minLong;

typedef struct point{
    uint8_t isNorth; // true or false
    float latitude;
    uint16_t y;
    uint8_t isEast; // true or false
    float longitude;
    uint16_t x;
} point;

int getY(point* point) {
    if (point->latitude == '\0') {
        return FALSE;
    }
    if (point->latitude > maxLat) {
        updateMaxLat = TRUE;
        return FALSE;
    }
    if (point->latitude < minLat) {
        updateMinLat = TRUE;
        return FALSE;
    }
    float latRange = maxLat - minLat;
    float latPerPixel = latRange/MAX_PIXELS;
    float yInRange = point->latitude - minLat;
    point->y = (uint16_t) (yInRange/latPerPixel); // y in number of pixels from bottom

    return TRUE;
}

int getX(point* point) {
    if (point->longitude == '\0') {
        return FALSE;
    }
    if (point->longitude > maxLong) {
        updateMaxLong = TRUE;
        return FALSE;
    }
    if (point->longitude < minLong) {
        updateMinLong = TRUE;
        return FALSE;
    }
    float longRange = maxLong - minLong;
    float longPerPixel = longRange/MAX_PIXELS;
    float xInRange = point->longitude - minLong;
    point->x = (uint16_t)(xInRange/longPerPixel); // x in number of pixels from right

    return TRUE;
}

int charsToFloat(uint8_t* ptr, int lat, float* toReturn) {
    float minutes = 0;
    if (lat) {
        if (ptr[4] != '.') {
            return FALSE;
        }
        *toReturn = 60*(10*((int)ptr[0] - 48) + ((int)ptr[1] - 48)); // get degrees to minutes
        minutes = 10*((int)ptr[2] - 48) + ((int)ptr[3] - 48) + 0.1*((int)ptr[5] - 48) + 0.01*((int)ptr[6] - 48)
                    + 0.001*((int)ptr[7] - 48) + 0.0001*((int)ptr[8] - 48);
        *toReturn = *toReturn + minutes;
    } else {
        if (ptr[5] != '.') {
            return FALSE;
        }
        *toReturn = 60*(100*((int)ptr[0] - 48) + 10*((int)ptr[1] - 48) + ((int)ptr[2] - 48)); // get degrees to minutes
        minutes = 10*((int)ptr[3] - 48) + ((int)ptr[4] - 48) + 0.1*((int)ptr[6] - 48) + 0.01*((int)ptr[7] - 48)
                    + 0.001*((int)ptr[8] - 48) + 0.0001*((int)ptr[9] - 48);
        *toReturn = *toReturn + minutes;
    }
    return TRUE;
}

void changeMaxLong(point* ptr) {
    if(maxLong < ptr->longitude) {
        maxLong = ptr->longitude + 0.5;
    }
}
void changeMaxLat(point* ptr) {
    if(maxLat < ptr->latitude) {
        maxLat = ptr->latitude + 0.5;
    }
}
void changeMinLong(point* ptr) {
    if(minLong > ptr->longitude) {
        minLong = ptr->longitude - 0.5;
    }
}
void changeMinLat(point* ptr) {
    if(minLat > ptr->latitude) {
        minLat = ptr->latitude - 0.5;
    }
}
void setMaxMinLat(point* ptr) {
    maxLat = ptr->latitude + 0.1;
    minLat = ptr->latitude - 0.4;
}
void setMaxMinLong(point* ptr) {
    maxLong = ptr->longitude + 0.1;
    minLong = ptr->longitude - 0.4;
}

// private internal variables that keep track of LCD parameters
static uint32_t CursorX = 0;// X-position of the cursor (0<=CursorX<=20)
static uint32_t CursorY = 0;// Y-position of the cursor (0<=CursorY<=12)
#define DATACOLOR LCD_YELLOW// color of data on the screen
#define HEADCOLOR LCD_WHITE // color of text headers on the screen
static uint16_t TextColor = LCD_YELLOW;
// private internal function outputs a character to desired channel(s)
void outchar(char letter){
  // send it to the LCD
  if(letter == '\r'){
    CursorX = 0;
  }else if(letter == '\n'){
    CursorY = CursorY + 1;
  }else{
    BSP_LCD_DrawChar(CursorX*6, CursorY*10, letter, TextColor, LCD_BLACK, 1);
    CursorX = CursorX + 1;
  }
  // send it to the UART
  UART0_OutChar(letter);
}
// private internal function outputs a null-terminated character string to desired channel(s)
// because of the way BSP_LCD_DrawString() works, '\r' and '\n' are printed, so use the following to skip lines:
//  outchar('\r');
//  outchar('\n');
void outstring(char *str){
  uint32_t successfulPrints;// number of characters successfully printed
  // send it to the LCD
  successfulPrints = BSP_LCD_DrawString(CursorX, CursorY, str, TextColor);
  CursorX = CursorX + successfulPrints;
  // send it to the UART
  UART0_OutString(str);
}
// private internal function outputs an unsigned decimal to desired channel(s)
void outudec(uint32_t n){
  uint32_t ncopy;
  // send it to the LCD
  BSP_LCD_SetCursor(CursorX, CursorY);
  BSP_LCD_OutUDec(n, TextColor);
  // synchronize the cursors
  ncopy = n;
  while(ncopy >= 10){
    CursorX = CursorX + 1;
    ncopy = ncopy%10;
  }
  CursorX = CursorX + 1;
  // send it to the UART
  UART0_OutUDec(n);
}
// private internal function wraps LCD back to start to limit screen flicker
void lcdwrap(void){
  CursorX = 0;
  CursorY = 0;
}
// private internal function sets the color of the next LCD text print
void lcdcolor(uint16_t newColor){
  TextColor = newColor;
}

//BSP_LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
void drawPoint(point* point) {
    int16_t width = 6;
    int16_t left = MAX_PIXELS - point->x + 5 - 3; // +5 for border, -3 for 1/2width of square
    if (left < 5) {
        left = 5;
        width = width - (5 - left);
    }
    if (left > MAX_PIXELS + 5 - 6) {
        width = width - (left - (MAX_PIXELS + 5 - 6));
    }

    int16_t height = 6;
    int16_t top = MAX_PIXELS - point->y + 5 - 3;
    if (top < 5) {
        top = 5;
        height = height - (5 - top);
    }
    if (top > MAX_PIXELS + 5 - 6) {
        height = height - (top- (MAX_PIXELS + 5 - 6));
    }
    BSP_LCD_FillRect(left, top, width, height, LCD_MAGENTA);
}


void main(void){
  uint8_t in;
  uint8_t packet[255];      // the maximum length of each packet is restricted to 255 bytes
  int packetindex = 0;      // pointer to next free position in 'packet'
  int i;
  uint32_t seconds;
  enum PacketState packetstate = INVALID;
  Clock_Init48MHz();        // running on crystal
  BSP_LCD_Init();
  Time = MainCount = 0;
//  SysTick_Init(480000,2);   // set up SysTick for 100 Hz interrupts
  LaunchPad_Init();         // P1.0 is red LED on LaunchPad
  UART0_Initprintf();       // serial port to PC for debugging
  EnableInterrupts();
  LS20031_Init(UART1_BAUD_57600);
/*
// original code, this does something
  printf("\nLS20031 example\n");
  while(1){
    in = UART1_InCharNonBlock();
    if(in){
      UART0_OutChar(in);
      MainCount++;
      LEDOUT ^= 0x01;       // toggle P1.0
    }
  }
// end of original code
*/
// my new code
  /*
  lcdcolor(LCD_CYAN);
  outchar('\r');
  outchar('\n');
  outchar('\r');
  outchar('\n');
  lcdwrap();
  outstring("LS20031 example");
  lcdcolor(DATACOLOR);
  */
  point* currPoint;
  point* tempPoint = (point*) malloc(sizeof(point));
  point** pointList;
  pointList = (point**) malloc(sizeof(point*) * 255);
  for (int j = 0; j < 255; j++) {
      pointList[j] = (point*) malloc(sizeof(point));
  }
  uint8_t pointListIdx = 0;
  uint8_t packetCounter = 0;
  //int m = 0;
  //int n = 0;
  while(1){
    in = UART1_InCharNonBlock();
    if(in){
      // got a character
      if(in == '$'){
        // start/restart building a packet
        packetindex = 0;
        packetstate = INCOMING;
      }else if((packetstate == INCOMING) && ((in == '\r') || (in == '\n'))){
        // packets are terminated by a <CR><LF>
        // checksum is in 'packetindex - 2' and 'packetindex - 1'
        packet[packetindex] = 0;
        packetstate = VALID;
      }else if((packetstate == INCOMING) && (packetindex < 255)){
        // add the character to the packet
        packet[packetindex] = in;
        packetindex = packetindex + 1;
        if(packetindex >= 255){
          packetstate = INVALID;
        }
      }
    }
    if(packetstate == VALID){
      if((packet[2] == 'G') && (packet[3] == 'G') && packet[4] == 'A'){
          packetCounter++;
          if (packetCounter == 30) {
              currPoint = pointList[pointListIdx];
              pointListIdx++;
              //outudec(pointListIdx);
              packetCounter = 0;
          } else {
              currPoint = tempPoint;
          }
        // this packet contains GPS location data
        i = 0;
        while(packet[i] != ','){
          i = i + 1;
        }
        i = i + 1;
        /*
        lcdcolor(HEADCOLOR);
        outchar('\r');
        outchar('\n');
        outstring("Time=");
        lcdcolor(DATACOLOR);
        outchar(packet[i]);
        outchar(packet[i+1]);
        outchar(':');
        outchar(packet[i+2]);
        outchar(packet[i+3]);
        outchar(':');
        outchar(packet[i+4]);
        outchar(packet[i+5]);
        outchar('.'); // this character is actually sent in position 6, skip it
        outchar(packet[i+7]);
        outchar(packet[i+8]);
        outchar(packet[i+9]);
        */
        while(packet[i] != ','){
          i = i + 1;
        }
        i = i + 1;
        if (!charsToFloat((packet + i), TRUE, &(currPoint->latitude)))
            continue;
        if (maxLat == '\0') {
            setMaxMinLat(currPoint);
        }
        if (!getY(currPoint)) {
            if (updateMaxLat)
                changeMaxLat(currPoint);
            else if (updateMinLat)
                changeMinLat(currPoint);
            else continue;
            if (packetCounter == 0) { // not sure if we want to resize everything only on rewrite or what
                for (int j = 0; j < pointListIdx; j++) {
                    getY(pointList[j]);
                }
            }
        }

        while(packet[i] != ','){
          i = i + 1;
        }
        i = i + 1;
        currPoint->isNorth = (packet[i] == 'N');
        i = i + 2;
        if (!charsToFloat((packet + i), FALSE, &(currPoint->longitude)))
            continue;
        if (maxLong == '\0') {
            setMaxMinLong(currPoint);
        }
        if (!getX(currPoint)) {
            if (updateMaxLong)
                changeMaxLong(currPoint);
            else if (updateMinLong)
                changeMinLong(currPoint);
            else continue;
            if (packetCounter == 0) {
                for (int j = 0; j < pointListIdx; j++) {
                    getX(pointList[j]);
                }
            }
        }

        while(packet[i] != ','){
          i = i + 1;
        }
        i = i + 1;
        currPoint->isEast = (packet[i] == 'E');

        /* test if you wanna print some data to screen for debug
        CursorX = 1; CursorY = 1;
        outudec(currPoint.x);
        outchar('\r');
        outchar('\n');
        outudec(currPoint.y);
        */
        // declaration for ref: void BSP_LCD_DrawPixel(int16_t x, int16_t y, uint16_t color) {
        //BSP_LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        // BSP_LCD_FillScreen(uint16_t color)
      }
      // draw point with square of width 5 pixels
     // BSP_LCD_FillScreen(LCD_BLACK);
      drawPoint(currPoint);
      packetstate = INVALID;
      LEDOUT ^= 0x01;       // toggle P1.0
    }
  }
// end of my new code
}
void main2(void){
  uint8_t in;
  uint8_t packet[255];      // the maximum length of each packet is restricted to 255 bytes
  int packetindex = 0;      // pointer to next free position in 'packet'
  int i;
  uint32_t seconds;
  enum PacketState packetstate = INVALID;
  Clock_Init48MHz();        // running on crystal
  BSP_LCD_Init();
  Time = MainCount = 0;
//  SysTick_Init(480000,2);   // set up SysTick for 100 Hz interrupts
  LaunchPad_Init();         // P1.0 is red LED on LaunchPad
  UART0_Initprintf();       // serial port to PC for debugging
  EnableInterrupts();
  LS20031_Init(UART1_BAUD_57600);
/*
// original code, this does something
  printf("\nLS20031 example\n");
  while(1){
    in = UART1_InCharNonBlock();
    if(in){
      UART0_OutChar(in);
      MainCount++;
      LEDOUT ^= 0x01;       // toggle P1.0
    }
  }
// end of original code
*/
// my new code
  lcdcolor(LCD_CYAN);
  outchar('\r');
  outchar('\n');
  outchar('\r');
  outchar('\n');
  lcdwrap();
  outstring("LS20031 example");
  lcdcolor(DATACOLOR);
  while(1){
    in = UART1_InCharNonBlock();
    if(in){
      // got a character
      if(in == '$'){
        // start/restart building a packet
        packetindex = 0;
        packetstate = INCOMING;
      }else if((packetstate == INCOMING) && ((in == '\r') || (in == '\n'))){
        // packets are terminated by a <CR><LF>
        // checksum is in 'packetindex - 2' and 'packetindex - 1'
        packet[packetindex] = 0;
        packetstate = VALID;
      }else if((packetstate == INCOMING) && (packetindex < 255)){
        // add the character to the packet
        packet[packetindex] = in;
        packetindex = packetindex + 1;
        if(packetindex >= 255){
          packetstate = INVALID;
        }
      }
    }
    if(packetstate == VALID){
      if((packet[2] == 'G') && (packet[3] == 'G') && packet[4] == 'A'){
        // this packet contains GPS location data
          for (int j = 0; j < packetindex; j++) {
              if (j % 18 == 0) {
                  outchar('\r');
                  outchar('\n');
              }
              outchar(packet[j]);
          }
        outchar('\r');
        outchar('\n');
        lcdwrap();
      }
      packetstate = INVALID;
      LEDOUT ^= 0x01;       // toggle P1.0
    }
  }
// end of my new code
}void main3(void){ // original main
    uint8_t in;
    uint8_t packet[255];      // the maximum length of each packet is restricted to 255 bytes
    int packetindex = 0;      // pointer to next free position in 'packet'
    int i;
    uint32_t seconds;
    enum PacketState packetstate = INVALID;
    Clock_Init48MHz();        // running on crystal
    BSP_LCD_Init();
    Time = MainCount = 0;
  //  SysTick_Init(480000,2);   // set up SysTick for 100 Hz interrupts
    LaunchPad_Init();         // P1.0 is red LED on LaunchPad
    UART0_Initprintf();       // serial port to PC for debugging
    EnableInterrupts();
    LS20031_Init(UART1_BAUD_57600);
  /*
  // original code, this does something
    printf("\nLS20031 example\n");
    while(1){
      in = UART1_InCharNonBlock();
      if(in){
        UART0_OutChar(in);
        MainCount++;
        LEDOUT ^= 0x01;       // toggle P1.0
      }
    }
  // end of original code
  */
  // my new code
    lcdcolor(LCD_CYAN);
    outchar('\r');
    outchar('\n');
    outchar('\r');
    outchar('\n');
    lcdwrap();
    outstring("LS20031 example");
    lcdcolor(DATACOLOR);
    while(1){
      in = UART1_InCharNonBlock();
      if(in){
        // got a character
        if(in == '$'){
          // start/restart building a packet
          packetindex = 0;
          packetstate = INCOMING;
        }else if((packetstate == INCOMING) && ((in == '\r') || (in == '\n'))){
          // packets are terminated by a <CR><LF>
          // checksum is in 'packetindex - 2' and 'packetindex - 1'
          packet[packetindex] = 0;
          packetstate = VALID;
        }else if((packetstate == INCOMING) && (packetindex < 255)){
          // add the character to the packet
          packet[packetindex] = in;
          packetindex = packetindex + 1;
          if(packetindex >= 255){
            packetstate = INVALID;
          }
        }
      }
      if(packetstate == VALID){
        if((packet[2] == 'G') && (packet[3] == 'G') && packet[4] == 'A'){
            printf("data packet\n");
          // this packet contains GPS location data
          i = 0;
          while(packet[i] != ','){
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("Time=");
          lcdcolor(DATACOLOR);
          outchar(packet[i]);
          outchar(packet[i+1]);
          outchar(':');
          outchar(packet[i+2]);
          outchar(packet[i+3]);
          outchar(':');
          outchar(packet[i+4]);
          outchar(packet[i+5]);
          outchar('.'); // this character is actually sent in position 6, skip it
          outchar(packet[i+7]);
          outchar(packet[i+8]);
          outchar(packet[i+9]);
          while(packet[i] != ','){
            i = i + 1;
          }
          outchar(packet[i-1]);
          outchar(packet[i]);
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outchar(packet[i]);
          outchar(packet[i+1]);
          outstring("Latitude=");
          CursorX = 1; CursorY = CursorY + 1; // skip to the next line on LCD only for spacing
          lcdcolor(DATACOLOR);
          outchar(packet[i]);
          outchar(packet[i+1]);
          outchar('d');
          outchar(packet[i+2]);
          outchar(packet[i+3]);
          outchar('\'');
          seconds = (((packet[i+5] - '0')*1000) +
                     ((packet[i+6] - '0')*100) +
                     ((packet[i+7] - '0')*10) +
                     ((packet[i+8] - '0')))*6;
          outudec(seconds/1000);
          outchar('.');
          outudec((seconds%1000)/100);
          outudec((seconds%100)/10);
          outudec((seconds%10));
          outchar('\"');
          while(packet[i] != ','){
            i = i + 1;
          }
          i = i + 1;
          outchar(' ');
          while(packet[i] != ','){
            outchar(packet[i]); // units of latitude (N/S)
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("Longitude=");
          CursorX = 1; CursorY = CursorY + 1; // skip to the next line on LCD only for spacing
          lcdcolor(DATACOLOR);
          outchar(packet[i]);
          outchar(packet[i+1]);
          outchar(packet[i+2]); // like latitude but with one more digit
          outchar('d');
          outchar(packet[i+3]);
          outchar(packet[i+4]);
          outchar('\'');
          seconds = (((packet[i+6] - '0')*1000) +
                     ((packet[i+7] - '0')*100) +
                     ((packet[i+8] - '0')*10) +
                     ((packet[i+9] - '0')))*6;
          outudec(seconds/1000);
          outchar('.');
          outudec((seconds%1000)/100);
          outudec((seconds%100)/10);
          outudec((seconds%10));
          outchar('\"');
          while(packet[i] != ','){
            i = i + 1;
          }
          i = i + 1;
          outchar(' ');
          while(packet[i] != ','){
            outchar(packet[i]); // units of longitude (E/W)
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("Quality=");
          if(packet[i] == '1'){
            lcdcolor(LCD_ORANGE);
            outstring("GPS fix");
            LaunchPad_Output(RED|GREEN);
          }else if(packet[i] == '2'){
            lcdcolor(LCD_GREEN);
            outstring("DGPS fix");
            LaunchPad_Output(GREEN);
          }else{
            lcdcolor(LCD_RED);
            outstring("INVALID");
            LaunchPad_Output(0);
          }
          lcdcolor(DATACOLOR);
          while(packet[i] != ','){
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("Satellites=");
          lcdcolor(DATACOLOR);
          while(packet[i] != ','){
            outchar(packet[i]);
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("HDOP=");
          lcdcolor(DATACOLOR);
          while(packet[i] != ','){
            outchar(packet[i]);
            i = i + 1;
          }
          i = i + 1;
          lcdcolor(HEADCOLOR);
          outchar('\r');
          outchar('\n');
          outstring("Altitude=");
          lcdcolor(DATACOLOR);
          while(packet[i] != ','){
            outchar(packet[i]);
            i = i + 1;
          }
          i = i + 1;
          outchar(' ');
          outchar(packet[i]); // units of altitude
          outchar('\r');
          outchar('\n');
          lcdwrap();
        }
        packetstate = INVALID;
        LEDOUT ^= 0x01;       // toggle P1.0
      }
    }
  // end of my new code
  }
