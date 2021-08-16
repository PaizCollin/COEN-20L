/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include "graphics.h"
#include "touch.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

typedef struct
    {
    uint16_t    cylinder ;
    uint8_t     head ;
    uint8_t     sector ;
    } CHS ;

void __attribute__((weak)) Log2Phys(uint32_t lba, uint32_t heads, uint32_t sectors, CHS *phy)
    {
    //phy->cylinder   = lba / (heads * sectors) ;
    //phy->head       = (lba / sectors) % heads ;
    //phy->sector     = (lba % sectors) + 1 ;
    }

#pragma GCC pop_options

typedef enum {FALSE = 0, TRUE = 1} BOOL ;

typedef struct
    {
    char *                  lbl ;
    unsigned                xpos ;
    unsigned                ypos ;
    unsigned *              pval ;
    char *                  (*text)(unsigned) ;
    unsigned                min ;
    unsigned                max ;
    } ADJUST ;

typedef struct
    {
    unsigned                xmin ;
    unsigned                ymin ;
    unsigned                width ;
    unsigned                height ;
    unsigned *              pval ;
    unsigned *              vmin ;
    unsigned *              vmax ;
    unsigned                xpos ;
    } SLIDER ;

// Public fonts defined in run-time library
typedef struct
    {
    const uint8_t *         table ;
    const uint16_t          Width ;
    const uint16_t          Height ;
    } sFONT ;

extern sFONT Font8, Font12, Font16, Font20, Font24 ;

#define FONT_INP            Font20
#define FONT_OUT            Font16
#define FONT_LBL            Font16

#define SLIDER_BGCLR        COLOR_LIGHTGRAY
#define SLIDER_FGCLR        COLOR_BLACK
#define SLIDER_WIDTH        16

#define CPU_CLOCK_SPEED_MHZ 168

#define VERT_INP_OFF        ((FONT_INP.Height - FONT_LBL.Height)/2)
#define VERT_OUT_OFF        ((FONT_OUT.Height - FONT_LBL.Height)/2)

#define ADJOFF_XLABEL       0
#define ADJOFF_XMINUS       (ADJOFF_XLABEL + 12*FONT_LBL.Width + 3)
#define ADJOFF_XVALUE       (ADJOFF_XMINUS + 1*FONT_INP.Width)
#define ADJOFF_XPLUS        (ADJOFF_XVALUE + 4*FONT_INP.Width)

#define ENTRIES(a)          (sizeof(a)/sizeof(a[0]))

#define MARGIN              10

#define ADJ_CYLS_XPOS       MARGIN
#define ADJ_CYLS_YPOS       60

#define ADJ_HEADS_XPOS      MARGIN
#define ADJ_HEADS_YPOS      (ADJ_CYLS_YPOS + 26)

#define ADJ_SCTRS_XPOS      MARGIN
#define ADJ_SCTRS_YPOS      (ADJ_HEADS_YPOS + 26)

#define CAPACITY_LABEL      MARGIN
#define CAPACITY_XPOS       (ADJOFF_XMINUS + MARGIN)
#define CAPACITY_YPOS       (ADJ_SCTRS_YPOS + 30)

#define LBA_XPOS            10
#define LBA_YPOS            (CAPACITY_YPOS + 45)
#define LBA_WIDTH           220
#define LBA_HEIGHT          SLIDER_WIDTH

#define CYL_LBL_XPOS        MARGIN
#define CYL_VAL_XPOS        (CYL_LBL_XPOS + 16*FONT_OUT.Width)
#define CYL_YPOS            (LBA_YPOS + 50)

#define HEAD_LBL_XPOS       MARGIN
#define HEAD_VAL_XPOS       CYL_VAL_XPOS
#define HEAD_YPOS           (CYL_YPOS + 20)

#define SCTR_LBL_XPOS       MARGIN
#define SCTR_VAL_XPOS       HEAD_VAL_XPOS
#define SCTR_YPOS           (HEAD_YPOS + 20)

#define RSLT_XPOS           (CYL_LBL_XPOS - 5)
#define RSLT_YPOS           (CYL_YPOS - 5)
#define RSLT_HSIZ           (XPIXELS - 10)
#define RSLT_VSIZ           (SCTR_YPOS - CYL_YPOS + FONT_LBL.Height + 10)

#define TS_XFUDGE           -4
#define TS_YFUDGE           -4

static BOOL                 Adjusted(unsigned x, unsigned y) ;
static BOOL                 Between(unsigned  min, unsigned  val, unsigned  max) ;
static char *               Capacity(unsigned cylinders, unsigned heads, unsigned sectors) ;
static void                 Delay(uint32_t msec) ;
static void                 DisplayAdjusts(void) ;
static void                 DisplayResults(void) ;
static char *               GetCylinders(unsigned cylinders) ;
static char *               GetHeads(unsigned heads) ;
static char *               GetSectors(unsigned sectors) ;
static unsigned             GetTimeout(uint32_t msec) ;
static void                 InitializeDisplay(void) ;
static void                 InitializeTouchScreen(void) ;
static void                 SetFontSize(sFONT *Font) ;
static void                 SetSlider(void) ;
static BOOL                 SliderMoved(unsigned x, unsigned y) ;
static void                 UpdateLBA(void) ;
static void                 UpdateLBAMax(void) ;
static void                 UpdateSlider(unsigned x) ;

static unsigned             lbamin      = 0 ;
static unsigned             lbaval      = 200000 ;
static unsigned             lbamax      = 1000000 ;
static unsigned             cylinders   = 768 ;
static unsigned             platters    = 4 ;
static unsigned             sectors     = 128 ;

static SLIDER slider = {LBA_XPOS, LBA_YPOS, LBA_WIDTH, LBA_HEIGHT, &lbaval, &lbamin, &lbamax} ;

static ADJUST adjusts[] =
    {
    {"#Cylinders:", ADJ_CYLS_XPOS,  ADJ_CYLS_YPOS,  &cylinders, GetCylinders,  512,  1024},
    {"#Heads:",     ADJ_HEADS_XPOS, ADJ_HEADS_YPOS, &platters,  GetHeads,        1,     8},
    {"#Sectors:",   ADJ_SCTRS_XPOS, ADJ_SCTRS_YPOS, &sectors,   GetSectors,     64,   256}
    } ;

int main(void)
    {
    unsigned x, y, delay1, delay2 ;

    InitializeHardware(NULL, "Lab 5E: Disk Drive Geometry") ;
    InitializeTouchScreen() ;
    InitializeDisplay() ;

    delay1 = delay2 = 0 ;
    while (1)
        {
        DisplayResults() ;
        Delay(delay1) ;
        delay1 = delay2 ;
        while (1)
            {
            if (TS_Touched())
                {
                x = TS_GetX() + TS_XFUDGE ;
                y = TS_GetY() + TS_YFUDGE ;

                if (SliderMoved(x, y)) break ;
                if (Adjusted(x, y))
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

static BOOL SliderMoved(unsigned x, unsigned y)
    {
    unsigned xmax, xmin ;
    float percent ;

    if (!Between(slider.ymin, y, slider.ymin + slider.height - 1))  return FALSE ;
    if (!Between(slider.xmin, x, slider.xmin + slider.width - 1))   return FALSE ;

    xmin = slider.xmin + SLIDER_WIDTH/2 ;
    if (x < xmin) x = xmin ;

    xmax = slider.xmin + slider.width - SLIDER_WIDTH/2 - 1 ;
    if (x > xmax) x = xmax ;

    percent = (float) (x - slider.xmin - SLIDER_WIDTH/2) / (slider.width - SLIDER_WIDTH - 1) ;
    *slider.pval = *slider.vmin + percent * (*slider.vmax - *slider.vmin) ;

    UpdateSlider(x) ;

    return TRUE ;
    }

static void UpdateSlider(unsigned x)
    {
    SetForeground(SLIDER_BGCLR) ;
    FillRect(slider.xpos - SLIDER_WIDTH/2 + 1, slider.ymin + 1, SLIDER_WIDTH, slider.height - 1) ;
    slider.xpos = x ;
    SetForeground(SLIDER_FGCLR) ;
    FillRect(slider.xpos - SLIDER_WIDTH/2 + 1, slider.ymin + 1, SLIDER_WIDTH, slider.height - 1) ;
    }

static void SetSlider(void)
    {
    UpdateSlider(slider.xmin + SLIDER_WIDTH/2 + (slider.width - SLIDER_WIDTH) * (*slider.pval - *slider.vmin)/(*slider.vmax - *slider.vmin)) ;
    }

static void UpdateLBA(void)
    {
    unsigned xpos, ypos ;
    char text[100] ;

    SetFontSize(&FONT_LBL) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    sprintf(text, "LBA = %07u", *slider.pval) ;
    xpos = (XPIXELS - strlen(text) * FONT_LBL.Width) / 2 ;
    ypos = slider.ymin - FONT_LBL.Height - 2 ;
    DisplayStringAt(xpos, ypos, text) ;
    }

static void UpdateLBAMax(void)
    {
    unsigned xpos, ypos ;
    char text[100] ;

    SetFontSize(&FONT_LBL) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    sprintf(text, "%07u", lbamax) ;
    xpos = XPIXELS - strlen(text) * FONT_LBL.Width - MARGIN ;
    ypos = slider.ymin + FONT_LBL.Height + 2 ;
    DisplayStringAt(xpos, ypos, text) ;
    }

static void DisplayResults(void)
    {
    char text[10] ;
    CHS phy ;

    SetSlider() ;
    UpdateLBA() ;

    SetFontSize(&FONT_OUT) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_YELLOW) ;

    Log2Phys(lbaval, 2*platters, sectors, &phy) ;

    sprintf(text, "%4u", phy.cylinder) ;
    DisplayStringAt(CYL_VAL_XPOS, CYL_YPOS, text) ;

    sprintf(text, "%4u", phy.head) ;
    DisplayStringAt(HEAD_VAL_XPOS, HEAD_YPOS, text) ;

    sprintf(text, "%4u", phy.sector) ;
    DisplayStringAt(SCTR_VAL_XPOS, SCTR_YPOS, text) ;
    }

static BOOL Adjusted(unsigned x, unsigned y)
    {
    ADJUST *adj ;
    int which ;

    adj = adjusts ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        if (Between(adj->ypos, y, adj->ypos + FONT_INP.Height - 1))
            {
            if (Between(adj->xpos + ADJOFF_XMINUS, x, adj->xpos + ADJOFF_XMINUS + FONT_INP.Width - 1))
                {
                if (*adj->pval > adj->min) --*adj->pval ;
                break ;
                }

            if (Between(adj->xpos + ADJOFF_XPLUS, x, adj->xpos + ADJOFF_XPLUS + FONT_INP.Width - 1))
                {
                if (*adj->pval < adj->max) ++*adj->pval ;
                break ;
                }
            }
        }

    if (which == ENTRIES(adjusts)) return FALSE ;

    DisplayAdjusts() ;
    return TRUE ;
    }

static char *GetCylinders(unsigned cylinders)
    {
    static char text[10] ;
    sprintf(text, "%4u", cylinders) ;
    return text ;
    }
static char *GetHeads(unsigned platters)
    {
    static char text[10] ;
    sprintf(text, "%4u", 2*platters) ;
    return text ;
    }

static char *GetSectors(unsigned sectors)
    {
    static char text[10] ;
    sprintf(text, "%4u", sectors) ;
    return text ;
    }

static void InitializeDisplay(void)
    {
    unsigned which, ypos ;
    ADJUST *adj ;

    SetForeground(SLIDER_BGCLR) ;
    FillRect(slider.xmin + 1, slider.ymin, slider.width - 1, slider.height) ;
    SetForeground(COLOR_RED) ;
    DrawRect(slider.xmin, slider.ymin, slider.width, slider.height) ;
    SetSlider() ;
    UpdateLBA() ;
    UpdateLBA() ;

    SetFontSize(&FONT_LBL) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    ypos = slider.ymin + slider.height + 2 ;
    DisplayStringAt(slider.xmin, ypos, "0") ;
    UpdateLBAMax() ;

    adj = adjusts ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        SetFontSize(&FONT_LBL) ;
        DisplayStringAt(adj->xpos + ADJOFF_XLABEL, adj->ypos + VERT_INP_OFF, adj->lbl) ;
        SetFontSize(&FONT_INP) ;

        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_BLACK) ;
        DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        DrawRect(adj->xpos + ADJOFF_XMINUS - 1, adj->ypos - 1, 6*FONT_INP.Width + 1, FONT_INP.Height + 1) ;

        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_BLACK) ;
        DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
        }

    SetForeground(COLOR_YELLOW) ;
    FillRect(RSLT_XPOS, RSLT_YPOS, RSLT_HSIZ, RSLT_VSIZ) ;
    SetForeground(COLOR_BLACK) ;
    DrawRect(RSLT_XPOS, RSLT_YPOS, RSLT_HSIZ, RSLT_VSIZ) ;

    SetFontSize(&FONT_LBL) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_YELLOW) ;
    DisplayStringAt(CYL_LBL_XPOS, CYL_YPOS + VERT_OUT_OFF,   "Physical Cyl:") ;
    DisplayStringAt(HEAD_LBL_XPOS, HEAD_YPOS + VERT_OUT_OFF, "Physical Head:") ;
    DisplayStringAt(SCTR_LBL_XPOS, SCTR_YPOS + VERT_OUT_OFF, "Physical Sector:") ;
    
    SetBackground(COLOR_WHITE) ;
    DisplayStringAt(CAPACITY_LABEL, CAPACITY_YPOS, "Capacity:") ;

    DisplayAdjusts() ;
    }

static void DisplayAdjusts(void)
    {
    ADJUST *adj ;
    int which ;

    SetFontSize(&FONT_INP) ;
    adj = adjusts ;
    for (which = 0; which < ENTRIES(adjusts); which++, adj++)
        {
        SetForeground(COLOR_WHITE) ;
        SetBackground(*adj->pval < adj->max ? COLOR_DARKGREEN : COLOR_RED) ;
        DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
        SetBackground(*adj->pval > adj->min ? COLOR_DARKGREEN : COLOR_RED) ;
        DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_WHITE) ;
        DisplayStringAt(adj->xpos + ADJOFF_XVALUE, adj->ypos, (*adj->text)(*adj->pval)) ;
        }

    SetFontSize(&FONT_LBL) ;
    DisplayStringAt(CAPACITY_XPOS, CAPACITY_YPOS, Capacity(cylinders, 2*platters, sectors)) ;
    UpdateLBAMax() ;
    UpdateLBA() ;
    }

static char *Capacity(unsigned cylinders, unsigned heads, unsigned sectors)
    {
    static char text[20] ;
    unsigned bytes ;

    lbamax = cylinders * heads * sectors - 1 ;
    if (lbaval > lbamax) lbaval = lbamax ;

    bytes = (lbamax + 1) * 512 ;
    sprintf(text, "%5u KB", bytes / 1024) ;
    if (strlen(text) <= 8) return text ;
    sprintf(text, "%5.1f MB", (float) bytes / (1024*1024)) ;
    if (strlen(text) <= 8) return text ;
    sprintf(text, "%5.2f GB", (float) bytes / (1024*1024*1024)) ;
    return text ;
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

static void SetFontSize(sFONT *Font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(Font) ;
    }

static unsigned GetTimeout(uint32_t msec)
    {
    unsigned cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
    return GetClockCycleCount() + cycles ;
    }

static void Delay(uint32_t msec)
    {
    uint32_t timeout = GetTimeout(msec) ;
    while ((int) (timeout - GetClockCycleCount()) > 0) ;
    }

static BOOL Between(unsigned min, unsigned val, unsigned max)
    {
    return (min <= val && val <= max) ;
    }

