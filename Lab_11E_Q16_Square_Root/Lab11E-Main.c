/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.

    Square root by abacus algorithm, Martin Guy @ UKC, June 1985.
    From a book on programming abaci by Mr C. Woo.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "library.h"
#include "graphics.h"
#include "intrinsics.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

typedef int32_t         Q16 ;

Q16 __attribute__((weak)) Q16GoodRoot(Q16 radicand)
    {
/*    uint32_t residue, sqroot, bit ;

    // set 'bit' to highest bit position less than radicand
    bit = (1 << 30) >> (__CLZ(radicand) & ~1) ;

    residue = (uint32_t) radicand ;     // residue will be the 'remainder'
  
    sqroot  = 0 ;                       // sqroot will be integer square root
    while (bit != 0)
        {
        uint32_t temp ;

        temp = sqroot + bit ;           // Compute once, use twice
        if (residue >= temp)            // Room to subtract?
            {
            residue -= temp ;           // Yes - deduct from residue
            sqroot += (bit << 1) ;      // and step sqroot
            }

        sqroot >>= 1 ;                  // Slide evolving sqroot 1 bit down the residue
        bit >>= 2 ;                     // Slide the bitpick 1 bit down the sqroot
        }

    return (Q16) (sqroot << 8) ;  */      // scale result for Q16 format
    }

Q16 __attribute__((weak)) Q16BestRoot(Q16 radicand)
    {
    uint64_t residue, sqroot, bit ;

    // set 'bit' to highest bit position less than radicand
    bit = (1 << 30) >> (__CLZ(radicand) & ~1) ;
    bit <<= 16 ;

    residue = ((uint64_t) radicand) << 16 ; // residue will be the 'remainder

    sqroot  = 0 ;                       // sqroot will be integer square root
    while (bit != 0)
        {
        uint64_t temp ;

        temp = sqroot + bit ;           // Compute once, use twice
        if (residue >= temp)            // Room to subtract?
            {
            residue -= temp ;           // Yes - deduct from residue
            sqroot += (bit << 1) ;      // and step sqroot
            }

        sqroot >>= 1 ;                  // Slide evolving sqroot 1 bit down the residue
        bit >>= 2 ;                     // Slide the bitpick 1 bit down the sqroot
        }

    return (Q16) sqroot ;               // scale result for Q16 format
    }

#pragma GCC pop_options

typedef struct
    {
    uint8_t *           table ;
    uint16_t            Width ;
    uint16_t            Height ;
    } sFONT ;

static Q16              FloatToQ16(float f) ;
static void             PlotGCC(int xlft, int ytop, char *label, float (*func)(float)) ;
static void             PlotQ16(int xlft, int ytop, char *label, Q16 (*func)(Q16)) ;
static float            Q16ToFloat(Q16 q) ;
static void             SetFontSize(sFONT *Font) ;

#define HEADER_ROWS     48
#define FOOTER_ROWS     16
#define MIDDLE_ROWS     (YPIXELS - HEADER_ROWS - FOOTER_ROWS)

#define MARGIN_HORIZ    10
#define MARGIN_VERT     11

#define PLOT_XLFT       MARGIN_HORIZ
#define PLOT_COLS       (XPIXELS - 2*MARGIN_HORIZ)
#define PLOT_ROWS       70
#define PLOT_FONT       Font12
#define PLOT_VCOL       (XPIXELS - MARGIN_HORIZ - PLOT_FONT.Width/2 - 9*PLOT_FONT.Width)

#define GCC_XLFT        PLOT_XLFT
#define GCC_YTOP        (HEADER_ROWS + MARGIN_VERT)
#define GCC_YBTM        (GCC_YTOP + PLOT_ROWS)

#define BEST_XLFT       PLOT_XLFT
#define BEST_YTOP       (GCC_YBTM + MARGIN_VERT)
#define BEST_YBTM       (BEST_YTOP + PLOT_ROWS) 

#define GOOD_XLFT       PLOT_XLFT
#define GOOD_YTOP       (BEST_YBTM + MARGIN_VERT)
#define GOOD_YBTM       (GOOD_YTOP + PLOT_ROWS)

#define Q16_TWO         ((Q16) 0x00020000)
#define LS16BITS(x)     ((uint16_t *) &x)[0]
#define MS16BITS(x)     ((uint16_t *) &x)[1]

extern  sFONT           Font8, Font12, Font16, Font20, Font24 ;

int main()
    {
    InitializeHardware(HEADER, "Lab 11E: Q16 Square Root") ;

    PlotGCC(GCC_XLFT,  GCC_YTOP,  "GCC sqrtf",   sqrtf) ;
    PlotQ16(GOOD_XLFT, GOOD_YTOP, "Q16GoodRoot", Q16GoodRoot) ;
    PlotQ16(BEST_XLFT, BEST_YTOP, "Q16BestRoot", Q16BestRoot) ;

    return 0 ;
    }

static void PlotGCC(int xlft, int ytop, char *label, float (*func)(float))
    {
    float f32, root, max ;
    int rows, row, col ;
    char text[100] ;

    SetForeground(COLOR_LIGHTGREEN) ;
    FillRect(xlft, ytop, PLOT_COLS, PLOT_ROWS) ;
    SetForeground(COLOR_BLACK) ;
    DrawRect(xlft, ytop, PLOT_COLS, PLOT_ROWS) ;

    max = (*func)(INT32_MAX/65536.0) ;
    for (col = 0; col < PLOT_COLS; col++)
        {
        f32  = (col / (float) PLOT_COLS) * (INT32_MAX/65536.0) ;
        root = (*func)(f32) ;
        rows = (root / max) * PLOT_ROWS ;
        DrawPixel(xlft + col, ytop + PLOT_ROWS - rows, COLOR_RED) ;
        }

    SetFontSize(&PLOT_FONT) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_LIGHTGREEN) ;

    sprintf(text, "Ymax: %9.5f", max) ;
    col = xlft + PLOT_FONT.Width/2 ;
    DisplayStringAt(col, ytop + PLOT_FONT.Height/2, text) ;

    root = (*func)(2.0) ;
    sprintf(text, "%s(2.0): ", label) ;
    row = ytop + PLOT_ROWS - (3*PLOT_FONT.Height)/2 ;
    col = PLOT_VCOL - PLOT_FONT.Width*strlen(text) ;
    DisplayStringAt(col, row, text) ;

    sprintf(text, "%.7f", root) ;
    DisplayStringAt(PLOT_VCOL, row, text) ;
    }

static void PlotQ16(int xlft, int ytop, char *label, Q16 (*func)(Q16))
    {
    Q16 q16, root, max ;
    int rows, row, col ;
    char text[100] ;

    SetForeground(COLOR_LIGHTGREEN) ;
    FillRect(xlft, ytop, PLOT_COLS, PLOT_ROWS) ;
    SetForeground(COLOR_BLACK) ;
    DrawRect(xlft, ytop, PLOT_COLS, PLOT_ROWS) ;

    max = (*func)(INT32_MAX) ;
    for (col = 0; col < PLOT_COLS; col++)
        {
        q16  = (col / (float) PLOT_COLS) * INT32_MAX ;
        root = (*func)(q16) ;
        rows = (root / (float) max) * PLOT_ROWS ;
        DrawPixel(xlft + col, ytop + PLOT_ROWS - rows, COLOR_RED) ;
        }

    SetFontSize(&PLOT_FONT) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_LIGHTGREEN) ;

    sprintf(text, "Ymax: %u", MS16BITS(max)) ;
    col = xlft + PLOT_FONT.Width/2 ;
    DisplayStringAt(col, ytop + PLOT_FONT.Height/2, text) ;
    col += PLOT_FONT.Width * strlen(text) ;
    sprintf(text, "%.5f", LS16BITS(max)/65536.0) ;
    DisplayStringAt(col, ytop + PLOT_FONT.Height/2, text + 1) ;

    q16  = FloatToQ16(2.0) ;
    root = (*func)(q16) ;
    sprintf(text, "%s(2.0): ", label) ;
    row = ytop + PLOT_ROWS - (3*PLOT_FONT.Height)/2 ;
    col = PLOT_VCOL - PLOT_FONT.Width*(2 + strlen(text)) ;
    DisplayStringAt(col + 2*PLOT_FONT.Width, row, text) ;

    sprintf(text, "%.5f", Q16ToFloat(root)) ;
    DisplayStringAt(PLOT_VCOL, row, text) ;
    }

static void SetFontSize(sFONT *Font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(Font) ;
    }

static float Q16ToFloat(Q16 q)
    {
    return q / 65536.0 ;
    }

static Q16 FloatToQ16(float f)
    {
    return (Q16) (65536*f) ;
    }

