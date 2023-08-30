#include "msp.h"
#include "../inc/LaunchPad.h"

/**
 * main.c
 */
void main(void)
{
    LaunchPad_Init();
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	while(1){

	}
}
