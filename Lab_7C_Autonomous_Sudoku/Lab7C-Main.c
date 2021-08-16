/*
    This code was written to support the book, "ARM Assembly for Embedded Applications",
    by Daniel W. Lewis. Permission is granted to freely share this software provided
    that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "library.h"
#include "graphics.h"
#include "touch.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

// Functions to be implemented in assembly
uint32_t __attribute__((weak)) GetNibble(void *nibbles, uint32_t which)
    {
/*    uint8_t byte ;

    byte = ((uint8_t *) nibbles)[which >> 1] ;
    if ((which & 1) == 1) byte >>= 4 ;
    return (uint32_t) (byte & 0b00001111) ; */
    }

void __attribute__((weak)) PutNibble(void *nibbles, uint32_t which, uint32_t value)
    {
/*    uint8_t *pbyte ;

    pbyte = (uint8_t *) nibbles + (which >> 1) ;

    if ((which & 1) == 1)
        {
        *pbyte &= 0b00001111 ;
        *pbyte |= value << 4 ;      
        }
    else
        {
        *pbyte &= 0b11110000 ;
        *pbyte |= value ;
        }    */
    }

#pragma GCC pop_options

typedef enum {FALSE = 0, TRUE = 1} BOOL ;

typedef struct
    {
    char *              status ;
    unsigned            initial ;
    unsigned            placed ;
    unsigned            removed ;
    unsigned            getCalls ;
    unsigned            putCalls ;
    unsigned            getCycles ;
    unsigned            putCycles ;
    float               elapsed ;
    } REPORT ;

typedef struct _tFont
    {
    const uint8_t *     table ;
    const uint16_t      Width ;
    const uint16_t      Height ;
    } sFONT;

extern sFONT            Font8 ;
extern sFONT            Font12 ;   // Smaller font used in footer
extern sFONT            Font16 ;   // Larger font used in header
extern sFONT            Font20 ;
extern sFONT            Font24 ;   // Largest font used for game

// Functions private to the main program
static int              Cell2Fill(int index) ;
static void             ClearFlags(int row, int col, int digit) ;
static BOOL             Conflict(int row, int col, int digit) ;
static void             DisplayBoard(void) ;
static void             DisplayCell(int row, int col, int digit) ;
static void             DisplayResults(REPORT *report) ;
static void             DrawGrid(void) ;
static void             EditConfiguration(void) ;
static void             InitializeGame(void) ;
static void             InitializeFlags(void) ;
static void             InitializeStats(void) ;
static void             InitializeTouchScreen(void) ;
static void             LEDs(int grn_on, int red_on) ;
static void             RandomizeGame(void) ;
static void             RandomizeMajor(void (*Swap)(int major1, int major2)) ;
static void             RandomizeMinor(void (*Swap)(int minor1, int minor2)) ;
static int              ReportHeader(int row, sFONT *font, char *text, int lines) ;
static int              ReportLine(int row, sFONT *font, char *fmt, ...) ;
static int              SanityChecksOK(void) ;
static void             SetFlags(int row, int col, int digit) ;
static void             SetFontSize(sFONT *font) ;
static int              SolvePuzzle(int index, int count) ;
static void             SwapCols(int col1, int col2) ;
static void             SwapRows(int row1, int row2) ;

#define TOP_EDGE        56
#define LFT_EDGE        10

#define ROWS            9
#define COLS            9
#define BLKS            9

#define CELLS           (ROWS*COLS)
#define WORDS           (CELLS + 7)/8

#define CELL_HEIGHT     25
#define CELL_WIDTH      23

#define HORZ_OFFSET     (LFT_EDGE + 3)
#define VERT_OFFSET     (TOP_EDGE + 1)

#define INDEX(row, col) ((row)*COLS+(col))
#define ENTRIES(a)      (sizeof(a)/sizeof(a[0]))

#define REPORT_XPOS     20
#define REPORT_YPOS     55
#define REPORT_WIDTH    18

#define EMPTY           0

static uint32_t storage[WORDS] ;
static uint32_t initial[WORDS] =
    {
    0x00900001, 0x02003007, 0x00060009, 0x03080100, 0x09009070,
    0x10200801, 0x05050400, 0x00010000, 0x00500209, 0x00005006,
    0x00000003
    } ;
static REPORT           report ;

#define FLAGS_ROWS      0
#define FLAGS_COLS      1
#define FLAGS_BLKS      2

static uint32_t         flags[3][9] ;
static uint32_t         digit_foreground ;
static uint32_t         digit_background ;

int main()
    {
    InitializeHardware(HEADER, "Lab 7C: Autonomous Sudoku") ;
    InitializeTouchScreen() ;

    if (!SanityChecksOK()) return 255 ;

    while (1)
        {
        unsigned cells_filled, strt, stop ;

        InitializeStats() ;
        RandomizeGame() ;
        DisplayBoard() ;
        InitializeFlags() ;
        InitializeGame() ;
        EditConfiguration() ;
        WaitForPushButton() ;   // Wait for user to start the algorithm

        digit_foreground = COLOR_BLUE ;
        digit_background = COLOR_WHITE ;

        strt = GetClockCycleCount() ;
        cells_filled = SolvePuzzle(0, report.initial) ;
        stop = GetClockCycleCount() ;
        report.elapsed = (stop - strt) / 168000000.0 ;

        if (cells_filled < CELLS)
            {
            report.status = "Failed" ;
            }
        else if (cells_filled == CELLS)
            {
            WaitForPushButton() ;
            report.status = "Solved" ;
            }
        else report.status = "Abort!" ;

        DisplayResults(&report) ;
        WaitForPushButton() ;
        }

    return 0 ;
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

static void InitializeStats(void)
    {
    unsigned strt, stop, ovhd ;

    memset(&report, 0, sizeof(report)) ;

    strt = GetClockCycleCount() ;
    stop = GetClockCycleCount() ;
    ovhd = stop - strt ;

    strt = GetClockCycleCount() ;
    GetNibble(storage, 0) ;
    stop = GetClockCycleCount() ;
    report.getCycles = stop - strt - ovhd ;

    strt = GetClockCycleCount() ;
    PutNibble(storage, 0, EMPTY) ;
    stop = GetClockCycleCount() ;
    report.putCycles = stop - strt - ovhd ;
    }

static int ReportHeader(int row, sFONT *font, char *title, int lines)
    {
    int height = 1 + (++lines * font->Height) + font->Height/4 ;
    int width  = 1 + REPORT_WIDTH*font->Width ;
    DrawRect(REPORT_XPOS - 1, row - 1, width, height) ;
    FillRect(REPORT_XPOS, row, REPORT_WIDTH*font->Width, font->Height) ;
    SetForeground(COLOR_WHITE) ;
    SetBackground(COLOR_BLACK) ;
    DisplayStringAt(REPORT_XPOS + font->Width, row, title) ;
    return row + font->Height + font->Height/4 ;
    }

static int ReportLine(int row, sFONT *font, char *format, ...)
    {
    char text[100] ;
    va_list args ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    va_start(args, format) ;
    vsnprintf(text, sizeof(text), format, args);
    DisplayStringAt(REPORT_XPOS + font->Width, row, text) ;
    va_end(args) ;
    return row + font->Height ;
    }

static void DisplayResults(REPORT *report)
    {
    sFONT *font = &Font16 ;
    int row ;

    ClearDisplay() ;
    SetFontSize(font) ;

    row = REPORT_YPOS ;

    row = ReportHeader(row, font, "PUZZLE RESULTS", 2) ;
    row = ReportLine(row, font, "   Status:%s", report->status) ;
    row = ReportLine(row, font, "  Elapsed:%.2fs", report->elapsed) ;

    row += 6 ;

    row = ReportHeader(row, font, "DIGIT PLACEMENTS", 3) ;
    row = ReportLine(row, font, "  Initial:%u", report->initial) ;
    row = ReportLine(row, font, " Attempts:%u", report->placed) ;
    row = ReportLine(row, font, " Removals:%u", report->removed) ;

    row += 6 ;

    row = ReportHeader(row, font, "FUNCTION CALLS", 2) ;
    row = ReportLine(row, font, "GetNibble:%u", report->getCalls) ;
    row = ReportLine(row, font, "PutNibble:%u", report->putCalls) ;

    row += 6 ;

    row = ReportHeader(row, font, "CLOCK CYCLES", 2) ;
    row = ReportLine(row, font, "GetNibble:%u", report->getCycles) ;
    row = ReportLine(row, font, "PutNibble:%u", report->putCycles) ;
    }

static void SetFontSize(sFONT *font)
    {
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(font) ;
    }

static void InitializeGame(void)
    {
    for (int word = 0; word < WORDS; word++)
        {
        storage[word] = initial[word] ;
        }
    }

static int SolvePuzzle(int index, int cells_filled)
    {
    int row, col ;

    // Check for user abort
    if (PushButtonPressed())
        {
        WaitForPushButton() ;
        return CELLS + 1 ;
        }

    if (cells_filled >= CELLS)
        {
        return cells_filled ;
        }

    report.getCalls++ ;
    if (GetNibble(storage, index) != EMPTY)
        {
        cells_filled = SolvePuzzle(Cell2Fill(index), cells_filled) ;
        return cells_filled ;
        }

    /*
     * Iterate through the possible digits for this empty cell
     * and recurse for every valid one, to test if it's part
     * of the valid solution.
     */
    row = index / ROWS ;
    col = index % COLS ;

    for (int digit = 1; digit <= 9; digit++)
        {
        int new_filled ;

        if (Conflict(row, col, digit)) continue ;

        SetColor(COLOR_RED) ;
        PutNibble(storage, index, digit) ;
        DisplayCell(row, col, digit) ;
        SetFlags(row, col, digit) ;
        report.placed++ ;
        report.putCalls++ ;

        new_filled = SolvePuzzle(Cell2Fill(index), cells_filled + 1) ;
        if (new_filled >= CELLS)
            {
            SetColor(COLOR_BLUE) ;
            DisplayCell(row, col, digit) ;
            return new_filled ;
            }

        ClearFlags(row, col, digit) ;
        }

    PutNibble(storage, index, EMPTY) ;
    DisplayCell(row, col, EMPTY) ;
    report.removed++ ;
    report.putCalls++ ;
    return cells_filled ;
    }

static void DisplayCell(int row, int col, int digit)
    {
    static int pxlrow[] =
        {
         2 + 0*CELL_HEIGHT,  3 + 1*CELL_HEIGHT,  4 + 2*CELL_HEIGHT,
         6 + 3*CELL_HEIGHT,  7 + 4*CELL_HEIGHT,  8 + 5*CELL_HEIGHT,
        10 + 6*CELL_HEIGHT, 11 + 7*CELL_HEIGHT, 12 + 8*CELL_HEIGHT
        } ;
    static int pxlcol[] =
        {
         2 + 0*CELL_WIDTH,  3 + 1*CELL_WIDTH,  4 + 2*CELL_WIDTH,
         6 + 3*CELL_WIDTH,  7 + 4*CELL_WIDTH,  8 + 5*CELL_WIDTH,
        10 + 6*CELL_WIDTH, 11 + 7*CELL_WIDTH, 12 + 8*CELL_WIDTH
        } ;

    if (digit == EMPTY)
        {
        SetForeground(COLOR_WHITE) ;
        FillRect(pxlcol[col] + HORZ_OFFSET - 3, pxlrow[row] + VERT_OFFSET - 1, Font24.Width + 6, Font24.Height + 1) ;
        }
    else
        {
        SetForeground(digit_background) ;
        FillRect(pxlcol[col] + HORZ_OFFSET - 3, pxlrow[row] + VERT_OFFSET - 1, Font24.Width + 6, Font24.Height + 1) ;
        SetForeground(digit_foreground) ;
        SetBackground(digit_background) ;
        DisplayChar(pxlcol[col] + HORZ_OFFSET, pxlrow[row] + VERT_OFFSET, digit + '0') ;
        }
    }

static void DisplayBoard(void)
    {
    ClearDisplay() ;
    DrawGrid() ;
    SetFontSize(&Font24) ;
    digit_foreground = COLOR_BLACK ;
    digit_background = COLOR_LIGHTGRAY ;
    for (int row = 0; row < ROWS; row++)
        {
        for (int col = 0; col < COLS; col++)
            {
            DisplayCell(row, col, GetNibble(initial, INDEX(row, col))) ;
            }
        }
    }

static void DrawGrid(void)
    {
    typedef struct { int r, c, w, h ;} LINE ;
    static LINE line[] =
        {
        { 0 + 0*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 2},
        { 2 + 1*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        { 3 + 2*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        { 4 + 3*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 2},
        { 6 + 4*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        { 7 + 5*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        { 8 + 6*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 2},
        {10 + 7*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        {11 + 8*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 1},
        {12 + 9*CELL_HEIGHT, 0, 14 + COLS*CELL_WIDTH, 2},

        {0,  0 + 0*CELL_WIDTH, 2, 14 + ROWS*CELL_HEIGHT},
        {0,  2 + 1*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0,  3 + 2*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0,  4 + 3*CELL_WIDTH, 2, 14 + ROWS*CELL_HEIGHT},
        {0,  6 + 4*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0,  7 + 5*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0,  8 + 6*CELL_WIDTH, 2, 14 + ROWS*CELL_HEIGHT},
        {0, 10 + 7*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0, 11 + 8*CELL_WIDTH, 1, 14 + ROWS*CELL_HEIGHT},
        {0, 12 + 9*CELL_WIDTH, 2, 14 + ROWS*CELL_HEIGHT},
        } ;
    LINE *l = line ;

    SetColor(COLOR_BLACK) ;
    for (int which = 0; which < ENTRIES(line); which++, l++)
        {
        FillRect(l->c + LFT_EDGE, l->r + TOP_EDGE, l->w, l->h) ;
        }
    }

// Checks to see if a particular digit is valid in a given position.
static BOOL Conflict(int row, int col, int digit)
    {
    uint32_t all_flags ;
    int blk ;

    if (digit == EMPTY) return FALSE ;

    blk = 3*(row/3) + col/3 ;

    all_flags = flags[FLAGS_ROWS][row] | flags[FLAGS_COLS][col] | flags[FLAGS_BLKS][blk] ;
    return (all_flags & (1 << digit)) != 0 ;
    }

static int SanityChecksOK(void)
    {
    uint32_t index, word, left , bugs ;

    for (int i = 0; i < WORDS; i++) storage[i] = 0 ;

    bugs = 0 ;

    do index = GetRandomNumber() % CELLS ; while (index < 8) ;
    PutNibble(storage, index, 0xF) ;
    word = index / 8 ;
    left  = index % 8 ;
    if (storage[word] != (0xF << 4*left)) bugs |= 0x1 ;
    storage[word] = 0 ;

    do index = GetRandomNumber() % CELLS ; while (index < 8) ;
    word = index / 8 ;
    left  = index % 8 ;
    storage[word] = 0xF << 4*left ;
    if (GetNibble(storage, index) != 0xF) bugs |= 0x2 ;
    storage[word] = 0 ;

    LEDs(!bugs, bugs) ;
    if (!bugs) return 1 ;

    SetForeground(COLOR_WHITE) ;
    SetBackground(COLOR_RED) ;
    if (bugs & 0x1) DisplayStringAt(5, 50, (uint8_t *) " Bad Function PutNibble\n") ;
    if (bugs & 0x2) DisplayStringAt(5, 70, (uint8_t *) " Bad Function GetNibble\n") ;
    return 0 ;
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

static void TS_Delay(unsigned clocks)
    {
#   define  DWT_CYCCNT ((volatile uint32_t *)   0xE0001004)
    uint32_t timeout = *DWT_CYCCNT + clocks ;
    while ((timeout - *DWT_CYCCNT) > 100) ;
    }

static void EditConfiguration(void)
    {
#   define MARGIN   2
    static int ymins[] =
        {
         2 + 0*CELL_HEIGHT,  3 + 1*CELL_HEIGHT,  4 + 2*CELL_HEIGHT,
         6 + 3*CELL_HEIGHT,  7 + 4*CELL_HEIGHT,  8 + 5*CELL_HEIGHT,
        10 + 6*CELL_HEIGHT, 12 + 7*CELL_HEIGHT, 13 + 8*CELL_HEIGHT
        } ;
    static int xmins[] =
        {
         2 + 0*CELL_WIDTH,   3 + 1*CELL_WIDTH,   4 + 2*CELL_WIDTH,
         6 + 3*CELL_WIDTH,   7 + 4*CELL_WIDTH,   8 + 5*CELL_WIDTH,
        10 + 6*CELL_WIDTH,  11 + 7*CELL_WIDTH,  12 + 8*CELL_WIDTH
        } ;

    SetFontSize(&Font24) ;
    digit_foreground = COLOR_WHITE ;
    digit_background = COLOR_LIGHTGRAY ;
    while (!PushButtonPressed())
        {
        int x, y, digit, row, col ;

        if (!TS_Touched()) continue ;

        x = TS_GetX() ;
        y = TS_GetY() ;

        do TS_Delay(8000000) ;
        while (TS_Touched()) ;

        // Find row
        for (row = 0; row < ROWS; row++)
            {
            int ymin = ymins[row] + TOP_EDGE + MARGIN ;
            int ymax = ymin + CELL_HEIGHT - 1 - 2*MARGIN ;
            if (ymin <= y && y <= ymax) break ;
            }
        if (row == ROWS) continue ;

        for (col = 0; col < COLS; col++)
            {
            int xmin = xmins[col] + LFT_EDGE + MARGIN ;
            int xmax = xmin + CELL_WIDTH - 1 - 2*MARGIN ;
            if (xmin <= x && x <= xmax)  break ;
            }
        if (col == COLS) continue ;

        digit = GetNibble(storage, INDEX(row, col)) ;
        if (digit != EMPTY) report.initial-- ;
        ClearFlags(row, col, digit) ;

        do digit = (digit + 1) % 10 ;
        while (Conflict(row, col, digit)) ;

        if (digit != EMPTY) report.initial++ ;
        SetFlags(row, col, digit) ;
        PutNibble(storage, INDEX(row, col), digit) ;
        DisplayCell(row, col, digit) ;
        }
    }

static void RandomizeGame(void)
    {
    RandomizeMajor(SwapRows) ;
    RandomizeMajor(SwapCols) ;
    RandomizeMinor(SwapRows) ;
    RandomizeMinor(SwapCols) ;
    }

static void RandomizeMajor(void (*Swap)(int, int))
    {
    int major1 = 3 * (GetRandomNumber() % 3) ;
    int major2 = 3 * (GetRandomNumber() % 3) ;

    if (major1 == major2) return ;

    for (int minor = 0; minor < 3; minor++)
        {
        (*Swap)(major1++, major2++) ;
        }
    }

static void RandomizeMinor(void (*Swap)(int, int))
    {
    for (int block = 0; block < BLKS; block++)
        {
        int minor1 = 3*(block / 3) + (GetRandomNumber() % 3) ;
        int minor2 = 3*(block / 3) + (GetRandomNumber() % 3) ;
        if (minor1 != minor2) (*Swap)(minor1, minor2) ;
        }
    }

static void SwapRows(int row1, int row2)
    {
    int idx1 = COLS*row1 ;
    int idx2 = COLS*row2 ;
    for (int col = 0; col < COLS; col++)
        {
        uint32_t cell1 = GetNibble(initial, idx1) ;
        uint32_t cell2 = GetNibble(initial, idx2) ;
        PutNibble(initial, idx1, cell2) ;
        PutNibble(initial, idx2, cell1) ;
        idx1 += 1 ;
        idx2 += 1 ;
        }
    }

static void SwapCols(int col1, int col2)
    {
    int idx1 = 1*col1 ;
    int idx2 = 1*col2 ;
    for (int row = 0; row < ROWS; row++)
        {
        uint32_t cell1 = GetNibble(initial, idx1) ;
        uint32_t cell2 = GetNibble(initial, idx2) ;
        PutNibble(initial, idx1, cell2) ;
        PutNibble(initial, idx2, cell1) ;
        idx1 += COLS ;
        idx2 += COLS ;
        }
    }

static void InitializeFlags(void)
    {
    memset(flags, 0, sizeof(flags)) ;
    for (int index = 0; index < CELLS; index++)
        {
        int digit = GetNibble(initial, index) ;

        report.getCalls++ ;
        if (digit != EMPTY)
            {
            int bit = 1 << digit ;
            int row = index / COLS ;
            int col = index % COLS ;
            int blk = 3*(row/3) + col/3 ;

            flags[FLAGS_ROWS][row] |= bit ;
            flags[FLAGS_COLS][col] |= bit ;
            flags[FLAGS_BLKS][blk] |= bit ;

            report.initial++ ;
            }
        }
    }

static void ClearFlags(int row, int col, int digit)
    {
    uint32_t bit = 1 << digit ;
    int blk = 3*(row/3) + col/3 ;
    flags[FLAGS_ROWS][row] &= ~bit ;
    flags[FLAGS_COLS][col] &= ~bit ;
    flags[FLAGS_BLKS][blk] &= ~bit ;
    }

static void SetFlags(int row, int col, int digit)
    {
    uint32_t bit = 1 << digit ;
    int blk = 3*(row/3) + col/3 ;
    flags[FLAGS_ROWS][row] |= bit ;
    flags[FLAGS_COLS][col] |= bit ;
    flags[FLAGS_BLKS][blk] |= bit ;
    }

static int Cell2Fill(int index)
    {
    return (index + 1) % CELLS ;
    }

