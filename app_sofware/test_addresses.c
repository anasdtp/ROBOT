/* Test each address one by one to find which crashes */
#include <stdio.h>

#define LEDS (volatile unsigned int *) 0x00002010

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    printf("Test existence PIOs\n");
    
    printf("1. Test LEDS...\n");
    *LEDS = 0xAA;
    printf("   LEDS OK!\n");
    delay(1000000);
    
    printf("2. Test SENSOR_CONTROL (0x1020)...\n");
    volatile unsigned int *ctrl = (volatile unsigned int *) 0x00001020;
    *ctrl = 0x00;
    printf("   SENSOR_CONTROL OK!\n");
    delay(1000000);
    
    printf("3. Test SENSOR_STATUS (0x1030)...\n");
    volatile unsigned int *status = (volatile unsigned int *) 0x00001030;
    unsigned int val = *status;
    printf("   SENSOR_STATUS OK! val=%d\n", val);
    delay(1000000);
    
    printf("4. Test SENSOR_DATA0 (0x1040)...\n");
    volatile unsigned int *data0 = (volatile unsigned int *) 0x00001040;
    val = *data0;
    printf("   SENSOR_DATA0 OK! val=%d\n", val);
    delay(1000000);
    
    printf("Tous les tests passes!\n");
    printf("Les PIOs existent et sont accessibles.\n");
    
    while(1) {
        *LEDS = 0x55;
        delay(500000);
        *LEDS = 0xAA;
        delay(500000);
    }
}
