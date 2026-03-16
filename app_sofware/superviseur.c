/*
 * Superviseur minimal: configure KP via PIO puis boucle.
 */

#include <stdio.h>

#define KP_BASE          0x000010B0
#define BASE_SPEED_BASE  0x000010C0
#define START_SL_BASE    0x00001110
#define FIN_SL_BASE      0x00001100
#define START_ROT_BASE   0x000010F0
#define DIR_ROT_BASE     0x000010E0
#define FIN_ROT_BASE     0x000010D0
#define KD_BASE          0x00001120

/* Capteurs de sol */
#define SENSOR_DATA0      0x00001040
#define SENSOR_DATA1      0x00001050
#define SENSOR_DATA2      0x00001060
#define SENSOR_DATA3      0x00001070
#define SENSOR_DATA4      0x00001080
#define SENSOR_DATA5      0x00001090
#define SENSOR_DATA6      0x000010A0
#define SENSOR_DATA_READY 0x00001030

#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)
#define IORD(base)       (*((volatile unsigned int*)(base)))

#define KP 100
#define KD 20
#define BASE_SPEED_RUN 2100
#define BASE_SPEED_ROT 1990
#define SENSOR_THRESHOLD 110

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

static int update_last_black_index(int last_index) {
    if ((IORD(SENSOR_DATA_READY) & 0x1) == 0) {
        return last_index;
    }

    unsigned int vals[7];
    int best_index = -1;
    unsigned int best_value = 0;
    int i = 0;

    vals[0] = IORD(SENSOR_DATA0) & 0xFF;
    vals[1] = IORD(SENSOR_DATA1) & 0xFF;
    vals[2] = IORD(SENSOR_DATA2) & 0xFF;
    vals[3] = IORD(SENSOR_DATA3) & 0xFF;
    vals[4] = IORD(SENSOR_DATA4) & 0xFF;
    vals[5] = IORD(SENSOR_DATA5) & 0xFF;
    vals[6] = IORD(SENSOR_DATA6) & 0xFF;

    for (i = 0; i < 7; i++) {
        if (vals[i] > SENSOR_THRESHOLD && vals[i] >= best_value) {
            best_value = vals[i];
            best_index = i;
        }
    }

    if (best_index >= 0) {
        return best_index;
    }

    return last_index;
}

int main(void) {
    state_t state = STATE_INIT_WAIT;
    int dir = 0;
    int last_black_index = 3;

    printf("=== Superviseur ===\n");
    printf("KP = %d\n", KP);
    printf("KD = %d\n", KD);

    /* Configure KP from software (12-bit signed) */
    IOWR(KP_BASE, (unsigned int)(KP & 0x0FFF));
    IOWR(KD_BASE, (unsigned int)(KD & 0x0FFF));
    IOWR(BASE_SPEED_BASE, 0);

    IOWR(START_SL_BASE, 1);
    IOWR(START_ROT_BASE, 0);
    IOWR(DIR_ROT_BASE, dir);

    while (1) {
        switch (state) {
            case STATE_INIT_WAIT:
                /* Wait for line detection: fin_sl goes low when line found */
                last_black_index = update_last_black_index(last_black_index);
                if ((IORD(FIN_SL_BASE) & 0x1) == 0) {
                    printf("Line detected -> LINE_FOLLOW\n");
                    IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_RUN & 0x0FFF));
                    state = STATE_LINE_FOLLOW;
                }
                break;

            case STATE_LINE_FOLLOW:
                last_black_index = update_last_black_index(last_black_index);
                if (IORD(FIN_SL_BASE) & 0x1) {
                    printf("Line lost -> pause then ROTATE\n");
                    IOWR(BASE_SPEED_BASE, 0);
                    // delay_ms(1000);
                    IOWR(START_SL_BASE, 0);
                    if (last_black_index < 3) {
                        dir = 1; /* line on right -> rotate right */
                    } else if (last_black_index > 3) {
                        dir = 0; /* line on left -> rotate left */
                    }
                    IOWR(DIR_ROT_BASE, dir);
                    printf("Last black index=%d, rotation dir=%d\n", last_black_index, dir);
                    IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_ROT & 0x0FFF));
                    IOWR(START_ROT_BASE, 1);
                    state = STATE_ROTATE;
                }
                break;

            case STATE_ROTATE:
            default:
                if (IORD(FIN_ROT_BASE) & 0x1) {
                    printf("Rotation done -> pause then LINE_FOLLOW\n");
                    IOWR(START_ROT_BASE, 0);
                    IOWR(BASE_SPEED_BASE, 0);
                    // delay_ms(1000);
                    IOWR(START_SL_BASE, 1);
                    IOWR(BASE_SPEED_BASE, (unsigned int)(BASE_SPEED_RUN & 0x0FFF));
                    state = STATE_LINE_FOLLOW;
                }
                break;
        }
    }

    return 0;
}
