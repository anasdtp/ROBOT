/* Test just reading sensor PIOs without triggering */
#include <stdio.h>

#define SENSOR_CONTROL (volatile unsigned int *) 0x00001020
#define SENSOR_STATUS  (volatile unsigned int *) 0x00001030
#define SENSOR_DATA0   (volatile unsigned int *) 0x00001040
#define LEDS           (volatile unsigned int *) 0x00002010

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    int count = 0;
    
    printf("Test lecture PIOs capteurs (sans trigger)\n");
    
    // Keep control at 0, just read status
    *SENSOR_CONTROL = 0x00;
    
    while (1) {
        unsigned int status = *SENSOR_STATUS;
        unsigned int data = *SENSOR_DATA0;
        
        printf("%d: Status=0x%02X Data0=%d\n", count++, status & 0xFF, data & 0xFF);
        
        *LEDS = data & 0xFF;
        
        delay(1000000);
        
        if (count > 20) {
            printf("Test OK - pas de reboot!\n");
            break;
        }
    }
    
    printf("Fin du test\n");
    while(1);  // Stop here
}
