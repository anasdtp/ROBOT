/*
 * Programme Triangle
 * Le robot parcourt un triangle équilatéral
 */

// Adresses des périphériques
#define MOTOR_RIGHT_BASE 0x00001010
#define MOTOR_LEFT_BASE  0x00001000
#define LEDS_BASE        0x00002010

// Accès mémoire direct
#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)

// Bits de contrôle moteur
#define GO_BIT      0x2000  // Bit 13
#define DIR_BIT     0x1000  // Bit 12

// Vitesses (caractérisation)
#define SPEED_MIN   0x700   // 1792
#define SPEED_MAX   0xFF0   // 4080
#define SPEED_FWD   0x900   // 2304 - Vitesse pour avancer
#define SPEED_TURN  0x800   // 2048 - Vitesse pour tourner

// Commandes moteurs
#define FORWARD_R   (GO_BIT | SPEED_FWD)
#define FORWARD_L   (GO_BIT | SPEED_FWD)
#define BACKWARD_R  (GO_BIT | DIR_BIT | SPEED_TURN)
#define BACKWARD_L  (GO_BIT | DIR_BIT | SPEED_TURN)
#define TURN_R      (GO_BIT | SPEED_TURN)
#define TURN_L      (GO_BIT | SPEED_TURN)
#define STOP        0x0000

// Délais (à calibrer selon votre robot)
#define DELAY_FORWARD  3000000   // Durée avancer (environ 3 secondes)
#define DELAY_TURN     1000000   // Durée rotation 120° (environ 1 seconde)
#define DELAY_PAUSE    500000    // Pause entre mouvements

// Fonction delay
void delay(volatile int count) {
    while(count--);
}

// Avancer en ligne droite
void move_forward() {
    IOWR(MOTOR_RIGHT_BASE, FORWARD_R);
    IOWR(MOTOR_LEFT_BASE, FORWARD_L);
    IOWR(LEDS_BASE, 0x01);  // LED0 = avancer
    delay(DELAY_FORWARD);
}

// Tourner à gauche (120 degrés pour triangle)
void turn_left_120() {
    // Moteur droit avance, moteur gauche recule
    IOWR(MOTOR_RIGHT_BASE, TURN_R);
    IOWR(MOTOR_LEFT_BASE, BACKWARD_L);
    IOWR(LEDS_BASE, 0x08);  // LED3 = tourner gauche
    delay(DELAY_TURN);
}

// Arrêter les moteurs
void stop_motors() {
    IOWR(MOTOR_RIGHT_BASE, STOP);
    IOWR(MOTOR_LEFT_BASE, STOP);
    IOWR(LEDS_BASE, 0x00);
}

// Pause
void pause() {
    stop_motors();
    delay(DELAY_PAUSE);
}

int main() {
    // Clignotement pour indiquer démarrage
    IOWR(LEDS_BASE, 0xFF);
    delay(1000000);
    IOWR(LEDS_BASE, 0x00);
    delay(1000000);
    
    // Boucle infinie : répéter le triangle
    while(1) {
        // Côté 1
        move_forward();
        pause();
        
        // Rotation 1 (120°)
        turn_left_120();
        pause();
        
        // Côté 2
        move_forward();
        pause();
        
        // Rotation 2 (120°)
        turn_left_120();
        pause();
        
        // Côté 3
        move_forward();
        pause();
        
        // Rotation 3 (120°) - retour position initiale
        turn_left_120();
        pause();
        
        // Pause longue avant de recommencer
        IOWR(LEDS_BASE, 0xFF);  // Toutes LEDs allumées = fin du triangle
        delay(2000000);
        IOWR(LEDS_BASE, 0x00);
        delay(1000000);
    }
    
    return 0;
}
