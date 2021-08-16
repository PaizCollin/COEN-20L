    .syntax     unified
    .cpu        cortex-m4
    .text



//Log2Phys(uint32_t lba, uint32_t heads, uint32_t sectors, CHS *phy)
    .global     Log2Phys
    .thumb_func
    .align
Log2Phys:
    PUSH        {R4,R5}             // preserve R4,R5
    MOV         R5,R1               // R5 <-- R1 (heads)
    MOV         R4,R0               // R4 <-- R0 (lba)

    //phy->cylinder = lba / (heads * sectors);
    MUL         R0,R5,R2            // R0 <-- R5 * R2
    UDIV        R0,R4,R0            // R0 <-- R4 / R0
    STRH        R0,[R3]             // R0 --> R3

    //phy->head = (lba / sectors) % heads;
    UDIV        R0,R4,R2            // R0 <-- R4 / R2
    UDIV        R1,R0,R5            // R1 <-- R0 / R5
    MLS         R0,R5,R1,R0         // R0 <-- R0 - (R5 * R1)
    STRB        R0,[R3,2]           // R1 --> R3 shifted

    //phy->sector = (lba % sectors) + 1;
    UDIV        R0,R4,R2            // R0 <-- R4 / R2
    MLS         R0,R2,R0,R4         // R0 <-- R4 - (R2 * R0)
    ADD         R0,R0,1             // R0 <-- R0 + 1
    STRB        R0,[R3,3]           // R0 --> R3 shifted

    POP         {R4,R5}             // revert R4,R5
    BX          LR                  // return

    .end