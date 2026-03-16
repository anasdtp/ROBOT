/*
 * Superviseur minimal: configure KP via PIO puis boucle.
 */

#include <stdio.h>

#define KP_BASE          0x000010B0
#define BASE_SPEED_BASE  0x000010C0
#define FIN_ROT_BASE     0x000010D0
#define DIR_ROT_BASE     0x000010E0
#define START_ROT_BASE   0x000010F0
#define FIN_SL_BASE      0x00001100
#define START_SL_BASE    0x00001110

#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)
#define IORD(base)       (*((volatile unsigned int*)(base)))

#define KP 500
#define BASE_SPEED_RUN 1856
#define BASE_SPEED_ROT 1500

typedef enum {
    STATE_INIT_WAIT = 0,
    STATE_LINE_FOLLOW,
    STATE_ROTATE
} state_t;

static void delay_ms(int ms) {
    volatile int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1000; j++) {
        }
    }
}

int main(void) {
    state_t state = STATE_INIT_WAIT;
    int dir = 0;

    printf("=== Superviseur ===\n");
    printf("KP = %d\n", KP);

    /* Configure KP from software (12-bit signed) */
    IOWR(KP_BASE, (unsigned int)(KP & 0x0FFF));
    IOWR(BASE_SPEED_BASE, 0);

    IOWR(START_SL_BASE, 1);
    IOWR(START_ROT_BASE, 0);
    IOWR(DIR_ROT_BASE, dir);

    while (1) {
        if (state == STATE_INIT_WAIT) {
            /* Wait for line detection: fin_sl goes low when line found */
            if ((IORD(FIN_SL_BASE) & 0x1) == 0) {
                IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_RUN & 0x0FFF));
                state = STATE_LINE_FOLLOW;
            }
        } else if (state == STATE_LINE_FOLLOW) {
            if (IORD(FIN_SL_BASE) & 0x1) {
                IOWR(BASE_SPEED_BASE, 0);
                delay_ms(1000);
                IOWR(START_SL_BASE, 0);
                dir = (dir == 0) ? 1 : 0;
                IOWR(DIR_ROT_BASE, dir);
                IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_ROT & 0x0FFF));
                IOWR(START_ROT_BASE, 1);
                state = STATE_ROTATE;
            }
        } else {
            if (IORD(FIN_ROT_BASE) & 0x1) {
                IOWR(START_ROT_BASE, 0);
                IOWR(BASE_SPEED_BASE, 0);
                delay_ms(1000);
                IOWR(START_SL_BASE, 1);
                IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_RUN & 0x0FFF));
                state = STATE_LINE_FOLLOW;
            }
        }
    }

    return 0;
}
