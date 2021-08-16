    .syntax     unified
    .cpu        cortex-m4
    .text



// FullWordAccess(int32_t *src)
    .global     FullWordAccess
    .thumb_func
    .align
FullWordAccess:
    .rept       100             // repeat 100 times 
    LDR         R1,[R0]         // load 32-bit value from R0 into R1
    .endr                       // end repeat
    BX          LR



// HalfWordAccess(int16_t *src)
    .global     HalfWordAccess
    .thumb_func
    .align
HalfWordAccess:
    .rept       100             // repeat 100 times
    LDRH        R1,[R0]         // load 16-bit value from R0 into R1
    .endr                       // end repeat
    BX          LR



// AddressDependency(uint32_t *src)
    .global     AddressDependency
    .thumb_func
    .align
AddressDependency:
    .rept       100             // repeat 100 times
    LDR         R1,[R0]         // load 32-bit value from R0 into R1
    LDR         R0,[R1]         // load 32-bit value from R1 into R0
    .endr                       // end repeat
    BX          LR



// NoAddressDependency(uint32_t *src)
    .global     NoAddressDependency
    .thumb_func
    .align
NoAddressDependency:
    .rept       100             // repeat 100 times
    LDR         R1,[R0]         // load 32-bit value from R0 into R1
    LDR         R2,[R0]         // load 32-bit value from R0 into R2
    .endr                       // end repeat
    BX          LR



// DataDependency(float f1)
    .global     DataDependency
    .thumb_func
    .align
DataDependency:
    .rept       100             // repeat 100 times
    VADD.F32    S1,S0,S0        // S0 + S0 into S1 (add floating points)
    VADD.F32    S0,S1,S1        // S1 + S1 into S0 (add floating points)
    .endr                       // end repeat
    VMOV        S1,S0           // copy (move) S0 to S1
    BX          LR



// NoDataDependency(float f1)
    .global     NoDataDependency
    .thumb_func
    .align
NoDataDependency:
    .rept       100             // repeat 100 times
   VADD.F32     S1,S0,S0        // S0 + S0 into S1 (add floating points)
   VADD.F32     S2,S0,S0        // S0 + S0 into S2 (add floating points)
   .endr                        // end repeat
   VMOV         S1,S0           // copy (move) S0 to S1
   BX           LR



// VDIVOverlap(float dividend, float divisor)
    .global     VDIVOverlap
    .thumb_func
    .align
VDIVOverlap:
    VDIV.F32    S2,S1,S0        // S1 / S0 into S2 (divide floating points)
    .rept       1               // repeat 1 time
    NOP                         // no operation
    .endr                       // end repeat
    VMOV        S3,S2           // copy (move) S2 to S3
    BX          LR