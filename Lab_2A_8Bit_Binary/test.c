#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

void     Unsigned2Bits(uint32_t n, int8_t bits[8])  {
    uint32_t stop = 128;
    for(int i = 7; i >= 0; i--){
        if(n >= stop) {
            bits[i] = 1;
            n -= stop;
        } else {
            bits[i] = 0;
        }
        stop /= 2;
    }
}

uint32_t Bits2Unsigned(int8_t bits[8])  {
    uint32_t result = 0;

    for(int i = 7; i >= 0; i--) {
        result = 2 * result + bits[i];
    }

    return result;
}

int32_t  Bits2Signed(int8_t bits[8])  {
    int32_t result = 0;
    int32_t result2 = 0;

    if(bits[7] == 1) {
        result = pow(2,7) * bits[7] * -1;
    } else {
        result = 0;
    }

    for(int i = 6; i >= 0; i--) {
        result2 = 2 * result2 + bits[i];
    }

    return result + result2;
}

int main() {
    /*int8_t byte[8];
    Unsigned2Bits(4, byte);
    for(int i = 7; i >= 0; i--){
        printf("%d", byte[i]);
    }
    */

    int8_t byte[8] = {1,1,0,0,0,0,0,1};
    uint32_t a = Bits2Signed(byte);
    printf("%d", a);
    return;

}