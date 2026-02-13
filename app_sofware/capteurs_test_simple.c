/* Simple test: just toggle control and check status */
#include <stdio.h>

// Base addresses from QSys
#define SENSOR_CONTROL (volatile char *) 0x00001020
#define SENSOR_STATUS  (volatile char *) 0x00001030
#define SENSOR_DATA0   (volatile char *) 0x00001040
#define LEDS           (volatile char *) 0x00002010

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    int count = 0;
    
    printf("=== Test Simple Capteurs ===\n");
    
    while (1) {
        printf("\n[%d] Test cycle\n", count++);
        
        // Read initial status
        printf("  Status AVANT trigger: 0x%02X\n", *SENSOR_STATUS);
        
        // Trigger
        *SENSOR_CONTROL = 0x01;
        printf("  Control mis a 1\n");
        
        // Wait and check status multiple times
        for (int i = 0; i < 10; i++) {
            delay(200000);
            unsigned char st = *SENSOR_STATUS;
            printf("  Status[%d]: 0x%02X %s\n", i, st, (st & 0x01) ? "READY!" : "");
            if (st & 0x01) {
                printf("  Data0 = %d\n", *SENSOR_DATA0);
                *LEDS = *SENSOR_DATA0;
                break;
            }
        }
        
        // Clear trigger
        *SENSOR_CONTROL = 0x00;
        printf("  Control mis a 0\n");
        delay(500000);
        
        printf("  Status APRES clear: 0x%02X\n", *SENSOR_STATUS);
    }
}
