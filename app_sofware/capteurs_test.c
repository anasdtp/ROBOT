/* Test program for ground sensors (LTC2308 ADC)
 * This program continuously reads the 7 ground sensors
 * and displays sensor 0 value on the LEDs
 */
#include <stdio.h>
// Base addresses from QSys
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

// Delay function
void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

void main() {
    unsigned int sensor_values[7];
    int i;
    
    printf("=== Test des capteurs de sol (LTC2308) ===\n");
    printf("Activation du signal 2 KHz...\n\n");
    
    // Enable continuous 2 KHz capture signal
    *SENSOR_CONTROL = 0x01;
    
    // Wait for first conversion
    delay(2000000);
    
    while (1) {
        // Check if data is ready
        unsigned int status = *SENSOR_STATUS;
        
        if (status & 0x01) {  // Check bit 0 (data_ready)
            // Read all sensor values
            sensor_values[0] = *SENSOR_DATA0 & 0xFF;
            sensor_values[1] = *SENSOR_DATA1 & 0xFF;
            sensor_values[2] = *SENSOR_DATA2 & 0xFF;
            sensor_values[3] = *SENSOR_DATA3 & 0xFF;
            sensor_values[4] = *SENSOR_DATA4 & 0xFF;
            sensor_values[5] = *SENSOR_DATA5 & 0xFF;
            sensor_values[6] = *SENSOR_DATA6 & 0xFF;
            
            // Display all sensor values
            printf("Capteurs: ");
            for (i = 0; i < 7; i++) {
                printf("[%d]=%3d  ", i, sensor_values[i]);
            }
            printf("\n");
            
            // Display sensor 0 on LEDs
            *LEDS = sensor_values[0];
            
        } else {
            printf("Attente donnees...\n");
        }
        
        // Wait before next read (les conversions se font automatiquement à 2 KHz)
        delay(2000000);  // Affichage toutes les ~8 secondes
    }
}
