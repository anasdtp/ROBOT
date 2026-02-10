/*
 * Programme de caractérisation des moteurs
 * 
 * Ce programme permet de déterminer:
 * 1. Vitesse minimale pour rotation des roues hors sol
 * 2. Vitesse minimale pour déplacement au sol (avec/sans piles)
 * 3. Vitesse minimale en mouvement sans arrêt (hystérésis)
 */

#include <stdio.h>

// Adresses des périphériques (à vérifier dans votre .sopcinfo après génération Qsys)
#define SWITCHES_BASE    0x00002000
#define LEDS_BASE        0x00002010
#define MOTOR_RIGHT_BASE 0x00001010
#define MOTOR_LEFT_BASE  0x00001000

// Macros pour accès aux périphériques
#define IOWR(base, offset, data) (*((volatile unsigned int*)(base + (offset << 2))) = data)
#define IORD(base, offset)       (*((volatile unsigned int*)(base + (offset << 2))))

// Définition des bits de contrôle
#define MOTOR_GO_BIT      (1 << 13)  // Bit 13: GO/STOP
#define MOTOR_DIR_BIT     (1 << 12)  // Bit 12: FORWARD(0)/BACKWARD(1)
#define MOTOR_SPEED_MASK  0x0FFF     // Bits 11-0: Vitesse

// Macros de contrôle moteur
#define MOTOR_FORWARD(speed)  (MOTOR_GO_BIT | ((speed) & MOTOR_SPEED_MASK))
#define MOTOR_BACKWARD(speed) (MOTOR_GO_BIT | MOTOR_DIR_BIT | ((speed) & MOTOR_SPEED_MASK))
#define MOTOR_STOP            0x0000

// Valeur maximale de vitesse (12 bits)
#define MAX_SPEED 0x3F00

// Fonction pour définir la vitesse des moteurs
void set_motor_speed(int right_cmd, int left_cmd) {
    IOWR(MOTOR_RIGHT_BASE, 0, right_cmd);
    IOWR(MOTOR_LEFT_BASE, 0, left_cmd);
}

// Fonction pour arrêter les moteurs
void stop_motors() {
    set_motor_speed(MOTOR_STOP, MOTOR_STOP);
}

// Test 1: Vitesse minimale pour rotation hors sol
void test_min_speed_off_ground() {
    printf("\n=== TEST 1: Vitesse minimale hors sol ===\n");
    printf("Placer le robot hors sol\n");
    printf("Appuyez sur un switch pour commencer...\n");
    
    while(IORD(SWITCHES_BASE, 0) == 0);
    
    int speed = 0;
    int increment = 50;
    volatile int delay;
    
    printf("\nRecherche vitesse minimale...\n");
    
    for(speed = 0; speed <= MAX_SPEED; speed += increment) {
        printf("Vitesse testee: %d / %d\n", speed, MAX_SPEED);
        set_motor_speed(MOTOR_FORWARD(speed), MOTOR_FORWARD(speed));
        for(delay=0; delay<500000; delay++); // 0.5 seconde
        
        // Demander à l'utilisateur si les roues tournent
        printf("Les roues tournent-elles? (SW0=oui, SW1=non)\n");
        while(IORD(SWITCHES_BASE, 0) == 0);
        
        if(IORD(SWITCHES_BASE, 0) & 0x01) {
            printf("Vitesse minimale hors sol: %d\n", speed);
            break;
        }
        
        // Attendre relâchement du switch
        while(IORD(SWITCHES_BASE, 0) != 0);
    }
    
    stop_motors();
}

// Test 2: Vitesse minimale pour déplacement au sol
void test_min_speed_on_ground() {
    printf("\n=== TEST 2: Vitesse minimale au sol ===\n");
    printf("Placer le robot au sol, a l'arret\n");
    printf("Test SANS piles d'abord\n");
    printf("Appuyez sur un switch pour commencer...\n");
    
    while(IORD(SWITCHES_BASE, 0) == 0);
    while(IORD(SWITCHES_BASE, 0) != 0);
    
    int speed = 0;
    int increment = 50;
    volatile int delay;
    
    printf("\nRecherche vitesse minimale au sol...\n");
    
    for(speed = 0; speed <= MAX_SPEED; speed += increment) {
        printf("Vitesse testee: %d / %d\n", speed, MAX_SPEED);
        set_motor_speed(MOTOR_FORWARD(speed), MOTOR_FORWARD(speed));
        for(delay=0; delay<1000000; delay++); // 1 seconde
        
        printf("Le robot avance-t-il? (SW0=oui, SW1=non)\n");
        while(IORD(SWITCHES_BASE, 0) == 0);
        
        if(IORD(SWITCHES_BASE, 0) & 0x01) {
            printf("Vitesse minimale au sol (sans piles): %d\n", speed);
            break;
        }
        
        stop_motors();
        for(delay=0; delay<500000; delay++);
        while(IORD(SWITCHES_BASE, 0) != 0);
    }
    
    stop_motors();
}

// Test 3: Calibration pour ligne droite
void test_straight_line() {
    printf("\n=== TEST 3: Calibration ligne droite ===\n");
    printf("Ajustement des vitesses pour trajet rectiligne\n");
    
    int speed_r = 2000;
    int speed_l = 2000;
    int running = 1;
    volatile int delay;
    
    printf("Commandes:\n");
    printf("SW0: augmenter moteur droit\n");
    printf("SW1: diminuer moteur droit\n");
    printf("SW2: augmenter moteur gauche\n");
    printf("SW3: diminuer moteur gauche\n");
    printf("SW7: terminer\n\n");
    
    set_motor_speed(MOTOR_FORWARD(speed_r), MOTOR_FORWARD(speed_l));
    
    while(running) {
        int sw = IORD(SWITCHES_BASE, 0);
        
        if(sw & 0x01) {
            speed_r += 10;
            if(speed_r > MAX_SPEED) speed_r = MAX_SPEED;
            printf("Moteur droit: %d, Moteur gauche: %d\n", speed_r, speed_l);
            set_motor_speed(MOTOR_FORWARD(speed_r), MOTOR_FORWARD(speed_l));
            for(delay=0; delay<100000; delay++);
        }
        if(sw & 0x02) {
            speed_r -= 10;
            if(speed_r < 0) speed_r = 0;
            printf("Moteur droit: %d, Moteur gauche: %d\n", speed_r, speed_l);
            set_motor_speed(MOTOR_FORWARD(speed_r), MOTOR_FORWARD(speed_l));
            for(delay=0; delay<100000; delay++);
        }
        if(sw & 0x04) {
            speed_l += 10;
            if(speed_l > MAX_SPEED) speed_l = MAX_SPEED;
            printf("Moteur droit: %d, Moteur gauche: %d\n", speed_r, speed_l);
            set_motor_speed(MOTOR_FORWARD(speed_r), MOTOR_FORWARD(speed_l));
            for(delay=0; delay<100000; delay++);
        }
        if(sw & 0x08) {
            speed_l -= 10;
            if(speed_l < 0) speed_l = 0;
            printf("Moteur droit: %d, Moteur gauche: %d\n", speed_r, speed_l);
            set_motor_speed(MOTOR_FORWARD(speed_r), MOTOR_FORWARD(speed_l));
            for(delay=0; delay<100000; delay++);
        }
        if(sw & 0x80) {
            running = 0;
        }
        
        for(delay=0; delay<50000; delay++);
    }
    
    stop_motors();
    printf("\nVitesses finales - Droit: %d, Gauche: %d\n", speed_r, speed_l);
}

// Test 4: Hystérésis
void test_hysteresis() {
    printf("\n=== TEST 4: Test d'hysteresis ===\n");
    printf("Le robot va accelerer puis deceler progressivement\n");
    printf("Appuyez sur un switch pour commencer...\n");
    
    while(IORD(SWITCHES_BASE, 0) == 0);
    while(IORD(SWITCHES_BASE, 0) != 0);
    
    // Accélération
    printf("\nAcceleration...\n");
    int speed_start = 2000;
    int speed;
    volatile int delay;
    set_motor_speed(MOTOR_FORWARD(speed_start), MOTOR_FORWARD(speed_start));
    for(delay=0; delay<2000000; delay++); // 2 secondes pour stabiliser
    
    // Décélération progressive
    printf("Deceleration progressive...\n");
    for(speed = speed_start; speed >= 0; speed -= 20) {
        printf("Vitesse: %d\n", speed);
        set_motor_speed(MOTOR_FORWARD(speed), MOTOR_FORWARD(speed));
        for(delay=0; delay<200000; delay++); // 0.2 seconde
        
        // Laisser à l'utilisateur le temps de voir si une roue s'arrête
        int sw = IORD(SWITCHES_BASE, 0);
        if(sw & 0x01) {
            printf("Une roue s'est arretee a la vitesse: %d\n", speed);
            break;
        }
    }
    
    stop_motors();
}

int main() {
    printf("\n========================================\n");
    printf("  PROGRAMME DE CARACTERISATION MOTEURS\n");
    printf("========================================\n");
    
    int choice = 0;
    int running = 1;
    
    while(running) {
        printf("\n\nMenu principal:\n");
        printf("1. Test vitesse mini hors sol\n");
        printf("2. Test vitesse mini au sol\n");
        printf("3. Calibration ligne droite\n");
        printf("4. Test hysteresis\n");
        printf("5. Test libre (SW controle vitesse)\n");
        printf("6. Quitter\n");
        printf("\nUtilisez les switches pour choisir (1-6)...\n");
        
        while(IORD(SWITCHES_BASE, 0) == 0);
        choice = IORD(SWITCHES_BASE, 0) & 0x07;
        while(IORD(SWITCHES_BASE, 0) != 0);
        
        switch(choice) {
            case 1:
                test_min_speed_off_ground();
                break;
            case 2:
                test_min_speed_on_ground();
                break;
            case 3:
                test_straight_line();
                break;
            case 4:
                test_hysteresis();
                break;
            case 5:
                printf("Mode test libre - SW0-4 pour vitesse, SW7 pour arreter\n");
                {
                    volatile int delay;
                    while(!(IORD(SWITCHES_BASE, 0) & 0x80)) {
                        int sw = IORD(SWITCHES_BASE, 0) & 0x1F;
                        int speed_test = sw * 200;
                        set_motor_speed(MOTOR_FORWARD(speed_test), MOTOR_FORWARD(speed_test));
                        printf("Vitesse: %d\n", speed_test);
                        for(delay=0; delay<100000; delay++);
                    }
                    stop_motors();
                }
                break;
            case 6:
                running = 0;
                break;
            default:
                printf("Choix invalide\n");
        }
    }
    
    stop_motors();
    printf("\nProgramme termine.\n");
    return 0;
}
