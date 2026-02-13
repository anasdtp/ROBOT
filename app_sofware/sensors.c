/*
 * Sensors reading program
 * Reads and displays the 7 ground sensors values
 */

#include <stdio.h>

// Sensor data addresses (from QSys memory map)
#define SENSOR_DATA0 0x00001040
#define SENSOR_DATA1 0x00001050
#define SENSOR_DATA2 0x00001060
#define SENSOR_DATA3 0x00001070
#define SENSOR_DATA4 0x00001080
#define SENSOR_DATA5 0x00001090
#define SENSOR_DATA6 0x000010a0

// Read macro for memory access
#define IORD(base) (*((volatile unsigned int*)(base)))

int main()
{
    unsigned int sensor0;
    unsigned int sensor1;
    unsigned int sensor2;
    unsigned int sensor3;
    unsigned int sensor4;
    unsigned int sensor5;
    unsigned int sensor6;
    
    int i = 0;
    printf("=== Ground Sensors Reader ===\n");
    printf("Press Ctrl+C to exit\n\n");
    
    while(1)
    {
        // Read all sensor values directly with mask
        sensor0 = (*(volatile unsigned int*)SENSOR_DATA0);
        sensor1 = (*(volatile unsigned int*)SENSOR_DATA1);
        sensor2 = (*(volatile unsigned int*)SENSOR_DATA2);
        sensor3 = (*(volatile unsigned int*)SENSOR_DATA3);
        sensor4 = (*(volatile unsigned int*)SENSOR_DATA4);
        sensor5 = (*(volatile unsigned int*)SENSOR_DATA5);
        sensor6 = (*(volatile unsigned int*)SENSOR_DATA6);
        
        // Display sensor values
        printf("\rS0:%3d  S1:%3d  S2:%3d  S3:%3d  S4:%3d  S5:%3d  S6:%3d   ",
               sensor0, 
               sensor1, 
               sensor2, 
               sensor3, 
               sensor4, 
               sensor5, 
               sensor6);
        
        fflush(stdout);
        
        // Small delay (100ms)
        for(i = 0; i < 500000; i++);
    }
    
    return 0;
}
