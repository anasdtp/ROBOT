/*
 * Test moteur avec profil d'accélération trapézoïdale
 * Accélération -> Vitesse constante -> Décélération -> Arrêt
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

// Plage de vitesse fonctionnelle (déterminée expérimentalement)
#define SPEED_MIN   0x3700   // 1792 - Vitesse minimale
#define SPEED_MAX   0x3FF0   // 4080 - Vitesse maximale

// Paramètres du profil trapézoïdal
#define ACCEL_STEP   8       // Incrément de vitesse par étape
#define DELAY_CYCLES 50000   // Délai entre chaque étape

// Fonction de délai simple
void delay(int cycles) {
    volatile int i;
    for(i = 0; i < cycles; i++);
}

int main() {
    int speed;
    
    // Allumer LED pour indiquer que le programme tourne
    IOWR(LEDS_BASE, 0xFF);
    
    // PHASE 1: ACCÉLÉRATION
    // Rampe d'accélération de SPEED_MIN à SPEED_MAX
    for(speed = SPEED_MIN; speed <= SPEED_MAX; speed += ACCEL_STEP) {
        IOWR(MOTOR_RIGHT_BASE, GO_BIT | speed);
        IOWR(MOTOR_LEFT_BASE, GO_BIT | speed);
        delay(DELAY_CYCLES);
    }
    
    // PHASE 2: VITESSE CONSTANTE
    // Maintenir la vitesse maximale pendant un moment
    speed = SPEED_MAX;
    IOWR(MOTOR_RIGHT_BASE, GO_BIT | speed);
    IOWR(MOTOR_LEFT_BASE, GO_BIT | speed);
    delay(DELAY_CYCLES * 100);  // Maintien plus long
    
    // PHASE 3: DÉCÉLÉRATION
    // Rampe de décélération de SPEED_MAX à SPEED_MIN
    for(speed = SPEED_MAX; speed >= SPEED_MIN; speed -= ACCEL_STEP) {
        IOWR(MOTOR_RIGHT_BASE, GO_BIT | speed);
        IOWR(MOTOR_LEFT_BASE, GO_BIT | speed);
        delay(DELAY_CYCLES);
    }
    
    // PHASE 4: ARRÊT
    // Arrêter les moteurs (retirer le bit GO)
    IOWR(MOTOR_RIGHT_BASE, 0);
    IOWR(MOTOR_LEFT_BASE, 0);
    
    // Éteindre les LEDs pour indiquer la fin
    IOWR(LEDS_BASE, 0x00);
    
    // Boucle infinie
    while(1);
    
    return 0;
}
