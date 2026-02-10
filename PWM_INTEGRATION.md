# Intégration du contrôle PWM des moteurs

## Modifications effectuées

### 1. Fichiers ajoutés
- **PWM_generation.vhd** : Module VHDL de génération PWM pour contrôle moteurs
- **app_sofware/motor_test.c** : Programme C de caractérisation des moteurs
- **ROBOT.vhd** : Modifié pour intégrer le PWM

### 2. Modifications VHDL (ROBOT.vhd)
- Ajout des ports moteurs : `MOTOR_R_P`, `MOTOR_R_N`, `MOTOR_L_P`, `MOTOR_L_N`
- Ajout du composant `PWM_generation`
- Connexion du PWM au système Nios II via signaux internes

## Modifications QSYS REQUISES ⚠️

Vous **DEVEZ** modifier votre système Qsys (`nios_system_sdram.qsys`) :

### Étapes dans Qsys:

1. **Ouvrir Qsys**
   - Ouvrir le fichier `nios_system_sdram.qsys`

2. **Ajouter PIO pour moteur droit**
   - Clic droit → Add Component → PIO (Parallel I/O)
   - Nom : `motor_right`
   - Configuration :
     - Width: **14 bits**
     - Direction: **Output**
     - Reset value: 0x0000
   - Connecter à `clk` et `reset`
   - Connecter au bus Avalon du Nios II

3. **Ajouter PIO pour moteur gauche**
   - Clic droit → Add Component → PIO (Parallel I/O)
   - Nom : `motor_left`
   - Configuration :
     - Width: **14 bits**
     - Direction: **Output**
     - Reset value: 0x0000
   - Connecter à `clk` et `reset`
   - Connecter au bus Avalon du Nios II

4. **Assigner les adresses**
   - System → Assign Base Addresses
   - Noter les adresses assignées (elles seront dans system.h)

5. **Export des signaux**
   - Double-cliquer sur `motor_right` → Export signal `external_connection`
   - Renommer en `motor_right_export`
   - Double-cliquer sur `motor_left` → Export signal `external_connection`
   - Renommer en `motor_left_export`

6. **Générer le système**
   - Generate HDL → Generate
   - Attendre la fin de la génération

7. **Dans Quartus II**
   - Ajouter `PWM_generation.vhd` au projet
   - Compiler le projet complet
   - Télécharger le .sof sur le FPGA

8. **Dans le logiciel Nios II**
   - Créer un nouveau projet BSP qui pointe vers `nios_system_sdram.sopcinfo`
   - Les bases `MOTOR_RIGHT_BASE` et `MOTOR_LEFT_BASE` seront automatiquement définies dans `system.h`
   - Compiler `motor_test.c`

## Format des données moteur (14 bits)

```
Bit 13: GO/STOP    (1=marche, 0=arrêt)
Bit 12: DIRECTION  (0=avant, 1=arrière)
Bits 11-0: VITESSE (0 à 4095)
```

## Exemple d'utilisation

```c
#define MOTOR_GO_BIT      (1 << 13)
#define MOTOR_DIR_BIT     (1 << 12)
#define MOTOR_FORWARD(speed)  (MOTOR_GO_BIT | (speed & 0xFFF))
#define MOTOR_BACKWARD(speed) (MOTOR_GO_BIT | MOTOR_DIR_BIT | (speed & 0xFFF))

// Avancer à vitesse 2000
IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_RIGHT_BASE, MOTOR_FORWARD(2000));
IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_LEFT_BASE, MOTOR_FORWARD(2000));

// Arrêter
IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_RIGHT_BASE, 0x0000);
IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_LEFT_BASE, 0x0000);
```

## Assignation des pins (à ajouter dans ROBOT.qsf)

Vous devez assigner les pins des moteurs selon votre carte:

```tcl
# Motor Right
set_location_assignment PIN_XX -to MOTOR_R_P
set_location_assignment PIN_XX -to MOTOR_R_N

# Motor Left  
set_location_assignment PIN_XX -to MOTOR_L_P
set_location_assignment PIN_XX -to MOTOR_L_N
```

Remplacez `PIN_XX` par les vrais numéros de pins de votre carte.

## Tests de caractérisation

Le programme `motor_test.c` permet de:
1. Déterminer la vitesse minimale hors sol
2. Déterminer la vitesse minimale au sol (avec/sans piles)
3. Calibrer pour mouvement rectiligne
4. Mesurer l'hystérésis

## Architecture du système

```
┌─────────────┐
│  Nios II    │
│   CPU       │
└──────┬──────┘
       │ Avalon Bus
       ├────────────┬────────────┐
       │            │            │
  ┌────▼─────┐ ┌───▼─────┐ ┌───▼─────┐
  │ motor_   │ │ motor_  │ │ autres  │
  │ right    │ │ left    │ │ PIOs    │
  │ (14 bits)│ │(14 bits)│ │         │
  └────┬─────┘ └───┬─────┘ └─────────┘
       │           │
       │           │
  ┌────▼───────────▼─────┐
  │  PWM_generation      │
  │  (VHDL)              │
  └────┬───────────┬─────┘
       │           │
    R_P,R_N    L_P,L_N
       │           │
    [Motor]    [Motor]
```

## Fréquence PWM

- Fréquence FPGA: 50 MHz
- Fréquence PWM: 16 kHz
- Résolution: 12 bits (0-4095)

## Prochaines étapes

1. ✅ Créer PWM_generation.vhd
2. ✅ Modifier ROBOT.vhd
3. ⚠️  **Modifier Qsys** (ajouter les 2 PIOs)
4. ⚠️  Assigner les pins moteurs dans QSF
5. ⚠️  Compiler le projet Quartus
6. ⚠️  Créer le BSP Nios II
7. ⚠️  Compiler et tester motor_test.c
