    .syntax     unified
    .cpu        cortex-m4
    .text



// GetNibble(void *nibbles, uint32_t which)
    .global     GetNibble
    .thumb_func
    .align
GetNibble:      // R0 <-- nibbles, R1 <-- which
    LSR         R12,R1,1                // R12 <-- R1 >> 1
    LDRB        R2,[R0,R12]             // R2 (byte) <-- nibbles[which >> 1]
    AND         R3,R1,1                 // R3 <-- R1 & 1
    CMP         R3,1                    // is R3 == 1?
    BNE         L1                      // if not, end if (L1)
    LSR         R2,R2,4                 // R2 <-- R2 >> 4
L1: AND         R0,R2,0b00001111        // R0 <-- R2 & 0b00001111
    BX          LR                      // return



// PutNibble(void *nibbles, uint32_t which, uint32_t value)
    .global     PutNibble
    .thumb_func
    .align
PutNibble:      // R0 <-- nibbles, R1 <-- which, R2 <-- value
    LSR         R12,R1,1                // R12 <-- R1 >> 1
    LDRB        R3,[R0,R12]             // R3 <-- R0 + R12
    AND         R1,R1,1                 // R1 <-- R1 & 1
    CMP         R1,1                    // is R1 == 1?
    BNE         L2                      // if not, go to else (L2)
    AND         R3,R3,0b00001111        // R3 <-- R3 & 0b00001111
    ORR         R3,R3,R2,LSL 4          // R3 <-- R3 | (R2 << 4)
    B           L3                      // endif (L3)
L2: AND         R3,R3,0b11110000        // R3 <-- R3 & 0b11110000
    ORR         R3,R3,R2                // R3 <-- R3 | R2 
L3: STRB        R3,[R0,R12]             // R3 --> R0 + R12
    BX          LR                      // return



    .end