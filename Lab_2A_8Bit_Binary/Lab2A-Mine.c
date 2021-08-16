#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


//converts signed 8 bit value to a signed decimal and returns it
int32_t  Bits2Signed(int8_t bits[8])  {
    int32_t result = 0;
    int32_t result2 = 0;

    if(bits[7] == 1) {      //if the first bit is a 1, then the decimal value will be negative (begins then at -128 and adds the subsequent bits' decimal value equivalents), else non-negative and begin the following loop at 0
        result = pow(2,7) * bits[7] * -1;
    } else {
        result = 0;
    }

    for(int i = 6; i >= 0; i--) {       //increments through each bit to sum up the decimal result
        result2 = 2 * result2 + bits[i];
    }

    return result + result2;
}

//converts unsigned 8 bit value to an unsigned decimal and returns it
uint32_t Bits2Unsigned(int8_t bits[8])  {
    uint32_t result = 0;

    for(int i = 7; i >= 0; i--) {       //increments through each bit to sum up the decimal result
        result = 2 * result + bits[i];
    }

    return result;
}

//increments 8 bit value by 1
void     Increment(int8_t bits[8])  {
    for(int i = 0; i < 8; i++){     //increments through each digit. adds a 1 when it reaches a zero, else it overflows)
        if(bits[i] == 0) {
            bits[i] = 1;
            return;
        } else {
            bits[i] = 0;
        }
    }
}

//converts an unsigned integer to its equivalent 8 bit value
void     Unsigned2Bits(uint32_t n, int8_t bits[8])  {
    uint32_t stop = 128;    //max value of the eighth bit
    for(int i = 7; i >= 0; i--){    //increments through the array of 8 bits backwards in correspondence to the integer value and subtracts from the integer value as each bit is set to 1 (or skips over in case of a 0)
        if(n >= stop) {     //if the integer is >= the max potential value of the bit, then set the bit to 1 and subtract that bit's corresponding decimal value from the integer n, else the bit is set to 0
            bits[i] = 1;
            n -= stop;
        } else {
            bits[i] = 0;
        }
        stop /= 2;      //sets the max value of the next bit in the 8bit sequence (128, 64, 32, ..., 1)
    }
}