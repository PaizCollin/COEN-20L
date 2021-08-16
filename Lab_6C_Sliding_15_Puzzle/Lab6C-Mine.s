    .syntax     unified
    .cpu        cortex-m4
    .text



// CopyCell(uint32_t *dst, uint32_t *src)
    .global     CopyCell
    .thumb_func
    .align
CopyCell:
    PUSH        {R4,R5,LR}              // preserve R4,R5,LR
    MOV         R4,R0                   // move R0 to R4
    MOV         R5,R1                   // move R1 to R5
    LDR         R2,=0                   // R2 <-- 0
    LDR         R3,=0                   // R3 <-- 0

L1: CMP         R2,60                   // is R2 < 60?
    BGE         L4                      // if not, end outer loop

L2: CMP         R3,60                   // is R3 < 60?
    BGE         L3                      // if not, end inner loop
    LDR         R1,[R5,R3,LSL 2]        // R1 <-- src[col]
    STR         R1,[R4,R3,LSL 2]        // R1 --> dst[col]
    ADD         R3,R3,1                 // R3 ++
    B           L2                      // repeat inner loop

L3: ADD         R4,R4,960               // R4 = R4 + (240 * 4)
    ADD         R5,R5,960               // R5 = R5 + (240 * 4)
    ADD         R2,R2,1                 // R2 ++
    LDR         R3,=0                   // R3 <-- 0
    B           L1                      // repeat outer loop

L4: POP         {R4,R5,PC}              // restore R4,R5,PC



// FillCell(uint32_t *dst, uint32_t pixel)
    .global     FillCell
    .thumb_func
    .align
FillCell:
    LDR         R2,=0                   // R2 <-- 0
    LDR         R3,=0                   // R3 <-- 0

L5: CMP         R2,60                   // is R2 < 60?
    BGE         L8                      // if not, end outer loop

L6: CMP         R3,60                   // is R3 < 60?
    BGE         L7                      // if not, end inner loop
    STR         R1,[R0,R3,LSL 2]        // R1 --> dst[col]
    ADD         R3,R3,1                 // R3 ++
    B           L6                      // repeat inner loop

L7: ADD         R0,R0,960               // R0 = R0 + (240 * 4)
    ADD         R2,R2,1                 // R2 ++
    LDR         R3,=0                   // R3 <-- 0
    B           L5                      // repeat outer loop
    
L8: BX          LR                      // return

    .end