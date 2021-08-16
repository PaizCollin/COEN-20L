    .syntax     unified
    .cpu        cortex-m4
    .text



// int32_t Add(int32_t a, int32_t b)
// adds a and b together and returns it
    .global     Add
    .thumb_func
    .align
Add:        
    ADD     R0,R0,R1        // add R1+R0 and put it into R0
    BX      LR              




// int32_t Less1(int32_t a)
// subtracts 1 from a and returns it
    .global     Less1
    .thumb_func
    .align
Less1:      
    SUB     R0,R0,#1        // subtract R0-1 and put it into R0
    BX      LR              



// int32_t Square2x(int32_t x)
// returns the square of 2x
    .global     Square2x
    .thumb_func
    .align
Square2x:   
    PUSH    {LR}            
    ADD     R0,R0,R0        // add R0+R0 and put it into R0
    BL      Square          // call and link Square function
    POP     {PC}            



// int32_t Last(int32_t x)
//returns the sum of x + the square root of x
    .global     Last
    .thumb_func
    .align
Last:       
    PUSH    {R4,LR}         // preserve R4,LR
    MOV     R4,R0           // moves (copies) x from R0 to R4
    BL      SquareRoot      // call and link SquareRoot function
    ADD     R0,R0,R4        // add R4+R0 and put it into R0
    POP     {R4,PC}         // restore R4,PC              



    .end
