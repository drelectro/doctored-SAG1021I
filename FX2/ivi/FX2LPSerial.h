
#ifndef _INCLUDED_FX2LPSERIAL_H
#define _INCLUDED_FX2LPSERIAL_H

#define FX2LP_SERIAL
#ifdef FX2LP_SERIAL


extern void FX2LPSerial_Init();

extern void
FX2LPSerial_XmitChar(__xdata char ch);

extern void
FX2LPSerial_XmitHex1(__xdata uint8_t b);

extern void
FX2LPSerial_XmitHex2(__xdata uint8_t b);

extern void
FX2LPSerial_XmitHex4(__xdata uint16_t w);

extern void
FX2LPSerial_XmitHex8(__xdata uint32_t w);

extern void
FX2LPSerial_XmitString(char *str);

#endif
#endif
