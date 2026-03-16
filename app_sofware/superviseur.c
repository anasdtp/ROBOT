/*
 * Superviseur minimal: configure KP via PIO puis boucle.
 */

#include <stdio.h>

#define KP_BASE 0x000010B0

#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)

#define KP 500

int main(void) {
    printf("=== Superviseur ===\n");
    printf("KP = %d\n", KP);

    /* Configure KP from software (12-bit signed) */
    IOWR(KP_BASE, (unsigned int)(KP & 0x0FFF));

    while (1) {
        /* Idle loop */
    }

    return 0;
}
