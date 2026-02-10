/*
 * Test moteur ultra-minimaliste
 * Les moteurs démarrent automatiquement en marche avant
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

int main() {
    int speed = 0x3A00;
    
    // Allumer LED pour indiquer que le programme tourne
    IOWR(LEDS_BASE, 0xFF);
    
    // Démarrer les deux moteurs en marche avant
    IOWR(MOTOR_RIGHT_BASE, GO_BIT | speed);
    IOWR(MOTOR_LEFT_BASE, GO_BIT | speed);
    
    // Boucle infinie
    while(1);
    
    return 0;
}
