/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "library.h"
#include "graphics.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

// Functions to be called from assembly:
float   DeltaX(void) ;
void    UpdateDisplay(unsigned n, float r, float v, float a) ;

// Function to be implemented in assembly:
void __attribute__((weak)) Integrate(void)
    {
    /*float x, v, r, prev, a, dx ;
    int n ;

    dx = DeltaX() ;
    v = a = 0.0 ;
    x = r = 1.0 ;
    n = 0 ;
    do
        {
        UpdateDisplay(n, r, v, a) ;

        prev = v ;

        r = (1/x + 1/(x + dx))/2 ;
        v += r*r ;
        a += r ;
        n++ ;

        x += dx ;

        } while (v != prev) ; */
    }

#pragma GCC pop_options

typedef enum {FALSE = 0, TRUE = 1} BOOL ;

typedef struct
    {
    uint8_t *           table ;
    uint16_t            Width ;
    uint16_t            Height ;
    } sFONT ;

extern sFONT            Font8, Font12, Font16, Font20, Font24 ;

#define PI              3.14159265358979323846

#define DELTA_X         0.08420836  // chosen for minimal precision loss

#define VAL_BOX_XLFT    75
#define VAL_BOX_YOFF    30
#define VAL_BOX_WIDTH   (13*VAL_TEXT_FONT.Width)
#define VAL_BOX_HEIGHT  (3*VAL_TEXT_FONT.Height)
#define VAL_BOX_XPAD    (VAL_TEXT_FONT.Width/2)
#define VAL_BOX_YPAD    (VAL_TEXT_FONT.Height/2)
#define VAL_TEXT_FONT   Font16
#define VAL_CLR_BGND    COLOR_YELLOW
#define VAL_CLR_BRDR    COLOR_RED
#define VAL_CLR_TEXT    COLOR_BLACK
#define VAL_BOX1_YTOP   (PLOT_YORIGIN - VAL_BOX_HEIGHT - VAL_BOX_YOFF)
#define VAL_BOX2_YTOP   (PLOT_YORIGIN + VAL_BOX_YOFF)

#define PLOT_TEXT_FONT  Font12
#define PLOT_HMARGIN    10
#define PLOT_XORIGIN    PLOT_HMARGIN
#define PLOT_YORIGIN    176
#define PLOT_HEIGHT     100
#define PLOT_WIDTH      (XPIXELS - 2*PLOT_HMARGIN)
#define PLOT_BGND_CLR   COLOR_WHITE
#define PLOT_TEXT_CLR   COLOR_BLACK
#define PLOT_AXIS_CLR   COLOR_BLACK
#define PLOT_DATA_CLR   COLOR_GREEN
#define PLOT_MARK_CLR   COLOR_RED
#define PLOT_MARK_RAD   2
#define PLOT_LBL_XOFF   (PLOT_TEXT_FONT.Width)
#define PLOT_LBL_YOFF   (PLOT_HEIGHT + PLOT_TEXT_FONT.Height/2)
#define PLOT_XGT_XOFF   (PLOT_WIDTH - PLOT_TEXT_FONT.Width/2)
#define PLOT_XGT_YOFF   (PLOT_TEXT_FONT.Height/2 - 1)
#define PLOT_YGT_XOFF   (PLOT_TEXT_FONT.Width/2 + 1)
#define PLOT_YGT_YOFF   (PLOT_HEIGHT + PLOT_TEXT_FONT.Height + 10)

// Functions private to the main program:
static void             DelayMS(unsigned msec) ;
static void             DisplayBtmBox(float v, float a) ;
static void             DisplayTopBox(unsigned n, float x) ;
static void             DrawXAxis(void) ;
static void             DrawYAxis(void) ;
static unsigned         GetTimeout(unsigned msec) ;
static void             InitializeSolution(void) ;
static void             SetFontSize(sFONT *Font) ;
static BOOL             UpdatePlot(float x, float r) ;

static BOOL             display ;
static float            xmax ;
static BOOL             init_topbox ;
static BOOL             init_btmbox ;
static BOOL             init_plot ;

int main(void)
    {
    InitializeHardware(HEADER, "Lab 9B: Gabriel's Horn") ;

    display = FALSE ;   // Run once to get value for xmax
    Integrate() ;

    display = TRUE ;    // Run again to paint display
    for (;;)
        {
        InitializeSolution() ;
        Integrate() ;
        WaitForPushButton() ;
        ClearDisplay() ;
        }

    return 0 ;
    }

float DeltaX(void)
    {
    return DELTA_X ;
    }

void UpdateDisplay(unsigned n, float r, float v, float a)
    {
    float x = 1.0 + DELTA_X * n ;

    if (n == 0) return ;

    if (display == FALSE)
        {
        xmax = x ;
        return ;
        }

    if (UpdatePlot(x, r))
        {
        DisplayTopBox(n, x) ;
        DisplayBtmBox(DELTA_X*PI*v, 2*DELTA_X*PI*a) ;
        DelayMS(100) ;
        }

    while (PushButtonPressed()) ;
    }

static void InitializeSolution(void)
    {
    init_topbox = init_btmbox = init_plot = TRUE ;
    }

static void DisplayTopBox(unsigned n, float x)
    {
    const int xpos = VAL_BOX_XLFT + VAL_BOX_XPAD ;
    char text[20] ;
    int ypos ;

    if (init_topbox)
        {
        SetForeground(VAL_CLR_BGND) ;
        FillRect(VAL_BOX_XLFT, VAL_BOX1_YTOP, VAL_BOX_WIDTH, VAL_BOX_HEIGHT) ;
        SetForeground(VAL_CLR_BRDR) ;
        DrawRect(VAL_BOX_XLFT, VAL_BOX1_YTOP, VAL_BOX_WIDTH, VAL_BOX_HEIGHT) ;
        init_topbox = FALSE ;
        }

    SetFontSize(&VAL_TEXT_FONT) ;
    SetForeground(VAL_CLR_TEXT) ;
    SetBackground(VAL_CLR_BGND) ;

    ypos = VAL_BOX1_YTOP + VAL_BOX_YPAD ;
    sprintf(text, "Steps: %5d", n) ;
    DisplayStringAt(xpos, ypos, text) ;

    ypos += VAL_TEXT_FONT.Height ;
    sprintf(text, "X: %.7f", x) ;
    text[12] = '\0' ;
    DisplayStringAt(xpos, ypos, text) ;
    }

static void DisplayBtmBox(float v, float a)
    {
    const int xpos = VAL_BOX_XLFT + VAL_BOX_XPAD ;
    char text[20] ;
    int ypos ;

    if (init_btmbox)
        {
        SetForeground(VAL_CLR_BGND) ;
        FillRect(VAL_BOX_XLFT, VAL_BOX2_YTOP, VAL_BOX_WIDTH, VAL_BOX_HEIGHT) ;
        SetForeground(VAL_CLR_BRDR) ;
        DrawRect(VAL_BOX_XLFT, VAL_BOX2_YTOP, VAL_BOX_WIDTH, VAL_BOX_HEIGHT) ;
        init_btmbox = FALSE ;
        }

    SetFontSize(&VAL_TEXT_FONT) ;
    SetForeground(VAL_CLR_TEXT) ;
    SetBackground(VAL_CLR_BGND) ;

    ypos = VAL_BOX2_YTOP + VAL_BOX_YPAD ;
    sprintf(text, "V: %.7f", v) ;
    DisplayStringAt(xpos, ypos, text) ;

    ypos += VAL_TEXT_FONT.Height ;
    sprintf(text, "A: %.7f", a) ;
    text[12] = '\0' ;
    DisplayStringAt(xpos, ypos, text) ;
    }

static void DrawYAxis(void)
    {
    const int yoff = PLOT_YORIGIN - PLOT_LBL_YOFF ;

    SetBackground(PLOT_BGND_CLR) ;

    SetForeground(PLOT_TEXT_CLR) ;
    SetFontSize(&PLOT_TEXT_FONT) ;
    DisplayStringAt(PLOT_XORIGIN - PLOT_YGT_XOFF, PLOT_YORIGIN - PLOT_YGT_YOFF, "^ y") ;
    DisplayStringAt(PLOT_XORIGIN + PLOT_LBL_XOFF, yoff + 0*PLOT_HEIGHT + 1, "+1.0") ;
    DisplayStringAt(PLOT_XORIGIN + PLOT_LBL_XOFF, yoff + 2*PLOT_HEIGHT + 1, "-1.0") ;

    SetForeground(PLOT_AXIS_CLR) ;
    DrawVLine(PLOT_XORIGIN - 1, PLOT_YORIGIN - PLOT_HEIGHT - 20, 2*(PLOT_HEIGHT + 20)) ;
    DrawHLine(PLOT_XORIGIN - 5, PLOT_YORIGIN - PLOT_HEIGHT, 10) ;
    DrawHLine(PLOT_XORIGIN - 5, PLOT_YORIGIN + PLOT_HEIGHT, 10) ;
    }

static void DrawXAxis(void)
    {
    static char xlbl[] = "log(x)" ;
    const int xpos = PLOT_XORIGIN + PLOT_WIDTH - PLOT_TEXT_FONT.Width * strlen(xlbl) ;

    SetBackground(PLOT_BGND_CLR) ;

    SetForeground(PLOT_TEXT_CLR) ;
    SetFontSize(&PLOT_TEXT_FONT) ;
    DisplayChar(PLOT_XORIGIN + PLOT_XGT_XOFF, PLOT_YORIGIN - PLOT_XGT_YOFF, '>') ;
    DisplayStringAt(xpos, PLOT_YORIGIN + PLOT_TEXT_FONT.Height/2, xlbl) ;

    SetForeground(PLOT_AXIS_CLR) ;
    DrawHLine(PLOT_XORIGIN - 5, PLOT_YORIGIN, PLOT_WIDTH + 5) ;
    }

static BOOL UpdatePlot(float x, float r)
    {
    static int oldrows, oldcol ;
    int newcol, newrows, dcol ;
    float ypos, dy ;

    if (init_plot)
        {
        oldrows = PLOT_HEIGHT ;
        oldcol = PLOT_XORIGIN ;
        init_plot = FALSE ;
        }

    newcol = PLOT_XORIGIN + PLOT_WIDTH * (logf(x)/logf(xmax)) ;
    if (newcol == oldcol) return FALSE ;

    newrows = PLOT_HEIGHT * r ;

    SetForeground(COLOR_WHITE) ;
    FillCircle(oldcol, PLOT_YORIGIN, 2) ;
    SetForeground(PLOT_DATA_CLR) ;
    for (dcol = 0; dcol <= PLOT_MARK_RAD; dcol++)
        {
        DrawVLine(oldcol - dcol, PLOT_YORIGIN - oldrows, 2*oldrows + 1) ;
        }

    ypos = (float) oldrows ;
    dy = (ypos - newrows) / (newcol - oldcol) ;
    while (oldcol++ < newcol)
        {
        ypos -= dy ;
        int height = (int) (0.5 + ypos) ;
        DrawVLine(oldcol, PLOT_YORIGIN - height, 2*height + 1) ;
        }

    SetForeground(PLOT_DATA_CLR) ;
    DrawVLine(newcol, PLOT_YORIGIN - newrows, 2*newrows + 1) ;
    DrawXAxis() ;
    DrawYAxis() ;
    SetForeground(PLOT_MARK_CLR) ;
    FillCircle(newcol, PLOT_YORIGIN, PLOT_MARK_RAD) ;

    oldcol = newcol ;
    oldrows = newrows ;

    return TRUE ;
    }

static void SetFontSize(sFONT *Font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(Font) ;
    }

static unsigned GetTimeout(unsigned msec)
    {
#   define CPU_CLOCK_SPEED_MHZ 168
    unsigned cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
    return GetClockCycleCount() + cycles ;
    }

static void DelayMS(unsigned msec)
    {
    unsigned timeout = GetTimeout(msec) ;
    while ((int) (timeout - GetClockCycleCount()) > 0) ;
    }


