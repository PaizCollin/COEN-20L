/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include "library.h"
#include "graphics.h"

static int tbd[] = {0, 0, 0, 0, 0, 0, 0} ;

#pragma GCC push_options
#pragma GCC optimize ("O0")

// Functions to be implemented in assembly
void __attribute__((weak)) HalfWordAccess(int16_t *src)
    {
    tbd[0] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) FullWordAccess(int32_t *src)
    {
    tbd[1] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) AddressDependency(uint32_t *src)
    {
    tbd[2] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) NoAddressDependency(uint32_t *src)
    {
    tbd[3] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) DataDependency(float f1)
    {
    tbd[4] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) NoDataDependency(float f1)
    {
    tbd[5] = 1 ;    // not part of the assembly source code
    }

void __attribute__((weak)) VDIVOverlap(float dividend, float divisor)
    {
    tbd[6] = 1 ;    // not part of the assembly source code
    }

#pragma GCC pop_options

// Public fonts defined in run-time library
typedef struct
    {
    const uint8_t * table ;
    const uint16_t  Width ;
    const uint16_t  Height ;
    } sFONT ;

static unsigned     Display(unsigned item, unsigned row, unsigned col, char *fmt, unsigned cycles) ;
static void         SetFontSize(sFONT *Font) ;

#define HDR_SIZE    48
#define FTR_SIZE    16

#define BOX_FONT    Font12
#define BOX_FCOLS   7
#define BOX_FROWS   12
#define BOX_PAD     6
#define BOX_TCOLS   30
#define BOX_TROWS   19
#define BOX_WIDTH   (BOX_TCOLS * BOX_FCOLS + 2*BOX_PAD)
#define BOX_HEIGHT  (BOX_TROWS * BOX_FROWS + 2*BOX_PAD)
#define BOX_XLFT    ((XPIXELS - BOX_WIDTH)/2)
#define BOX_YTOP    (HDR_SIZE + ((YPIXELS - HDR_SIZE - FTR_SIZE - BOX_HEIGHT)/2))

extern sFONT        Font8, Font12, Font16, Font20, Font24 ;

static uint32_t     words[2] ;

int main()
    {
    static uint32_t iparams[4], dummy[2] ;
    static float fparams[4] ;
    unsigned ovhd, row, col ;

    InitializeHardware(HEADER, "Lab 4C: Optimization") ;
    ovhd = CountCycles(CallReturnOverhead, dummy, dummy, dummy) ;

    SetForeground(COLOR_YELLOW) ;
    FillRect(BOX_XLFT, BOX_YTOP, BOX_WIDTH, BOX_HEIGHT) ;
    SetForeground(COLOR_RED) ;
    DrawRect(BOX_XLFT, BOX_YTOP, BOX_WIDTH, BOX_HEIGHT) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_YELLOW) ;
    SetFontSize(&BOX_FONT) ;

    col = BOX_XLFT + BOX_PAD ;
    row = BOX_YTOP + BOX_PAD ;

    row = Display(0, row, col, "  Half Word Access:", 0) ;
    iparams[0] = (uint32_t) words | 0b00 ;
    row = Display(0, row, col, "    Adrs = X...X00:%4s cycles", CountCycles(HalfWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b01 ;
    row = Display(0, row, col, "    Adrs = X...X01:%4s cycles", CountCycles(HalfWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b10 ;
    row = Display(0, row, col, "    Adrs = X...X10:%4s cycles", CountCycles(HalfWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b11 ;
    row = Display(0, row, col, "    Adrs = X...X11:%4s cycles", CountCycles(HalfWordAccess, iparams, dummy, dummy) - ovhd) ;

    row += BOX_FONT.Height ;

    row = Display(1, row, col, "  Full Word Access:", 0) ;
    iparams[0] = (uint32_t) words | 0b00 ;
    row = Display(1, row, col, "    Adrs = X...X00:%4s cycles", CountCycles(FullWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b01 ;
    row = Display(1, row, col, "    Adrs = X...X01:%4s cycles", CountCycles(FullWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b10 ;
    row = Display(1, row, col, "    Adrs = X...X10:%4s cycles", CountCycles(FullWordAccess, iparams, dummy, dummy) - ovhd) ;
    iparams[0] = (uint32_t) words | 0b11 ;
    row = Display(1, row, col, "    Adrs = X...X11:%4s cycles", CountCycles(FullWordAccess, iparams, dummy, dummy) - ovhd) ;

    row += BOX_FONT.Height ;

    words[0] = (uint32_t) &words[1] ;
    words[1] = (uint32_t) &words[0] ;
    iparams[0] = words[0] ;
    row = Display(2, row, col, "Address Dependency:%4s cycles", CountCycles(AddressDependency,   iparams, dummy, dummy) - ovhd) ;
    row = Display(3, row, col, "     No Dependency:%4s cycles", CountCycles(NoAddressDependency, iparams, dummy, dummy) - ovhd) ;

    row += BOX_FONT.Height ;

    fparams[0] = 1.0 ;
    row = Display(4, row, col, "   Data Dependency:%4s cycles", CountCycles(DataDependency,   iparams, fparams, dummy) - ovhd) ;
    row = Display(5, row, col, "     No Dependency:%4s cycles", CountCycles(NoDataDependency, iparams, fparams, dummy) - ovhd) ;

    row += BOX_FONT.Height ;

    fparams[0] = 1000.0 ;
    fparams[1] = 10.0 ;
    row = Display(6, row, col, "      VDIV Overlap:%4s cycles", CountCycles(VDIVOverlap, iparams, fparams, dummy) - ovhd) ;

    return 0 ;
    }

static unsigned Display(unsigned item, unsigned row, unsigned col, char *fmt, unsigned cycles)
    {
    char num[10], bfr[100] ;

    if (tbd[item] != 0) sprintf(bfr, fmt, "TBD") ;
    else
        {
        sprintf(num, "%u", cycles) ;
        sprintf(bfr, fmt, num) ;
        }
    DisplayStringAt(col, row, (uint8_t *) bfr) ;
    return row + BOX_FONT.Height ;
    }

static void SetFontSize(sFONT *Font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(Font) ;
    }


