/* File: LIBRARY.H */

#ifndef __LIBRARY_H
#define __LIBRARY_H

#define	HEADER		NULL
#define	ERROR_FLAG	"ERR"

extern void		CallReturnOverhead(void) ;
extern void		ClearDisplay(void) ;
extern void		ClearScreen(int color) ;
extern unsigned	CountCycles(void *function, void *iparams, void *fparams, void *results) ;
extern void		DisplayHeader(char *header) ;
extern void		DisplayFooter(char *footer) ;
extern uint32_t	GetClockCycleCount(void) ;
extern uint32_t	GetRandomNumber(void) ;
extern void		InitializeHardware(char *header, char *footer) ;
extern unsigned	PrintBits(int bin[]) ;
extern void		PrintByte(uint8_t byte) ;
extern int		PushButtonPressed(void) ;
extern void		WaitForPushButton(void) ;

#define		asm		__asm

#endif
