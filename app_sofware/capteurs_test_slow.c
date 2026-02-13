/* Test triggering the sensors - slow with printf */
#include <stdio.h>

#define SENSOR_CONTROL (volatile unsigned int *) 0x00001020
#define SENSOR_STATUS  (volatile unsigned int *) 0x00001030
#define SENSOR_DATA0   (volatile unsigned int *) 0x00001040
#define SENSOR_DATA1   (volatile unsigned int *) 0x00001050
#define SENSOR_DATA2   (volatile unsigned int *) 0x00001060
#define SENSOR_DATA3   (volatile unsigned int *) 0x00001070
#define SENSOR_DATA4   (volatile unsigned int *) 0x00001080
#define SENSOR_DATA5   (volatile unsigned int *) 0x00001090
#define SENSOR_DATA6   (volatile unsigned int *) 0x000010A0
#define LEDS           (volatile unsigned int *) 0x00002010

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    unsigned int sensor_values[7];
    int cycle = 0;
    int i = 0;
    printf("=== Test capteurs avec trigger ===\n\n");
    
    while (1) {
        printf("[Cycle %d]\n", cycle++);
        // Trigger
        *SENSOR_CONTROL = 0x01;
        
        // Wait for conversion (at 50MHz, ADC needs time)
        delay(1000000); 
        
        // Check status
        unsigned int status = *SENSOR_STATUS;
        printf("  Status = 0x%02X %s\n", status, (status & 0x01) ? "READY!" : "NOT READY");
        
        if (status & 0x01) {
            // Read all sensors
            sensor_values[0] = *SENSOR_DATA0 & 0xFF;
            sensor_values[1] = *SENSOR_DATA1 & 0xFF;
            sensor_values[2] = *SENSOR_DATA2 & 0xFF;
            sensor_values[3] = *SENSOR_DATA3 & 0xFF;
            sensor_values[4] = *SENSOR_DATA4 & 0xFF;
            sensor_values[5] = *SENSOR_DATA5 & 0xFF;
            sensor_values[6] = *SENSOR_DATA6 & 0xFF;
            
            printf("  Capteurs: ");
            i = 0;
            for (i = 0; i < 7; i++) {
                printf("[%d]=%3d ", i, sensor_values[i]);
            }
            printf("\n");
            
            *LEDS = sensor_values[0];
        } else {
            printf("  ERREUR: Donnees non pretes!\n");
        }
        
        // Clear trigger
        *SENSOR_CONTROL = 0x00;
        printf("  Trigger clear\n\n");
        
        delay(1000000); 
    }
}
