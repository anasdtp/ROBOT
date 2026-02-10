/*
 * Programme de caractérisation des moteurs - Version légère
 * Pas de printf, contrôle par switches, retour visuel par LEDs
 * 
 * CONTROLES:
 * SW0 : Augmenter vitesse (+50)
 * SW1 : Diminuer vitesse (-50)
 * SW2 : Tester moteur DROIT seul
 * SW3 : Tester moteur GAUCHE seul
 * SW4 : Tester les DEUX moteurs (marche avant)
 * SW5 : Mode calibration ligne droite (ajuster indépendamment)
 * SW6 : Basculer entre ajustement droit/gauche (en mode calib)
 * SW7 : ARRÊT moteurs
 * 
 * LEDs indiquent la vitesse courante divisée par 50 (0-255)
 */

// Adresses des périphériques
#define SWITCHES_BASE    0x00002000
#define LEDS_BASE        0x00002010
#define MOTOR_RIGHT_BASE 0x00001010
#define MOTOR_LEFT_BASE  0x00001000

// Accès mémoire
#define IOWR(base, data) (*((volatile unsigned int*)(base)) = data)
#define IORD(base)       (*((volatile unsigned int*)(base)))

// Bits de contrôle moteur
#define GO_BIT      0x2000  // Bit 13
#define DIR_BIT     0x1000  // Bit 12
#define SPEED_MASK  0x0FFF  // Bits 11-0

// Constantes (valeurs déterminées expérimentalement)
#define MIN_SPEED   0x3700   // 1792 - Vitesse minimale fonctionnelle
#define MAX_SPEED   0x3FF0   // 4080 - Vitesse maximale sûre
#define STEP        50
#define DEBOUNCE    50000

// Anti-rebond simple
void delay(int count) {
    volatile int i;
    for(i = 0; i < count; i++);
}

// Attendre que les switches soient relâchés
void wait_release() {
    while(IORD(SWITCHES_BASE) != 0)
        delay(1000);
    delay(DEBOUNCE);
}

int main() {
    int speed_right = MIN_SPEED;
    int speed_left = MIN_SPEED;
    int speed_common = MIN_SPEED;
    int mode = 0;  // 0=normal, 1=calibration
    int adjust_right = 1;  // En mode calib: 1=ajuster droit, 0=ajuster gauche
    unsigned int sw, last_sw = 0;
    
    // LED clignotement pour indiquer démarrage
    IOWR(LEDS_BASE, 0xFF);
    delay(500000);
    IOWR(LEDS_BASE, 0x00);
    delay(500000);
    IOWR(LEDS_BASE, 0xFF);
    delay(500000);
    IOWR(LEDS_BASE, 0x00);
    
    // Boucle principale
    while(1) {
        sw = IORD(SWITCHES_BASE);
        
        // Ne traiter que les changements (front montant)
        if(sw != last_sw && sw != 0) {
            
            // SW7 : Arrêt
            if(sw & 0x80) {
                IOWR(MOTOR_RIGHT_BASE, 0);
                IOWR(MOTOR_LEFT_BASE, 0);
                IOWR(LEDS_BASE, 0x00);
                speed_common = MIN_SPEED;
                speed_right = MIN_SPEED;
                speed_left = MIN_SPEED;
                wait_release();
            }
            
            // SW0 : Augmenter vitesse
            else if(sw & 0x01) {
                if(mode == 0) {
                    // Mode normal
                    speed_common += STEP;
                    if(speed_common > MAX_SPEED) speed_common = MAX_SPEED;
                    speed_right = speed_common;
                    speed_left = speed_common;
                } else {
                    // Mode calibration
                    if(adjust_right) {
                        speed_right += STEP;
                        if(speed_right > MAX_SPEED) speed_right = MAX_SPEED;
                    } else {
                        speed_left += STEP;
                        if(speed_left > MAX_SPEED) speed_left = MAX_SPEED;
                    }
                }
                wait_release();
            }
            
            // SW1 : Diminuer vitesse
            else if(sw & 0x02) {
                if(mode == 0) {
                    // Mode normal
                    speed_common -= STEP;
                    if(speed_common < MIN_SPEED) speed_common = MIN_SPEED;
                    speed_right = speed_common;
                    speed_left = speed_common;
                } else {
                    // Mode calibration
                    if(adjust_right) {
                        speed_right -= STEP;
                        if(speed_right < MIN_SPEED) speed_right = MIN_SPEED;
                    } else {
                        speed_left -= STEP;
                        if(speed_left < MIN_SPEED) speed_left = MIN_SPEED;
                    }
                }
                wait_release();
            }
            
            // SW2 : Tester moteur DROIT seul
            else if(sw & 0x04) {
                IOWR(MOTOR_RIGHT_BASE, GO_BIT | (speed_right & SPEED_MASK));
                IOWR(MOTOR_LEFT_BASE, 0);
                IOWR(LEDS_BASE, (speed_right / 50) | 0x80);  // Bit 7 = moteur droit
                wait_release();
            }
            
            // SW3 : Tester moteur GAUCHE seul
            else if(sw & 0x08) {
                IOWR(MOTOR_RIGHT_BASE, 0);
                IOWR(MOTOR_LEFT_BASE, GO_BIT | (speed_left & SPEED_MASK));
                IOWR(LEDS_BASE, (speed_left / 50) | 0x40);  // Bit 6 = moteur gauche
                wait_release();
            }
            
            // SW4 : Tester les DEUX moteurs
            else if(sw & 0x10) {
                IOWR(MOTOR_RIGHT_BASE, GO_BIT | (speed_right & SPEED_MASK));
                IOWR(MOTOR_LEFT_BASE, GO_BIT | (speed_left & SPEED_MASK));
                // Afficher la moyenne sur les LEDs
                int avg = ((speed_right + speed_left) / 2) / 50;
                IOWR(LEDS_BASE, avg);
                wait_release();
            }
            
            // SW5 : Basculer mode calibration
            else if(sw & 0x20) {
                mode = !mode;
                if(mode) {
                    // Entrer en mode calibration
                    IOWR(LEDS_BASE, 0xAA);  // Pattern pour indiquer mode calib
                    delay(500000);
                } else {
                    // Sortir du mode calibration
                    IOWR(LEDS_BASE, 0x55);
                    delay(500000);
                    speed_common = (speed_right + speed_left) / 2;
                }
                wait_release();
            }
            
            // SW6 : Basculer entre ajustement droit/gauche (mode calib uniquement)
            else if(sw & 0x40) {
                if(mode == 1) {
                    adjust_right = !adjust_right;
                    if(adjust_right) {
                        // Clignoter LED7 (droit)
                        IOWR(LEDS_BASE, 0x80);
                        delay(300000);
                        IOWR(LEDS_BASE, 0x00);
                    } else {
                        // Clignoter LED6 (gauche)
                        IOWR(LEDS_BASE, 0x40);
                        delay(300000);
                        IOWR(LEDS_BASE, 0x00);
                    }
                }
                wait_release();
            }
        }
        
        last_sw = sw;
        delay(10000);
    }
    
    return 0;
}
