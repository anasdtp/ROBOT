# IMPORTANT: Vérification des adresses mémoire

## ⚠️ AVANT DE COMPILER motor_test.c

Le fichier `motor_test.c` utilise des adresses mémoire **provisoires**. Vous **DEVEZ** les vérifier et les corriger après avoir généré votre système Qsys.

### Étapes à suivre:

1. **Dans Platform Designer (Qsys)**:
   - Ouvrir `nios_system_sdram.qsys`
   - Cliquer sur **System → Assign Base Addresses**
   - Noter les adresses assignées dans la colonne "Base"

2. **Vérifier les adresses dans motor_test.c**:

```c
// Lignes 13-16 dans motor_test.c
#define SWITCHES_BASE    0x00002000  // ← Vérifier
#define LEDS_BASE        0x00002010  // ← Vérifier  
#define MOTOR_RIGHT_BASE 0x00002020  // ← Vérifier
#define MOTOR_LEFT_BASE  0x00002030  // ← Vérifier
```

3. **Trouver les vraies adresses**:
   - Dans Qsys, regarder la colonne "Base" pour chaque composant:
     - `switches` → noter l'adresse (ex: 0x00002000)
     - `LEDs` → noter l'adresse (ex: 0x00002010)
     - `motor_right` → noter l'adresse (ex: 0x00002020)
     - `motor_left` → noter l'adresse (ex: 0x00002030)

4. **Mettre à jour motor_test.c** avec les vraies adresses

5. **Compiler** dans le Monitor Program

### Alternative: Utiliser system.h (recommandé)

Si vous créez un projet BSP (Board Support Package) standard:

```c
#include "system.h"
#include "altera_avalon_pio_regs.h"

// Les adresses seront automatiquement définies:
// SWITCHES_BASE
// LEDS_BASE  
// MOTOR_RIGHT_BASE
// MOTOR_LEFT_BASE
```

### Structure du système actuel:

```
Adresse      Composant           Taille
-----------------------------------------
0x00000000   onchip_memory2_0    32KB
0x00001000   motor_left          16 bytes
0x00001010   motor_right         16 bytes
0x00002000   switches            16 bytes
0x00002010   LEDs                16 bytes
0x00002020   jtag_uart_0         8 bytes
0x04000000   SDRAM               32MB
```

**Note**: Ces adresses correspondent à la configuration Qsys actuelle visible dans Platform Designer.

---

## 🔧 Paramètres de Caractérisation des Moteurs

### Plage de Vitesse Fonctionnelle
Déterminée expérimentalement :

- **Vitesse minimale** : `0x700` (1792 décimal)
- **Vitesse maximale** : `0xFF0` (4080 décimal)
- **Codage** : 12 bits (bits 11-0 du registre de contrôle)

### Format du Registre Moteur (14 bits)

```
Bit 13 : GO/STOP   (1 = moteur activé, 0 = arrêté)
Bit 12 : DIRECTION (0 = avant, 1 = arrière)  
Bits 11-0 : VITESSE (valeur PWM, utiliser plage 0x700 - 0xFF0)
```

### Exemples de Valeurs Complètes

| Commande | Valeur Hex | Binaire | Description |
|----------|------------|---------|-------------|
| Marche avant min | `0x2700` | 0010 0111 0000 0000 | GO=1, DIR=0, Speed=0x700 |
| Marche avant moy | `0x2900` | 0010 1001 0000 0000 | GO=1, DIR=0, Speed=0x900 |
| Marche avant max | `0x2FF0` | 0010 1111 1111 0000 | GO=1, DIR=0, Speed=0xFF0 |
| Marche arrière min | `0x3700` | 0011 0111 0000 0000 | GO=1, DIR=1, Speed=0x700 |
| Marche arrière max | `0x3FF0` | 0011 1111 1111 0000 | GO=1, DIR=1, Speed=0xFF0 |
| Arrêt | `0x0000` | 0000 0000 0000 0000 | GO=0, DIR=0, Speed=0 |

### Constantes C Recommandées

```c
// Adresses périphériques
#define MOTOR_RIGHT_BASE 0x00001010
#define MOTOR_LEFT_BASE  0x00001000
#define LEDS_BASE        0x00002010
#define SWITCHES_BASE    0x00002000

// Bits de contrôle
#define GO_BIT      0x2000  // Bit 13
#define DIR_BIT     0x1000  // Bit 12

// Plage de vitesse
#define SPEED_MIN   0x700   // 1792 - Vitesse minimale fonctionnelle
#define SPEED_MAX   0xFF0   // 4080 - Vitesse maximale sûre
#define SPEED_MED   0x900   // 2304 - Vitesse moyenne

// Macros utiles
#define MOTOR_FORWARD(speed)  (GO_BIT | ((speed) & 0xFFF))
#define MOTOR_BACKWARD(speed) (GO_BIT | DIR_BIT | ((speed) & 0xFFF))
#define MOTOR_STOP            0x0000
```

### Notes de Caractérisation

- ⚠️ Ne pas utiliser de vitesses < 0x700 : les moteurs risquent de ne pas démarrer ou tourner de façon instable
- ⚠️ Ne pas dépasser 0xFF0 : limite de sécurité pour éviter le surchauffe
- Les deux moteurs peuvent nécessiter des vitesses légèrement différentes pour un trajet rectiligne
- Utiliser `motor_characterization.c` pour calibrer les vitesses exactes
