# État du Projet Robot Suiveur de Ligne

## 📋 Vue d'ensemble

Ce document explique le **but général du projet** et l'**état d'avancement** par rapport aux 6 étapes requises.

### But du projet
Implémenter un **robot autonome** capable de :
1. Démarrer au repos
2. Détecter une ligne noire au sol
3. Suivre cette ligne en temps réel
4. Gérer les virages (recherche de ligne)
5. Gérer les aller-retours sur la piste

### Architecture matérielle
- **FPGA Intel Altera** (carte DE-series)
- **Processeur NIOS II** (system-on-chip logiciel)
- **Mémoire SDRAM** pour le stockage
- **7 capteurs infrarouges** pour détection de la ligne
- **2 moteurs DC** piloté par driver DRV8848
- **ADC LTC2308** pour numériser les capteurs

---

## 🎯 État par étape

### ✅ Étape 1 : Architecture NIOS-SDRAM-PIO (20 pts)
**Status : TERMINÉE**

#### Qu'est-ce qui a été fait ?
L'architecture système complète est implémentée dans [ROBOT.vhd](ROBOT.vhd) :

**Composants intégrés :**
- ✅ Système Qsys `nios_system_sdram`
  - Processeur NIOS II
  - Contrôleur SDRAM
  - PIOs (ports parallèles) pour switches et LEDs
  - PIOs pour moteurs (14 bits chacun)
  - PIOs pour capteurs

- ✅ Capteurs ADC `capteurs_sol`
  - Interface SPI avec ADC LTC2308
  - 7 canaux de capteurs infrarouges

- ✅ Génération PWM `PWM_generation`
  - Conversion des commandes 14-bits en signaux PWM
  - Fréquence : 16 kHz
  - Contrôle bidirectionnel des moteurs

**Schéma de connexion :**
```
NIOS II (Qsys)
    ├─ Entrées : SW[7:0], MTR_Fault_n, ADC_SDO
    ├─ Sorties : LED[7:0], MTRR_P/N, MTRL_P/N
    └─ SDRAM : signaux standard

Capteurs ADC (capteurs_sol)
    ├─ Entrée : 2 kHz clock-divider du FPGA
    ├─ Sortie : 7 canaux × 8 bits à 2 kHz
    └─ Contrôle moteurs : signaux SPI vers LTC2308

Génération PWM (PWM_generation)
    ├─ Entrée : 14 bits × 2 moteurs
    └─ Sortie : 4 signaux PWM (P/N pour chaque moteur)
```

#### À faire
- ⏳ **Validation** : Vérifier que tous les signaux circulent correctement

---

### ⏳ Étape 2 : Caractérisation des moteurs (20 pts)
**Status : PARTIELLEMENT TERMINÉE**

#### Qu'est-ce qui a été fait ?
- ✅ Module PWM `PWM_generation.vhd` implémenté et intégré
- ✅ Programme C de test : [motor_characterization.c](app_sofware/motor_characterization.c)
- ✅ Programme C simple : [motor_test.c](app_sofware/motor_test.c)
- ✅ Documentation : [GUIDE_CHARACTERISATION.md](GUIDE_CHARACTERISATION.md)

#### Qu'est-ce que fait le module PWM ?
Le module `PWM_generation` génère des signaux PWM pour piloter les moteurs DC :

```vhdl
Format entrée (14 bits) :
  Bit 13 : GO/STOP (1 = marche, 0 = arrêt)
  Bit 12 : DIRECTION (0 = avant, 1 = arrière)
  Bits 11-0 : VITESSE (0 à 4095)

Sortie :
  4 signaux PWM : MTRR_P, MTRR_N, MTRL_P, MTRL_N
  Fréquence : 16 kHz (50 MHz / 3125)
```

#### À faire
**3 tests de caractérisation obligatoires :**

1. **TEST 1 : Vitesse minimale hors sol** 🔄
   - Suspendre le robot (roues en l'air)
   - Trouver la vitesse minimale pour que les roues tournent
   - Noter cette valeur

2. **TEST 2a : Vitesse minimale au sol (sans piles)** 🔄
   - Robot au sol, allégé (sans batteries)
   - Trouver la vitesse minimale pour avancer
   - Noter cette valeur

3. **TEST 2b : Vitesse minimale au sol (avec piles)** 🔄
   - Robot au sol avec batteries
   - Trouver la vitesse minimale pour avancer
   - Noter cette valeur

4. **TEST 3 : Calibration ligne droite** 🔄
   - Ajuster les vitesses des moteurs pour une trajectoire rectiligne
   - Documenter les déltas entre moteur droit/gauche


**Résultats attendus :**
```
Vitesse minimale hors sol (roues) : ~1500-2000
Vitesse minimale au sol (sans piles) : ~2000-2500
Vitesse minimale au sol (avec piles) : ~2500-3500
Facteur de correction moteurs : ±5-10%
```

---

### ✅ Étape 3 : Calcul de la position du robot (20 pts)
**Status : TERMINÉE**

#### Qu'est-ce qui a été fait ?
- ✅ Module `position_ligne` implémenté et intégré dans [ROBOT.vhd](ROBOT.vhd)
- ✅ Module `capteurs_sol` pour lecture 7 capteurs ADC
- ✅ Affichage de la position sur LEDs

#### Qu'est-ce que font ces modules ?

**Module `capteurs_sol` :**
- Lit 7 capteurs infrarouges via interface SPI
- Fréquence : 2 kHz
- Retourne : 7 × 8 bits (capteurs seuillés)

**Module `position_ligne` :**
- Entrée : 7 bits capteurs
- Traitement **combinatoire** :
  - Cherche une séquence de `1` consécutifs
  - Calcule la position centrale : de -3 (extrême gauche) à +3 (extrême droite)
  - Détecte perte de ligne (`line_lost`)
- Sortie : 4 bits position + 1 bit perte de ligne

```vhdl
Mapping LED pour position :
  -3 : LED0 (extrême gauche)
  -2 : LED1
  -1 : LED2
   0 : LED3 (centre)
  +1 : LED4
  +2 : LED5
  +3 : LED7 (extrême droite)
```

#### À faire
- ⏳ **Validation** : Tester sur une piste avec ligne noire, vérifier affichage LEDs

---

### ⏳ Étape 4 : Suivi de ligne (20 pts)
**Status : PARTIELLEMENT TERMINÉE**

#### Qu'est-ce qui a été fait ?
- ✅ Module `ctl_sl` (PID controller) implémenté et intégré
- ✅ Module `MUX` (correcteur moteurs) implémenté et intégré
- ✅ Pipeline complet de suivi

#### Qu'est-ce que font ces modules ?

**Module `ctl_sl` (PID Line Following) :**
```
Entrée : position_ligne (-3 à +3)
Sortie : correction PWM (14 bits)

Logique :
  - Position = 0 → correction = 0 (ligne droite)
  - Position < 0 → correction > 0 (tourner à droite)
  - Position > 0 → correction < 0 (tourner à gauche)
```

**Module `MUX` (Motor Correction) :**
```
Entrée : 
  - motor_right_in : commande du moteur droit
  - motor_left_in : commande du moteur gauche
  - correction_in : correction PID

Traitement :
  motor_right_out = motor_right_in + correction
  motor_left_out = motor_left_in - correction
  (saturation à ±8191)

Résultat :
  - Si nécessaire tourner à droite : droite accélère, gauche ralentit
  - Si nécessaire tourner à gauche : gauche accélère, droite ralentit
```

#### À faire
**Blocages actuels :**
- ⚠️ Le module `ctl_sl` est déclaré mais **PAS IMPLÉMENTÉ** en VHDL
- ⚠️ Aucun lien avec le C (superviseur NIOS) pour envoyer les commandes moteur

**À faire :**
1. **Implémenter le contrôleur PID** (`ctl_sl.vhd`) 🔄
   - Entrée : position de ligne (-3 à +3)
   - Signal `ready` pour synchroniser
   - Sortie : correction (14 bits signés)
   
   **Formule simple de correction :**
   ```c
   correction = KP * position + KI * integral + KD * derivative
   // Ou plus simple : correction = KP * position
   // Ajuster KP selon les tests
   ```

2. **Implémenter le superviseur C** 🔄
   - Lire les commandes utilisateur
   - Envoyer vitesse de base aux moteurs
   - Laisser la correction PID s'appliquer en hardware

3. **Tester le suivi de ligne** 🔄
   - Placer le robot sur une ligne noire
   - Vérifier qu'il suit sans dévier

---

### ❌ Étape 5 : Gestion de la rotation (20 pts)
**Status : PAS COMMENCÉE**

#### Qu'est-ce qu'il faut faire ?
Implémenter un **automate de rotation** qui :
- Attend un signal `start_rot` du C
- Reçoit direction : `dir_rot` (0 = gauche, 1 = droite)
- Tourne en place jusqu'à retrouver la ligne au centre
- Signale `fin_rot` quand positionné

**Module à créer :** `ctl_rot.vhd`

#### Implémentation suggérée

```vhdl
ENTITY ctl_rot IS
PORT (
  clk : IN STD_LOGIC;
  reset_n : IN STD_LOGIC;
  start_rot : IN STD_LOGIC;
  dir_rot : IN STD_LOGIC;  -- 0=gauche, 1=droite
  position_in : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
  ready : IN STD_LOGIC;
  correction : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
  fin_rot : OUT STD_LOGIC
);
END ctl_rot;
```

**Automate :**
```
État IDLE:
  - Attendre start_rot = 1
  - Aller à ROTATION

État ROTATION:
  - Si dir_rot = 0 : correction = -max (tourne gauche)
  - Si dir_rot = 1 : correction = +max (tourne droite)
  - Si position_in = 0 (ligne au centre) :
    - fin_rot = 1
    - Aller à IDLE
```

---

### ❌ Étape 6 : Gestion des aller-retours (20 pts)
**Status : PAS COMMENCÉE**

#### Qu'est-ce qu'il faut faire ?
Implémenter un **superviseur NIOS II en C** qui pilote l'automate global :

**États du robot :**
```
À l'initialisation :
  ├─ IDLE : Attendre détection de ligne
  │
  ├─ LINE_FOLLOWING : Suivre la ligne
  │  └─ Quand fin_SL = 1 → pause 1s → ROTATION
  │
  ├─ ROTATION_SEARCH : Tourner pour retrouver la ligne
  │  └─ Quand fin_rot = 1 → pause 1s → LINE_FOLLOWING
  │
  └─ Répéter indéfiniment
```

**Programme C requis :** [ROBOT.c](app_sofware/ROBOT.c) ou nouveau fichier

```c
#include "system.h"
#include "alt_types.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>

// États du superviseur
enum State { WAIT_LINE, LINE_FOLLOW, ROTATION, PAUSE };

int main() {
  enum State state = WAIT_LINE;
  int count = 0;
  
  while(1) {
    switch(state) {
      case WAIT_LINE:
        // Moteurs arrêtés, attendre détection
        if (sensor_line_detected()) {
          state = LINE_FOLLOW;
        }
        break;
        
      case LINE_FOLLOW:
        // start_SL = 1 (signaler au hardware)
        if (fin_SL_detected()) {
          // Pause 1 seconde
          usleep(1000000);
          state = ROTATION;
        }
        break;
        
      case ROTATION:
        // start_rot = 1, dir_rot = alternance gauche/droite
        if (fin_rot_detected()) {
          // Pause 1 seconde
          usleep(1000000);
          state = LINE_FOLLOW;
        }
        break;
    }
  }
  return 0;
}
```

#### À faire
1. ⚠️ Créer/modifier [ROBOT.c](app_sofware/ROBOT.c) avec le superviseur
2. ⚠️ Tester les aller-retours complets
3. ⚠️ Valider la cohérence des rotations (gauche/droite)

---

## 📊 Résumé de l'avancement

| Étape | Nom | Status | % Fait |
|-------|-----|--------|--------|
| 1 | Architecture NIOS-SDRAM-PIO | ✅ Complet | 100% |
| 2 | Caractérisation moteurs | ⏳ Tests à faire | 40% |
| 3 | Calcul position robot | ✅ Complet | 100% |
| 4 | Suivi de ligne | ⚠️ | 60% |
| 5 | Gestion rotation | ❌ À faire | 0% |
| 6 | Aller-retours | ❌ À faire | 0% |
| **TOTAL** | | | **50%** |

---

## 🚀 Prochaines étapes prioritaires

### Priorité 1 (Pour fonctionnement de base)
1. ✅ Valider l'architecture NIOS (étape 1)
2. 🔄 Faire les 3 tests de moteurs (étape 2)
3. ✅ Vérifier les capteurs et position (étape 3)
4. 🔄 **Implémenter et tester le PID** (étape 4) ← À faire immédiatement

### Priorité 2 (Pour autonomie)
5. 🔄 **Implémenter l'automate de rotation** (étape 5)
6. 🔄 **Implémenter le superviseur NIOS** (étape 6)

---

## 📁 Structure des fichiers

```
ROBOT/
├── ROBOT.vhd                    ← Architecture principale
├── PWM_generation.vhd           ← Génération PWM (✅ fait)
├── capteurs_sol.vhd             ← Lecture ADC (✅ fait)
├── position_ligne.vhd           ← Calcul position (✅ fait)
├── ctl_sl.vhd                   ← Contrôleur PID (⚠️ À implémenter)
├── ctl_rot.vhd                  ← Rotation (❌ À créer)
├── MUX.vhd                       ← Correcteur moteurs (✅ fait)
├── nios_system_sdram.qsys       ← Système Qsys
├── nios_system_sdram.sopcinfo   ← Info Qsys générée
│
├── app_sofware/
│   ├── motor_characterization.c ← Tests moteurs (✅ existe)
│   ├── motor_test.c             ← Test simple (✅ existe)
│   ├── motor_simple.c           ← Exemple simple (✅ existe)
│   ├── sensors.c                ← Lecture capteurs (✅ existe)
│   ├── ROBOT.c                  ← Superviseur (⚠️ À compléter)
│   ├── line_follower.c          ← Suivi ligne (⚠️ À vérifier)
│   └── triangle.c               ← Géométrie (✅ existe)
│
├── GUIDE_CHARACTERISATION.md    ← Procédure tests (✅ fait)
├── PWM_INTEGRATION.md           ← Intégration PWM (✅ fait)
├── TRIANGLE_GUIDE.md            ← Guide triangle (✅ fait)
├── IMPORTANT_ADDRESSES.md       ← Adresses mémoire (✅ fait)
└── ETAT_DU_PROJET.md           ← Ce fichier
```

---

## ⚠️ Points critiques à vérifier

### Avant de tester
- [ ] Vérifier adresses Qsys dans [IMPORTANT_ADDRESSES.md](IMPORTANT_ADDRESSES.md)
- [ ] Mettre à jour `motor_test.c` avec bonnes adresses
- [ ] Créer/compléter le BSP (Board Support Package)
- [ ] Compiler le design Quartus complet

### Pendant les tests
- [ ] Observer les LEDs pour chaque étape
- [ ] Documenter les valeurs de calibration
- [ ] Vérifier que les moteurs ne chauffent pas
- [ ] Attention au driver NMOS DRV8848 (brochage !)

### Après chaque étape
- [ ] Archiver les résultats de test
- [ ] Noter les paramètres calibrés
- [ ] Mettre à jour ce document

---

## 📞 Notes et références

- **Fréquence FPGA :** 50 MHz
- **Fréquence capteurs :** 2 kHz (divider 25000)
- **Fréquence PWM :** 16 kHz
- **Format moteurs :** 14 bits (Go + Dir + Speed)
- **Vitesse typique :** 2000-3500 (sur 0-4095)
- **Capteurs :** 7 × 8 bits, à 2 kHz via ADC LTC2308
- **LED feedback :** Position ligne affichée en live

---

**Dernière mise à jour :** Mars 2026  
**Version de ce document :** 1.0
