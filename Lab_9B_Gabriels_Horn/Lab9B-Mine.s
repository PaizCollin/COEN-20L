    .syntax     unified
    .cpu        cortex-m4
    .text



// Integrate(void)
    .global Integrate
    .thumb_func
    .align
Integrate:
    PUSH        {R4,LR}                 // preserve LR, R4
    VPUSH       {S16,S17,S18,S19,S20,S21}
    BL          DeltaX                  // call func DeltaX
    VMOV        S5,S0                   // S5 <-- S0 (dx)
    VSUB.F32    S1,S1,S1                // S1 <-- 0.0 (v)
    VSUB.F32    S2,S2,S2                // S2 <-- 0.0 (a)
    VMOV        S3,1.0                  // S3 <-- 1.0 (x)
    VMOV        S0,1.0                  // S0 <-- 1.0 (r)
    LDR         R0,=0                   // R0 <-- 0 (n)

L1: 
    MOV         R4,R0                   // R4 <-- R0 (n)
    VMOV        S16,S0                  // S16 <-- S0 (r)
    VMOV        S17,S1                  // S17 <-- S1 (v)
    VMOV        S18,S2                  // S18 <-- S2 (a)
    VMOV        S19,S3                  // S19 <-- S3 (x)
    VMOV        S20,S5                  // S20 <-- S5 (dx)
    BL          UpdateDisplay           // call func UpdateDisplay
    MOV         R0,R4                   // R0 <-- R4 (n)
    VMOV        S0,S16                  // S0 <-- S16 (r)
    VMOV        S1,S17                  // S1 <-- S17 (v)
    VMOV        S2,S18                  // S2 <-- S18 (a)
    VMOV        S3,S19                  // S3 <-- S19 (x)
    VMOV        S5,S20                  // S5 <-- S20 (dx)

    VMOV        S4,S1                   // prev = v

    VMOV        S6,1.0                  // S6 <-- 1.0
    VDIV.F32    S7,S6,S3                // S7 <-- 1 / x
    VADD.F32    S8,S5,S3                // S8 <-- x + dx
    VDIV.F32    S8,S6,S8                // S8 <-- 1 / (x + dx)
    VADD.F32    S8,S7,S8                // S8 <-- 1/x + 1/(x + dx)
    VMOV        S7,2.0                  // S7 <-- 2.0
    VDIV.F32    S0,S8,S7                // r = 1/x + 1/(x + dx)
    
    VMUL.F32    S6,S0,S0                // S6 <-- r * r
    VADD.F32    S1,S1,S6                // v += r * r
    VADD.F32    S2,S2,S0                // a += r
    ADD         R0,R0,1                 // n++
    VADD.F32    S3,S3,S5                // x += dx 
    VCMP.F32    S1,S4                   // v != prev
    VMRS        APSR_nzcv,FPSCR         // move flags
    BNE         L1                      // v != prev, loop again

    VPOP        {S16,S17,S18,S19,S20,S21}
    POP         {R4,PC}                 // return



    .end