
#include <fx2usb.h>

#include "FX2LPserial.h"

void FX2LPSerial_Init()  // initialize UART0
{
	UART230 = _230UART0; // Enable high speed clock for UART0
	PCON &= ~_SMOD0; // Disable baud rate doubler for UART0 (Set 115Kbps)   
	SCON0 = 0x5A ;
	TI_0 = 1;

	CPUCS = ((CPUCS & ~_CLKSPD0) | _CLKSPD1) ;	//Setting up the clock frequency

	FX2LPSerial_XmitString("\r\n-> ") ;		//Clearing the output screen
	 
}

void FX2LPSerial_XmitChar(__xdata char ch) // prints a character
{
	while (TI_0== 0) ;
	TI_0 = 0 ;
	SBUF0 = ch ;	//print the character
}

void FX2LPSerial_XmitHex1(__xdata uint8_t b) // intermediate function to print the 4-bit nibble in hex format
{
	if (b < 10)
		FX2LPSerial_XmitChar(b + '0') ;
	else
		FX2LPSerial_XmitChar(b - 10 + 'A') ;
}

void FX2LPSerial_XmitHex2(__xdata uint8_t b) // prints the value of the BYTE variable in hex
{
	FX2LPSerial_XmitHex1((b >> 4) & 0x0f) ;
	FX2LPSerial_XmitHex1(b & 0x0f) ;
}

void FX2LPSerial_XmitHex4(__xdata uint16_t w) // prints the value of the WORD variable in hex
{
	FX2LPSerial_XmitHex2((w >> 8) & 0xff) ;
	FX2LPSerial_XmitHex2(w & 0xff) ;
}

void FX2LPSerial_XmitHex8(__xdata uint32_t w) // prints the value of the WORD variable in hex
{
	FX2LPSerial_XmitHex4((w >> 16) & 0xffff) ;
	FX2LPSerial_XmitHex4(w & 0xffff) ;
}

void FX2LPSerial_XmitString(char *str)
{
	while (*str)
		FX2LPSerial_XmitChar(*str++) ;
}


