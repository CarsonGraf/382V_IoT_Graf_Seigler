/**
 * @file      UART1.h
 * @brief     Provide receive/transmit functions for EUSCI A2
 * @details   EUSCI_A2 UART is connected to exernal device like CC2650 or HC12
 * @remark    UCA2RXD (VCP receive) connected to P3.2
 * @remark    UCA2TXD (VCP transmit) connected to P3.3
 * @remark    J1.3  from Bluetooth (DIO3_TXD) to LaunchPad (UART RxD){MSP432 P3.2}
 * @remark    J1.4  from LaunchPad to Bluetooth (DIO2_RXD) (UART TxD){MSP432 P3.3}
 * @remark    Busy-wait device driver for the EUSCI A2 UART output
 * @remark    Interrupting device driver for the EUSCI A2 UART input
 * @version   TI-RSLK MAX v1.1
 * @author    Daniel Valvano and Jonathan Valvano
 * @copyright Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu,
 * @warning   AS-IS
 * @note      For more information see  http://users.ece.utexas.edu/~valvano/
 * @date      August 1, 2023
<table>
<caption id="UART1ports">UART connected via pins</caption>
<tr><th>Pin  <th>signal
<tr><th>P3.2 <td>UCA2RXD (VCP receive)
<tr><th>P3.3 <td>UCA2TXD (VCP transmit)
</table>
 ******************************************************************************/
/*!
 * @defgroup MSP432
 * @brief MSP432 peripherals
 * @{*/

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

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

// standard ASCII symbols
/**
 * \brief Carriage return character
 */
#define CR   0x0D
/**
 * \brief Line feed character
 */
#define LF   0x0A
/**
 * \brief Back space character
 */
#define BS   0x08
/**
 * \brief escape character
 */
#define ESC  0x1B
/**
 * \brief space character
 */
#define SP   0x20
/**
 * \brief delete character
 */
#define DEL  0x7F

/**
 * \brief baud rate, clock/baud rate = 12,000,000/9600 = 1250
 */
#define UART1_BAUD_9600  1250

/**
 * \brief baud rate, clock/baud rate = 12,000,000/115200 = 104.1667
 */
#define UART1_BAUD_115200  104

/**
 * \brief baud rate, clock/baud rate = 12,000,000/57600 = 208.333
 */
#define UART1_BAUD_57600  208
/**
 * @details   Initialize EUSCI_A2 for UART operation
 * @details   115,200 baud rate (assuming 12 MHz SMCLK clock),
 * @details   8 bit word length, no parity bits, one stop bit
 * @param  none
 * @return none
 * @brief  Initialize EUSCI A2
 */
void UART1_Init(void);

/**
 * @details   Initialize EUSCI_A2 for UART operation
 * @details   assumes 12 MHz SMCLK clock,
 * @details   8 bit word length, no parity bits, one stop bit
 *
 * @param  n specifies baud rate, n = 12,000,000/baud rate
 * @return none
 * @brief  Initialize EUSCI A2
 */
void UART1_InitB(uint32_t n);

/**
 * @details   Receive a character from EUSCI_A2 UART
 * @details   Interrupt synchronization,
 * @details   blocking, spin if RxFifo is empty
 * @param  none
 * @return ASCII code of received data
 * @note   UART1_Init or UART1_InitB must be called once prior
 * @brief  Receive byte into MSP432
 */
uint8_t UART1_InChar(void);

/**
 * @details   Wait for new serial port input, interrupt synchronization
 * @param  none
 * @return an 8-bit byte received or 0 if no char
 * @note   return 0 if RxFifo is empty
 * */
uint8_t UART1_InCharNonBlock(void);

/**
 * @details   Receive a character from EUSCI_A2 UART
 * @details   Interrupt synchronization,
 * @details   non-blocking, does not spin if RxFifo is empty
 * @param  none
 * @return ASCII code of received data, returns 0 if no input available
 * @note   UART1_Init or UART1_InitB must be called once prior
 * @brief  Receive byte into MSP432
 */
uint8_t UART1_InCharNonBlock(void);

/**
 * @details   Transmit a character to EUSCI_A2 UART
 * @details   Busy-wait synchronization,
 * @details   blocking, wait for UART to be ready
 * @param  data is the ASCII code for data to send
 * @return none
 * @note   UART1_Init or UART1_InitB must be called once prior
 * @brief  Transmit byte out of MSP432
 */
void UART1_OutChar(uint8_t data);

/**
 * @details   Transmit a string to EUSCI_A2 UART
 * @param  pt is pointer to null-terminated ASCII string to be transferred
 * @return none
 * @note   UART1_Init or UART1_InitB must be called once prior
 * @brief  Transmit string out of MSP432
 */
void UART1_OutString(uint8_t *pt);


/**
 * @details   Wait for all transmission to finish
 * @details   Busy-wait synchronization,
 * @details   blocking, wait for all UART output to be finished
 * @param  none
 * @return none
 * @brief  wait for UART output to complete
 */
void UART1_FinishOutput(void);


/**
 * @details   Check the receive FIFO from EUSCI_A2 UART
 * @details   non-blocking
 * @param  none
 * @return number of characters in FIFO available for reading
 * @brief  Check status of receive FIFO
 */
uint32_t UART1_InStatus(void);

/** @}*/
