    .syntax     unified
    .cpu        cortex-m4
    .text



// Zeller1(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    .global     Zeller1
    .thumb_func
    .align
Zeller1:
    PUSH        {R4,R5,LR}              // preserve R4, R5, LR
    LDR         R4,=2;                  // R4 <-- 2
    MUL         R4,R3,R4                // 2*C
    LDR         R5,=4                   // R5 <-- 4
    UDIV        R3,R3,R5                // R3 <-- C/4
    ADD         R2,R2,R2,LSR 2          // R2 <-- D + (D/4)
    ADD         R2,R2,R3                // R2 <-- (C/4) + D + (D/4)
    SUB         R2,R2,R4                // R2 <-- (C/4) + D + (D/4) - (2*C)
    ADD         R2,R2,R0                // R2 <-- k + (C/4) + D + (D/4) - (2*C)
    LDR         R0,=13                  // R0 <-- 13
    MUL         R1,R1,R0                // R1 <-- m*13
    SUB         R1,R1,1                 // R1 <-- (m*13)-1
    LDR         R0,=5                   // R0 <-- 5
    UDIV        R1,R1,R0                // R1 <-- ((m*13)-1)/5
    ADD         R1,R1,R2                // R1 <-- k + ((m*13)-1)/5 + (C/4) + D + (D/4) - (2*C) = f
    LDR         R2,=7                   // R2 <-- 7 (divisor)
    SDIV        R0,R1,R2                // R0 <-- f / 7 (quotient)
    MLS         R0,R0,R2,R1             // R0 <-- R1 - (R0*R2)
    CMP         R0,0                    // remained < 0?
    IT          LT                      // if less than,
    ADDLT       R0,R0,7                 // (remainder < 0) ? 7:0
    POP         {R4,R5,PC}              // restore R4, R5, PC



// Zeller2(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    .global     Zeller2
    .thumb_func
    .align
Zeller2:
    PUSH        {R4,R5,LR}              // preserve R4, R5, LR
    LSR         R4,R2,2                 // R4 <-- D/4
    ADD         R2,R2,R4                // R2 <-- D + (D/4)
    LSR         R4,R3,2                 // R4 <-- C/4
    LDR         R5,=2                   // R5 <-- 2
    MLS         R3,R3,R5,R4             // R3 <-- (C/4) - (2*C)
    LDR         R4,=13                  // R4 <-- 13
    MUL         R1,R1,R4                // R1 <-- 13 * m
    SUB         R1,R1,1                 // R1 <-- (13*m) - 1
    LDR         R4,=3435973837          // R4 <-- (2^32) / 5
    UMULL       R5,R4,R4,R1             // R5.R4 <-- (x*(2^n/5))
    LSR         R1,R4,2                 // R1 <-- R4 / 4
    ADD         R0,R0,R1                // R0 <-- k + (13*m-1)/5
    ADD         R0,R0,R2                // R0 <-- k + (13*m-1)/5 + D + (D/4)
    ADD         R0,R0,R3                // R0 <-- k + ((m*13)-1)/5 + (C/4) + D + (D/4) - (2*C) = f
    LDR         R1,=613566757           // R1 <-- (2^32)/7
    UMULL       R2,R1,R1,R0             // R2.R1 <-- (f * (2^n/7))
    ADDS        R1,R1,R0                // R1 <-- R1 + R0 (record carry)
    RRX         R1,R1                   // R1 <-- shift right, fill with carry
    LSR         R3,R1,2                 // R3 <-- R1 / 4
    LDR         R1,=7                   // R1 <-- 7 (divisor)
    MLS         R2,R3,R1,R0             // R2 <-- R0 - (R3*R1)
    AND         R3,R1,R2,ASR 31         // (remainder < 0) ? 7:0
    ADDS.N      R0,R2,R3                // if remainder is < 0, add divisor
    POP         {R4,R5,PC}              // restore R4, R5, PC



    // Zeller3(uint32_t k, uint32_t m, uint32_t D, uint32_t C)
    .global     Zeller3
    .thumb_func
    .align
Zeller3:
    PUSH        {R4,LR}                 // preserve R4, LR
    ADD         R2,R2,R2,LSR 2          // R2 <-- D + (D/4)
    LSR         R4,R3,2                 // R4 <-- C/4
    LSL         R3,R3,1                 // R3 <-- 2*C
    RSB         R3,R3,R4                // R3 <-- (C/4) - (2*C)
    ADD         R4,R1,R1,LSL 4          // R4 <-- m + (16*m) = 17*m
    SUB         R1,R4,R1,LSL 2          // R1 <-- (17*m) - (4*m) = 13*m
    SUB         R1,R1,1                 // R1 <-- (13*m) - 1
    LDR         R4,=5                   // R4 <-- 5
    UDIV        R1,R1,R4                // R1 <-- (13*m-1)/5
    ADD         R0,R0,R1                // R0 <-- k + (13*m-1)/5
    ADD         R0,R0,R2                // R0 <-- k + (13*m-1)/5 + D + (D/4)
    ADD         R0,R0,R3                // R0 <-- k + ((m*13)-1)/5 + (C/4) + D + (D/4) - 2*C = f
    LDR         R1,=7                   // R1 <-- 7
    SDIV        R3,R0,R1                // R3 <-- f / 7
    ADD         R2,R3,R3,LSL 2          // R2 <-- (f/7) + 4*(f/7) = 5*(f/7)
    ADD         R2,R2,R3,LSL 1          // R2 <-- 5*(f/7) + 2*(f/7) = 7*(f/7)
    SUB         R2,R0,R2                // R2 <-- f - (7 * (f/7))
    AND         R3,R1,R2,ASR 31         // (remainder < 0) ? 7:0
    ADDS.N      R0,R2,R3                // if remainder is < 0, add divisor
    POP         {R4,PC}                 // restore R4, PC



    .end