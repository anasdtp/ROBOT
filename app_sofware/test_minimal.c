/* Minimal test - no sensor access, just blink LEDs */
#include <stdio.h>

#define LEDS (volatile char *) 0x00002010

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    unsigned char led_val = 0;
    int count = 0;
    
    printf("Programme minimal - test LEDs\n");
    
    while (1) {
        *LEDS = led_val;
        led_val++;
        
        printf("Count: %d, LED: %d\n", count++, led_val);
        
        delay(1000000);
    }
}
