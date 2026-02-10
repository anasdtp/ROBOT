# Programme Triangle - Guide d'Utilisation

## Description
Le robot parcourt automatiquement un triangle équilatéral et répète indéfiniment la figure.

## Fichier
- **Fichier source** : [triangle.c](app_sofware/triangle.c)

## Calibration Nécessaire

Le programme utilise des **délais temporels** pour contrôler les distances et les angles. Ces valeurs doivent être **calibrées** selon votre robot :

### Paramètres à ajuster dans le code :

```c
#define SPEED_FWD      0x900     // Vitesse pour avancer (ajuster si nécessaire)
#define SPEED_TURN     0x800     // Vitesse pour tourner (ajuster si nécessaire)
#define DELAY_FORWARD  3000000   // Durée pour un côté du triangle
#define DELAY_TURN     1000000   // Durée pour tourner 120°
```

## Procédure de Calibration

### Étape 1 : Calibrer la rotation (120°)

1. Marquer la position et l'orientation initiale du robot au sol
2. Compiler et télécharger le programme
3. Observer la première rotation après le premier côté
4. Le robot doit tourner d'environ **120 degrés** (un tiers de tour)

**Ajustements :**
- Si le robot tourne **trop** : **diminuer** `DELAY_TURN`
- Si le robot tourne **pas assez** : **augmenter** `DELAY_TURN`

**Exemple :**
```c
// Rotation trop courte (90° au lieu de 120°)
#define DELAY_TURN     1300000   // Augmenter de 30%

// Rotation trop longue (150° au lieu de 120°)  
#define DELAY_TURN     800000    // Diminuer de 20%
```

### Étape 2 : Calibrer la longueur des côtés

Les trois côtés doivent avoir la **même longueur** pour former un triangle équilatéral.

**Ajustements :**
- Si les côtés sont **trop courts** : **augmenter** `DELAY_FORWARD`
- Si les côtés sont **trop longs** : **diminuer** `DELAY_FORWARD`

**Exemple :**
```c
// Pour des côtés de 50 cm environ
#define DELAY_FORWARD  2000000

// Pour des côtés de 1 mètre environ
#define DELAY_FORWARD  4000000
```

### Étape 3 : Vérifier la trajectoire complète

Après calibration, le robot devrait :
1. Partir d'un point A
2. Former un triangle
3. Revenir **approximativement** au point A
4. Répéter le motif

**Note :** Une petite dérive est normale (accumulation d'erreurs). Si la dérive est importante :
- Vérifier l'alignement des roues
- Utiliser `motor_characterization.c` pour équilibrer les vitesses gauche/droite
- Ajuster `SPEED_FWD` séparément pour chaque moteur (voir code avancé ci-dessous)

## Indicateurs LED

Pendant l'exécution :

| LED(s) allumée(s) | Signification |
|-------------------|---------------|
| LED0 (0x01) | Robot avance |
| LED3 (0x08) | Robot tourne à gauche |
| Toutes éteintes | Pause entre mouvements |
| Toutes allumées (0xFF) | Triangle terminé, pause avant recommencer |

## Code Avancé : Ajustement Différentiel

Si votre robot dévie pendant la ligne droite, utilisez cette version modifiée :

```c
// Dans les #define
#define SPEED_FWD_R  0x920   // Vitesse moteur droit (ajuster indépendamment)
#define SPEED_FWD_L  0x8E0   // Vitesse moteur gauche (ajuster indépendamment)

// Dans la fonction move_forward()
void move_forward() {
    IOWR(MOTOR_RIGHT_BASE, GO_BIT | SPEED_FWD_R);
    IOWR(MOTOR_LEFT_BASE, GO_BIT | SPEED_FWD_L);
    IOWR(LEDS_BASE, 0x01);
    delay(DELAY_FORWARD);
}
```

## Dépannage

### Le robot ne forme pas un triangle fermé
- **Cause** : Angles de rotation incorrects ou vitesses déséquilibrées
- **Solution** : Recalibrer `DELAY_TURN` et vérifier les vitesses des moteurs

### Le robot dévie pendant les lignes droites
- **Cause** : Différence de vitesse entre moteurs gauche et droit
- **Solution** : Utiliser `motor_characterization.c` (mode calibration) pour égaliser les vitesses

### Les côtés ne sont pas égaux
- **Cause** : Usure des batteries ou friction inégale
- **Solution** : Mesurer la distance parcourue en X secondes et ajuster `DELAY_FORWARD`

### Le triangle s'agrandit ou se déforme au fil des répétitions
- **Cause** : Accumulation d'erreurs (normal sans capteurs)
- **Solution** : C'est une limite du contrôle en boucle ouverte. Pour une précision optimale, il faudrait des capteurs (encodeurs roues, gyroscope, etc.)

## Améliorations Possibles

1. **Variation de taille** : Modifier `DELAY_FORWARD` pour des triangles plus grands/petits
2. **Autres formes** : 
   - Carré : 4 avancées + rotations de 90° (`DELAY_TURN / 1.33`)
   - Hexagone : 6 avancées + rotations de 60° (`DELAY_TURN / 2`)
3. **Vitesse variable** : Augmenter/diminuer `SPEED_FWD` et `SPEED_TURN`
4. **Sens inverse** : Tourner à droite au lieu de tourner à gauche

## Exemple de Valeurs Calibrées

Voici des valeurs de départ typiques (robot DE0-Nano avec moteurs standards) :

```c
#define SPEED_FWD      0x900      // 2304
#define SPEED_TURN     0x800      // 2048
#define DELAY_FORWARD  2500000    // ~2.5 sec → côté de ~40-50cm
#define DELAY_TURN     850000     // ~0.85 sec → rotation 120°
```

**Important :** Ces valeurs sont données à titre indicatif et doivent être ajustées pour votre robot spécifique !

## Compilation

1. Dans le **Monitor Program** : File → Change Compiled File
2. Sélectionner `triangle.c`
3. Compiler et télécharger
4. Observer le robot et ajuster les paramètres si nécessaire
