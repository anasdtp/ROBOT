/*
 * Suivi de Ligne Robot piloté par Capteurs de Sol
 * Adaptation du TP_ZUMO32U4 pour plateforme ROBOT NIOS II
 * 
 * Fonctionnement:
 * - Lit 5 capteurs de sol (utilise les capteurs DATA0-4)
 * - Utilise un PID simple pour le suivi de ligne
 * - Contrôle les moteurs droit et gauche
 * - Démarre si au moins 1 switch est à 1
 * - LEDs affichent l'état
 */

#include <stdio.h>

/* ===== ADRESSES PERIPHERIQUES ===== */
#define SWITCHES_BASE     0x00002000
#define LEDS_BASE         0x00002010
#define MOTOR_RIGHT_BASE  0x00001010
#define MOTOR_LEFT_BASE   0x00001000

/* Capteurs de sol */
#define SENSOR_DATA0      0x00001040
#define SENSOR_DATA1      0x00001050
#define SENSOR_DATA2      0x00001060
#define SENSOR_DATA3      0x00001070
#define SENSOR_DATA4      0x00001080
#define SENSOR_DATA5      0x00001090
#define SENSOR_DATA6      0x000010a0
#define SENSOR_DATA_READY 0x00001030

/* ===== MACROS ACCES MEMOIRE ===== */
#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)
#define IORD(base)       (*((volatile unsigned int*)(base)))

/* ===== PARAMETRES MOTEURS ===== */
#define GO_BIT            0x2000    /* Bit 13 */
#define DIR_BIT           0x1000    /* Bit 12 */
#define SPEED_MASK        0x0FFF    /* Bits 11-0 */

#define MIN_SPEED         0x600    /* 1792 - Vitesse minimale */
#define MAX_SPEED         0x9F0    /* 4080 - Vitesse maximale */

#define BASE_SPEED        1856    /* 2304 - Vitesse de base */

/* ===== PARAMETRES PID ===== */
#define KP                900     /* Gain proportionnel */
#define KI                0.0      /* Gain intégral */
#define KD                0.0       /* Gain dérivé */
#define MAX_INTEGRAL      500.0     /* Anti-windup */

/* ===== CONSTANTES CAPTEURS ===== */
#define NUM_SENSORS       7

/* ===== VARIABLES GLOBALES ===== */
unsigned int sensor_values[NUM_SENSORS];
unsigned int sensor_thresholds[NUM_SENSORS] = {110, 110, 110, 110, 110, 110, 110};  /* Seuil par capteur - calibré */
int robot_running = 0;            /* Robot actif si 1 */

/* Variables PID */
float integral = 0.0;
float prev_error = 0.0;
int last_position = 0;  /* Dernière position connue pour affichage LED */

/* Délai simple */
void delay_ms(int ms) {
    volatile int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 1000; j++);
}


int map(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* Lire tous les capteurs de sol */
void read_sensors(void) {
    sensor_values[0] = (IORD(SENSOR_DATA0)) & 0xFFFF;
    sensor_values[1] = (IORD(SENSOR_DATA1)) & 0xFFFF;
    sensor_values[2] = (IORD(SENSOR_DATA2)) & 0xFFFF;
    sensor_values[3] = (IORD(SENSOR_DATA3)) & 0xFFFF;
    sensor_values[4] = (IORD(SENSOR_DATA4)) & 0xFFFF;
    sensor_values[5] = (IORD(SENSOR_DATA5)) & 0xFFFF;
    sensor_values[6] = (IORD(SENSOR_DATA6)) & 0xFFFF;
}

/* 
 * Calculer la position de la ligne
 * Retourne: -3 (extrême gauche) à +3 (extrême droite)
 *           0 = ligne au centre
 */
int line_position(void) {
    int weighted_sum = 0;
    int sum = 0;
    int i;
    int black_count = 0;
    
    for(i = 0; i < NUM_SENSORS; i++) {
        if(sensor_values[i] > sensor_thresholds[i]) {
            /* Capteur detecte du noir */
            black_count++;
            /* Pondération: -3 (gauche) à +3 (droite) */
            int weight = (i - 3);  /* -3, -2, -1, 0, 1, 2, 3 */
            weighted_sum += weight;
            sum += 1;
        }
    }
    
    if(sum == 0) {
        /* Pas de ligne detectée */
        return prev_error > 0 ? 3 : -3;
    }
    
    return weighted_sum / sum;
}

/* 
 * Calculer la commande PID
 * error: écart par rapport à la position idéale (0)
 * Retourne: différence de vitesse (-500 à +500)
 */
int calculate_pid(float error) {
    float derivative;
    float output;
    
    /* Intégral avec anti-windup */
    integral += error;
    if(integral > MAX_INTEGRAL) integral = MAX_INTEGRAL;
    if(integral < -MAX_INTEGRAL) integral = -MAX_INTEGRAL;
    
    /* Dérivé */
    derivative = error - prev_error;
    prev_error = error;
    
    /* Calcul PID */
    output = KP * error + KI * integral + KD * derivative;
    
    /* Saturation */
    // if(output > 700) output = 700;
    // if(output < -700) output = -700;
    
    return (int)output;
}

/* Arrêter les moteurs */
void stop_motors(void) {
    IOWR(MOTOR_RIGHT_BASE, 0);
    IOWR(MOTOR_LEFT_BASE, 0);
}

/* 
 * Contrôler les moteurs
 * speed_right, speed_left: vitesse (valeur positive ou négative)
 * Si négatif: rotation inversée (DIR_BIT activé)
 */
void set_motors(int speed_right, int speed_left) {
    int right_cmd = GO_BIT;
    int left_cmd = GO_BIT;
    
    /* Moteur droit */
    if(speed_right < 0) {
        right_cmd |= DIR_BIT;
        speed_right = -speed_right;  /* Inversion pour correspondre à la logique du moteur */
    }
    
    /* Moteur gauche */
    if(speed_left < 0) {
        left_cmd |= DIR_BIT;
        speed_left = -speed_left;  /* Inversion pour correspondre à la logique du moteur */
    }

    /* Saturation */
    if(speed_right > MAX_SPEED) speed_right = MAX_SPEED;
    if(speed_right < MIN_SPEED) speed_right = MIN_SPEED;
    if(speed_left > MAX_SPEED) speed_left = MAX_SPEED;
    if(speed_left < MIN_SPEED) speed_left = MIN_SPEED;

    right_cmd |= (-speed_right & SPEED_MASK);
    left_cmd |= (-speed_left & SPEED_MASK);

    IOWR(MOTOR_RIGHT_BASE, right_cmd);
    IOWR(MOTOR_LEFT_BASE, left_cmd);
}

/* Suivi de ligne avec PID */
void line_follow(void) {
    int position;
    int speed_correction;
    int right_speed, left_speed;
    
    /* Lire capteurs */
    read_sensors();
    
    /* Calculer position ligne (pour affichage LED et PID) */
    position = line_position();
    last_position = position;  /* Mémoriser pour affichage LED */
    
    /* PID basé sur la position de la ligne */
    speed_correction = calculate_pid((float)position);
    
    /* Appliquer correction aux moteurs */
    /* Si capteur_2 > capteur_4 : ligne vire à gauche, ralentir droit, accélérer gauche */
    right_speed = BASE_SPEED + speed_correction;
    left_speed = BASE_SPEED - speed_correction;
    
    /* Envoyer aux moteurs */
    set_motors(right_speed, left_speed);
}

/* Vérifier état des switches */
int check_switches(void) {
    unsigned int sw = IORD(SWITCHES_BASE);
    return (sw != 0) ? 1 : 0;
}

/* Affiche l'état sur les LEDs */
void set_leds_status(unsigned char status) {
    IOWR(LEDS_BASE, status);
}

/* Affiche la position de la ligne sur les LEDs */
unsigned char position_to_leds(int position) {
    unsigned char led = 0;
    
    /* Mapper la position (-2000 à +2000) sur les LEDs (8 bits) - Une LED à la fois */
    if(position <= -1500) {
        led = 0x01;  /* LED0 - Extrême gauche */
    } else if(position <= -1000) {
        led = 0x02;  /* LED1 - Gauche */
    } else if(position <= -500) {
        led = 0x04;  /* LED2 - Légèrement gauche */
    } else if(position <= 0) {
        led = 0x08;  /* LED3 - Centre-gauche */
    } else if(position < 500) {
        led = 0x10;  /* LED4 - Centre-droite */
    } else if(position < 1000) {
        led = 0x20;  /* LED5 - Légèrement droite */
    } else if(position < 1500) {
        led = 0x40;  /* LED6 - Droite */
    } else {
        led = 0x80;  /* LED7 - Extrême droite */
    }
    
    return led;
}

int main(void) {
    int switch_state = 0;
    unsigned char led_state = 0;
    
    printf("=== Line Follower Robot ===\n");
    printf("Appuyez sur un switch pour demarrer\n");
    
    /* LED clignotement initial */
    IOWR(LEDS_BASE, 0xFF);
    delay_ms(500);
    IOWR(LEDS_BASE, 0x00);
    delay_ms(500);
    
    while(1) {
        /* Lire l'état des switchs */
        switch_state = check_switches();
        
        if(switch_state) {
            /* Robot actif */
            robot_running = 1;
            
            if(IORD(SENSOR_DATA_READY) == 1) {
                /* Suivi de ligne */
                line_follow();
            }
            else{
                /* Afficher la position sur les LEDs */
                led_state = position_to_leds(last_position);
            }
            
        } else {
            /* Robot arrêté */
            if(robot_running) {
                stop_motors();
                robot_running = 0;
                integral = 0.0;
                prev_error = 0.0;
            }
            led_state = 0x00;  /* LEDs éteintes */
        }
        
        set_leds_status(led_state);
        delay_ms(5);  /* Délai court - la lecture capteurs attend le flag */
    }
    
    return 0;
}
