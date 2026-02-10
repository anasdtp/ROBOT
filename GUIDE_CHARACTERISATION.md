# Guide de Caractérisation des Moteurs

Programme : `motor_characterization.c`

## Contrôles

| Switch | Fonction |
|--------|----------|
| SW0 | Augmenter vitesse (+50) |
| SW1 | Diminuer vitesse (-50) |
| SW2 | Tester moteur DROIT seul |
| SW3 | Tester moteur GAUCHE seul |
| SW4 | Tester les DEUX moteurs |
| SW5 | Mode calibration ligne droite |
| SW6 | Basculer droit/gauche (mode calib) |
| SW7 | ARRÊT moteurs |

## LEDs

- **Bits 0-5** : Vitesse courante / 50 (0-127 affiché)
- **Bit 6** : Moteur gauche actif (SW3)
- **Bit 7** : Moteur droit actif (SW2)
- **Pattern 0xAA** : Entrée en mode calibration
- **Pattern 0x55** : Sortie du mode calibration

## Protocole de Test

### TEST 1 : Vitesse minimale hors sol

**Objectif** : Trouver la vitesse minimale pour que les roues tournent (robot suspendu)

**Procédure** :
1. Suspendre le robot (roues en l'air)
2. Appuyer sur **SW7** pour arrêter les moteurs (vitesse = 0)
3. Appuyer plusieurs fois sur **SW0** pour augmenter la vitesse par pas de 50
4. Appuyer sur **SW4** pour démarrer les deux moteurs
5. Observer si les roues tournent
6. Si non, **SW7** → augmenter avec **SW0** → **SW4** à nouveau
7. Répéter jusqu'à ce que les roues tournent
8. Noter la vitesse affichée sur les LEDs (multiplier par 50)

**Exemple** : 
- LEDs montrent `00100000` (bit 5 = 32 en décimal)
- Vitesse = 32 × 50 = **1600**

---

### TEST 2a : Vitesse minimale au sol SANS piles

**Objectif** : Trouver la vitesse minimale pour avancer au sol (robot léger)

**Procédure** :
1. Placer le robot au sol, **sans les piles**
2. Répéter la procédure du TEST 1 mais observer le déplacement au sol
3. Noter la vitesse minimale

**Résultat attendu** : Vitesse supérieure au TEST 1 (friction au sol)

---

### TEST 2b : Vitesse minimale au sol AVEC piles

**Objectif** : Trouver la vitesse minimale pour avancer au sol (robot chargé)

**Procédure** :
1. Installer les piles dans le robot
2. Répéter la procédure du TEST 1 mais observer le déplacement au sol
3. Noter la vitesse minimale

**Résultat attendu** : Vitesse supérieure au TEST 2a (plus de poids)

---

### TEST 3 : Calibration ligne droite

**Objectif** : Ajuster les vitesses des moteurs pour un trajet rectiligne

**Procédure** :

1. **Initialisation**
   - Partir d'une vitesse moyenne (ex: 2000)
   - Utiliser **SW0** pour monter à ~40 sur les LEDs (2000 / 50 = 40)
   - Appuyer sur **SW4** pour démarrer les deux moteurs

2. **Observation**
   - Le robot dévie-t-il à gauche ? → Moteur gauche trop lent
   - Le robot dévie-t-il à droite ? → Moteur droit trop lent

3. **Entrer en mode calibration**
   - Appuyer sur **SW7** pour arrêter
   - Appuyer sur **SW5** → LEDs clignotent `0xAA` (mode calib activé)
   - Par défaut, vous ajustez le moteur DROIT

4. **Ajuster le moteur droit**
   - **SW0** pour augmenter la vitesse du moteur droit
   - **SW1** pour diminuer la vitesse du moteur droit
   - **SW2** pour tester le moteur droit seul
   - LED7 clignote = vous ajustez le moteur droit

5. **Ajuster le moteur gauche**
   - Appuyer sur **SW6** pour basculer vers le moteur GAUCHE
   - LED6 clignote = vous ajustez le moteur gauche
   - **SW0** pour augmenter la vitesse du moteur gauche
   - **SW1** pour diminuer la vitesse du moteur gauche
   - **SW3** pour tester le moteur gauche seul

6. **Tester ensemble**
   - **SW4** pour tester les deux moteurs simultanément
   - Observer la trajectoire
   - **SW7** pour arrêter et ajuster à nouveau si nécessaire

7. **Finaliser**
   - Une fois satisfait, appuyer sur **SW5** pour sortir du mode calib
   - Les vitesses sont conservées

**Exemple d'ajustement** :
- Robot dévie à gauche → Moteur gauche trop lent
- **SW6** pour sélectionner moteur gauche
- **SW0** plusieurs fois pour augmenter
- **SW4** pour tester → Trajectoire plus droite
- Affiner jusqu'à trajectoire rectiligne

**Note** : Noter les valeurs finales :
- Vitesse moteur droit = _______
- Vitesse moteur gauche = _______

---

### TEST 4 : Hystéresis

**Objectif** : Trouver la vitesse minimale en mouvement sans arrêt

**Procédure** :

1. **Démarrage**
   - Partir d'une vitesse confortable (ex: 2000)
   - **SW4** pour démarrer les deux moteurs
   - Attendre que le robot soit en mouvement stable (2-3 secondes)

2. **Décélération progressive**
   - Appuyer sur **SW1** pour diminuer la vitesse par pas de 50
   - Observer si le robot continue d'avancer
   - Continuer à diminuer tant que le robot avance

3. **Point critique**
   - Observer quand une roue commence à ralentir ou s'arrêter
   - Noter la vitesse sur les LEDs × 50
   - Cette vitesse = vitesse minimale en mouvement

4. **Calcul de l'hystérésis**
   ```
   Hystérésis = Vitesse_min_démarrage (TEST 2) - Vitesse_min_mouvement (TEST 4)
   ```

**Exemple** :
- TEST 2 : Vitesse min démarrage = 1800
- TEST 4 : Vitesse min mouvement = 1200
- Hystéresis = 1800 - 1200 = **600** (hystéresis présent)

**Interprétation** :
- Hystéresis > 0 : Il faut plus de couple pour démarrer que pour maintenir le mouvement (normal)
- Hystéresis ≈ 0 : Pas d'hystérésis significative

---

## Tableau de Résultats

Remplir au fur et à mesure :

| Test | Description | Vitesse (valeur décimale) |
|------|-------------|---------------------------|
| 1 | Vitesse min hors sol | _______ |
| 2a | Vitesse min sol sans piles | _______ |
| 2b | Vitesse min sol avec piles | _______ |
| 3 | Vitesse moteur DROIT (ligne droite) | _______ |
| 3 | Vitesse moteur GAUCHE (ligne droite) | _______ |
| 4 | Vitesse min en mouvement | _______ |
| 4 | Hystéresis calculée | _______ |

---

## Trucs et Astuces

### Lecture des LEDs
- Si toutes les LEDs sont éteintes : vitesse = 0
- Compter les LEDs allumées en binaire : 
  - LED0 = 1, LED1 = 2, LED2 = 4, LED3 = 8, LED4 = 16, LED5 = 32
  - Exemple : LED2 + LED5 = 4 + 32 = 36 → vitesse = 36 × 50 = 1800

### Si le robot ne démarre pas
- Vérifier que le driver moteur est alimenté (MTR_Sleep_n = 1)
- Vérifier les connexions moteurs
- Augmenter la vitesse progressivement
- Tester chaque moteur individuellement (SW2 et SW3)

### Si le robot tourne sur lui-même
- Un moteur est plus rapide que l'autre
- Utiliser le mode calibration (SW5) pour égaliser

### Pour des ajustements fins
- Modifier `#define STEP 50` dans le code source
- Recompiler avec un pas de 10 ou 25 pour plus de précision
