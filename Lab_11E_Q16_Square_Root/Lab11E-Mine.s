    .syntax     unified
    .cpu        cortex-m4
    .text



// Q16GoodRoot(Q16 radicand)
    .global Q16GoodRoot
    .thumb_func
    .align
Q16GoodRoot:                        // R0 <-- radicand
    PUSH        {LR,R4,R5}          // preserve LR, R4, R5
    CLZ         R4,R0               // __CLZ(radicand)
    BIC         R4,R4,1             // R4 <-- R4 & ~1
    LDR         R5,=1               // R5 <-- 1
    LSL         R5,R5,30            // R5 <-- R5 << 30
    LSR         R3,R5,R4            // R3 <-- R5 >> R4
    MOV         R1,R0               // R1 <-- R0
    LDR         R2,=0               // R2 <-- 0

L1: CBZ         R3,L2               // if R3 == 0, end while
    ADD         R4,R2,R3            // R4 <-- R2 + R3 
    CMP         R1,R4               // R1 >= R4
    ITT         HS                  // R1 >= R4
    SUBHS       R1,R1,R4            // R1 <-- R1 - R4
    ADDHS       R2,R2,R3,LSL 1      // R2 <-- R2 + (R3 << 1)
    LSR         R2,R2,1             // R2 <-- R2 >> 1
    LSR         R3,R3,2             // R3 <-- R3 >> 2
    B           L1                  // loop back

L2: LSL         R0,R2,8             // R2 <-- R2 << 8
    POP         {PC,R4,R5}          // restore R4, R5; return



.end