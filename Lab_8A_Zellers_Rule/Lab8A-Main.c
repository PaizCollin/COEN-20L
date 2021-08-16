/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "library.h"
#include "graphics.h"
#include "touch.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

// Functions to be implemented in assembly language
uint32_t __attribute__((weak)) Zeller1(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    {
    int32_t f, remainder ;

    f = k + (13*m-1)/5 + D + D/4 + C/4 - 2*C ;
    remainder = f % 7 ;
    if (remainder < 0) remainder += 7 ;
    return remainder ;
    }

uint32_t __attribute__((weak)) Zeller2(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    {
    int32_t f, remainder ;

    f = k + (13*m-1)/5 + D + D/4 + C/4 - 2*C ;
    remainder = f % 7 ;
    if (remainder < 0) remainder += 7 ;
    return remainder ;
    }

uint32_t __attribute__((weak)) Zeller3(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    {
    int32_t f, remainder ;

    f = k + (13*m-1)/5 + D + D/4 + C/4 - 2*C ;
    remainder = f % 7 ;
    if (remainder < 0) remainder += 7 ;
    return remainder ;
    }

#pragma GCC pop_options

typedef enum {FALSE = 0, TRUE = 1} BOOL ;

typedef struct
    {
    char *                  lbl ;
    int                     xpos ;
    int                     ypos ;
    int *                   pval ;
    char *                  (*text)(int) ;
    int                     min ;
    int                     max ;
    BOOL                    roll ;
    } ADJUST ;

// Public fonts defined in run-time library
typedef struct
    {
    const uint8_t *         table ;
    const uint16_t          Width ;
    const uint16_t          Height ;
    } sFONT ;

extern sFONT                Font8, Font12, Font16, Font20, Font24 ;

// Private functions defined in this file
static BOOL                 Adjusted(void) ;
static BOOL                 Between(uint32_t min, uint32_t val, uint32_t max) ;
static int                  DaysInMonth(int month, int year) ;
static void                 Delay(uint32_t msec) ;
static void                 DisplayAdjusts(void) ;
static void                 DisplayCycles(unsigned which, unsigned cycles) ;
static void                 DisplayWeekday(int day) ;
static void                 Error(char *functname, char *format, ...) ;
static uint32_t             GetTimeout(uint32_t msec) ;
static void                 InitializeDate(void) ;
static void                 InitializeTouchScreen(void) ;
static BOOL                 LeapYear(int year) ;
static void                 LEDs(int grn_on, int red_on) ;
static char *               Month(int val) ;
static char *               Number(int val) ;
static void                 SanityCheck(void) ;
static void                 SetFontSize(sFONT *Font) ;
static void                 SetupAdjusts(void) ;

#define CPU_CLOCK_SPEED_MHZ 168

#define FONT_ERR            Font12
#define FONT_ADJ            Font20
#define FONT_CYC            Font12

#define ENTRIES(a)          (sizeof(a)/sizeof(a[0]))
#define OPAQUE              0xFF

#define ADJOFF_XLABEL       0
#define ADJOFF_XMINUS       (ADJOFF_XLABEL + 7*FONT_ADJ.Width + 3)
#define ADJOFF_XVALUE       (ADJOFF_XMINUS + 1*FONT_ADJ.Width)
#define ADJOFF_XPLUS        (ADJOFF_XVALUE + 4*FONT_ADJ.Width)

#define ADJUST_YMNTH        65
#define ADJUST_YDATE        (ADJUST_YMNTH + 30)
#define ADJUST_YYEAR        (ADJUST_YDATE + 30)

#define ADJUST_XMNTH        10
#define ADJUST_XDATE        ADJUST_XMNTH
#define ADJUST_XYEAR        ADJUST_XDATE

#define DAY_YPOS            160
#define DAY_XPOS            ((XPIXELS - DAY_WIDTH)/2)
#define DAY_WIDTH           (11*FONT_ADJ.Width)
#define DAY_HEIGHT          (2*FONT_ADJ.Height)

#define CYCLES_YPOS(k)      (235 + (k)*FONT_CYC.Height)
#define CYCLES_XPOS         10

#define ERROR_YPOS          CYCLES_YPOS(1)

#define TS_XFUDGE           -4
#define TS_YFUDGE           -4

static struct { int mnth, date, year ; } adjust ;
static ADJUST adjusts[] =
    {
    {"Month:", ADJUST_XMNTH, ADJUST_YMNTH, &adjust.mnth, Month, 1, 12, TRUE},
    {" Date:", ADJUST_XDATE, ADJUST_YDATE, &adjust.date, Number, 1, 31, TRUE},
    {" Year:", ADJUST_XYEAR, ADJUST_YYEAR, &adjust.year, Number, 1752, 3000, FALSE}
    } ;
static uint32_t (*functions[])() = {Zeller1, Zeller2, Zeller3} ;
static char *functname[] = {"Zeller1", "Zeller2", "Zeller3"} ;
static char *label[] = {"Uses Div & Mul", "No Divide", "No Multiply"} ;
static char *weekday[] =
    {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    } ;
static char *months[] =
    {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    } ;

#define Z_K(date)           date
#define Z_M(mnth)           (1 + (mnth + 9) % 12)
#define Z_D(mnth, year)     ((year - (mnth < 3)) % 100)
#define Z_C(mnth, year)     ((year - (mnth < 3)) / 100) 

#pragma GCC push_options
#pragma GCC optimize ("O0")
int main()
    {
    uint32_t params[4], results[2], delay1, delay2 ;
    unsigned which, day, cycles, ovhd ;

    InitializeHardware(NULL, "Lab 8A: Zeller's Rule") ;
    InitializeTouchScreen() ;
    SanityCheck() ;
    InitializeDate() ;
    SetupAdjusts() ;
    ovhd = CountCycles(CallReturnOverhead, params, params, results) ;

    delay1 = delay2 = 0 ;
    while (1)
        {
        struct tm tm ;

        params[0] = Z_K(adjust.date) ;
        params[1] = Z_M(adjust.mnth) ;
        params[2] = Z_D(adjust.mnth, adjust.year) ;
        params[3] = Z_C(adjust.mnth, adjust.year) ;

        memset(&tm, 0, sizeof(tm)) ;
        tm.tm_mon  = adjust.mnth - 1 ;
        tm.tm_mday = adjust.date ;
        tm.tm_year = adjust.year - 1900 ;
        mktime(&tm) ;

        for (which = 0; which < ENTRIES(functions); which++)
            {
            cycles = CountCycles(functions[which], params, params, results) - ovhd ;
            day = (unsigned) results[0] ;
            if (day != tm.tm_wday) Error(functname[which], "Returns %u != %u", day, tm.tm_wday) ;
            DisplayWeekday(day) ;
            DisplayCycles(which, cycles) ;
            }

        Delay(delay1) ;
        delay1 = delay2 ;
        while (1)
            {
            if (TS_Touched())
                {
                if (Adjusted())
                    {
                    delay2 = 30 ;
                    break ;
                    }
                }
            else delay1 = 500 ;
            }
        }

    return 0 ;
    }
#pragma GCC pop_options

static void SanityCheck(void)
    {
    unsigned day ;

    LEDs(1, 0) ;

    day = Zeller1(Z_K(1), Z_M(1), Z_D(1, 1950), Z_C(1, 1950)) ;
    if (day != 0) Error("Zeller1", "1 Jan 1950 != %u", day) ;

    day = Zeller2(Z_K(25), Z_M(12), Z_D(12, 1975), Z_C(12, 1975)) ;
    if (day != 4) Error("Zeller2", "25 Dec 1975 != %u", day) ;

    day = Zeller3(Z_K(1), Z_M(4), Z_D(4, 2001), Z_C(4, 2001)) ;
    if (day != 0) Error("Zeller3", "1 Apr 2001 != %u", day) ;
    }

static void InitializeDate(void)
    {
    unsigned which ;
    char month[10] ;

    sscanf(__DATE__, "%s %d %d", month, &adjust.date, &adjust.year) ;
    for (which = 0; which < ENTRIES(months); which++)
        {
        if (strcmp(month, months[which]) != 0) continue ;
        adjust.mnth = which + 1 ;
        break ;
        }
    if (which == ENTRIES(months)) Error("Main Program", "Bad current date") ;
    }

static void DisplayWeekday(int day)
    {
    static BOOL initial = TRUE ;
    int xpos ;

    if (initial)
        {
        SetColor(COLOR_YELLOW) ;
        FillRect(DAY_XPOS, DAY_YPOS, DAY_WIDTH, DAY_HEIGHT) ;
        SetColor(COLOR_RED) ;
        DrawRect(DAY_XPOS, DAY_YPOS, DAY_WIDTH, DAY_HEIGHT) ;
        initial = FALSE ;
        }

    SetFontSize(&FONT_ADJ) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_YELLOW) ;
    DisplayStringAt(DAY_XPOS + FONT_ADJ.Width, DAY_YPOS + FONT_ADJ.Height/2, "         ") ;
    xpos = (240 - strlen(weekday[day])*FONT_ADJ.Width)/2 ;
    DisplayStringAt(xpos, DAY_YPOS + FONT_ADJ.Height/2, weekday[day]) ;
    }

static void DisplayCycles(unsigned which, unsigned cycles)
    {
    char text[100] ;

    SetFontSize(&FONT_CYC) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    sprintf(text, "%s: %u cyc (%s)", functname[which], cycles, label[which]) ;
    DisplayStringAt(CYCLES_XPOS, CYCLES_YPOS(which), text) ;
    }

static void Error(char *functname, char *format, ...)
    {
    uint32_t width, row, col, chars ;
    va_list args ;
    char text[100] ;

    va_start(args, format) ;
    vsprintf(text, format, args) ;
    va_end(args) ;

    chars = strlen(text) ;
    if (chars < 19) chars = 19 ;
    width = FONT_ERR.Width * (chars + 2) ;
    col = (XPIXELS - width) / 2 ;
    row = ERROR_YPOS ;

    SetFontSize(&FONT_ERR) ;
    SetColor(COLOR_RED) ;
    FillRect(col, row, width, 3*FONT_ERR.Height) ;
    SetColor(COLOR_BLACK) ;
    DrawRect(col, row, width, 3*FONT_ERR.Height) ;
    row += FONT_ERR.Height/2 ;

    SetForeground(COLOR_WHITE) ;
    SetBackground(COLOR_RED) ;
    DisplayStringAt(col + FONT_ERR.Width, row + FONT_ERR.Height, text) ;
    sprintf(text, "Function %s:", functname) ;
    DisplayStringAt(col + FONT_ERR.Width, row, text) ;

    LEDs(0, 1) ;
    for (;;) ;
    }

static void LEDs(int grn_on, int red_on)
    {
    static uint32_t * const pGPIOG_MODER    = (uint32_t *) 0x40021800 ;
    static uint32_t * const pGPIOG_ODR      = (uint32_t *) 0x40021814 ;
    
    *pGPIOG_MODER |= (1 << 28) | (1 << 26) ;    // output mode
    *pGPIOG_ODR &= ~(3 << 13) ;         // both off
    *pGPIOG_ODR |= (grn_on ? 1 : 0) << 13 ;
    *pGPIOG_ODR |= (red_on ? 1 : 0) << 14 ;
    }

static void SetFontSize(sFONT *Font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(Font) ;
    }

static void InitializeTouchScreen(void)
    {
    static char *message[] =
        {
        "If this message remains on",
        "the screen, there is an",
        "initialization problem with",
        "the touch screen. This does",
        "NOT mean that there is a",
        "problem with your code.",
        " ",
        "To correct the problem,",
        "disconnect and reconnect",
        "the power.",
        NULL
        } ;
    char **pp ;
    unsigned x, y ;

    x = 25 ;
    y = 100 ;
    for (pp = message; *pp != NULL; pp++)
        {
        DisplayStringAt(x, y, *pp) ;
        y += 12 ;
        }
    TS_Init() ;
    ClearDisplay() ;
    }

static void SetupAdjusts(void)
    {
    ADJUST *adj ;
    int which ;

    SetFontSize(&FONT_ADJ) ;
    adj = adjusts ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        DisplayStringAt(adj->xpos + ADJOFF_XLABEL, adj->ypos, adj->lbl) ;

        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_BLACK) ;
        DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        DrawRect(adj->xpos + ADJOFF_XMINUS - 1, adj->ypos - 1, 6*FONT_ADJ.Width + 1, FONT_ADJ.Height + 1) ;

        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_BLACK) ;
        DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
        }

    DisplayAdjusts() ;
    }

static int DaysInMonth(int month, int year)
    {
    static int days[] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} ;
    days[1] = LeapYear(year) ? 29 : 28 ;
    return days[month - 1] ;
    }

static void DisplayAdjusts(void)
    {
    ADJUST *adj ;
    int which ;

    SetFontSize(&FONT_ADJ) ;
    adj = adjusts ;
    adjusts[1].max = DaysInMonth(adjust.mnth, adjust.year) ;
    if (adjust.date > adjusts[1].max) adjust.date = adjusts[1].max ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        SetForeground(COLOR_WHITE) ;
        SetBackground(adj->roll || *adj->pval < adj->max ? COLOR_DARKGREEN : COLOR_RED) ;
        DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
        SetBackground(adj->roll || *adj->pval > adj->min ? COLOR_DARKGREEN : COLOR_RED) ;
        DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        DisplayStringAt(adj->xpos + ADJOFF_XVALUE, adj->ypos, (*adj->text)(*adj->pval)) ;
        }
    }

static BOOL Adjusted(void)
    {
    ADJUST *adj ;
    int which, x, y ;

    x = TS_GetX() + TS_XFUDGE ;
    y = TS_GetY() + TS_YFUDGE ;

    adj = adjusts ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        if (Between(adj->ypos, y, adj->ypos + FONT_ADJ.Height - 1))
            {
            if (Between(adj->xpos + ADJOFF_XMINUS, x, adj->xpos + ADJOFF_XMINUS + FONT_ADJ.Width - 1))
                {
                if (*adj->pval > adj->min) --*adj->pval ;
                else if (adj->roll) *adj->pval = adj->max ;
                break ;
                }

            if (Between(adj->xpos + ADJOFF_XPLUS, x, adj->xpos + ADJOFF_XPLUS + FONT_ADJ.Width - 1))
                {
                if (*adj->pval < adj->max) ++*adj->pval ;
                else if (adj->roll) *adj->pval = adj->min ;
                break ;
                }
            }
        }

    if (which == ENTRIES(adjusts)) return FALSE ;

    DisplayAdjusts() ;
    return TRUE ;
    }

static BOOL Between(uint32_t min, uint32_t val, uint32_t max)
    {
    return (min <= val && val <= max) ;
    }

static char *Number(int val)
    {
    static char text[10] ;
    sprintf(text, "%-4d", val) ;
    return text ;
    }

static char *Month(int val)
    {
    return months[val - 1] ;
    }

static uint32_t GetTimeout(uint32_t msec)
    {
    uint32_t cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
    return GetClockCycleCount() + cycles ;
    }

static void Delay(uint32_t msec)
    {
    uint32_t timeout = GetTimeout(msec) ;
    while ((int) (timeout - GetClockCycleCount()) > 0) ;
    }

static BOOL LeapYear(int year)
    {
    if ((year % 400) == 0)  return TRUE ;
    if ((year % 100) == 0)  return FALSE ;
    return ((year % 4) == 0) ;
    }

