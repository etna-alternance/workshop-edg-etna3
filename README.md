# SDC-COMU / Atelier EDG / Programmer une bande LED connectée avec un ESP32

# Objectifs

- Câbler et programmer un ESP32 pour contrôler une bande LED adressable (WS2812B) et des boutons physiques.
- Développer un moteur de jeu "bare-metal" en C++ : gérer le temps réel (non-bloquant avec `millis()`), les collisions et les états.
- Connecter le jeu au réseau WiFi local pour écouter des événements externes via MQTT (Cloud to Edge).
- Découvrir les contraintes de l'embarqué (limites de courant, rebonds matériels) et les métiers du Edge Computing.

# Contexte

Créer un jeu vidéo sur un PC avec Unity ou Unreal Engine est une chose. Le créer sur un microcontrôleur à 3€ avec 520 Ko de RAM en est une autre. Ici, pas de système d'exploitation pour gérer le matériel à ta place, pas de multithreading magique, et chaque appel bloquant fige l'intégralité du système.

Aujourd'hui, tu vas construire un jeu d'arcade unidimensionnel. Une "base" à défendre, des pixels ennemis qui tombent, et un canon contrôlé par 4 boutons pour tirer des pixels de la bonne couleur.

Ce projet miniaturise les problématiques de l'ingénierie embarquée et de la robotique : acquérir des signaux matériels (boutons), traiter la logique en temps réel sans jamais bloquer le processeur, restituer un état visuel complexe (LED adressables), et communiquer avec le cloud.

# Consignes

## Kit matériel fourni (1 kit par binôme)

| Composant | Quantité | Rôle |
| - | - | - |
| ESP32 (DevKit) | 1 | Microcontrôleur WiFi — cerveau du jeu |
| Bande LED WS2812E | 1 (~140 pixels) | L'écran du jeu (LEDs adressables intelligentes) |
| Boutons poussoirs | 3 | Contrôleurs (Tir Rouge, Tir Vert, Tir Bleu) |
| Résistance 330Ω-470Ω| 1 | Protection du signal Data de la bande LED |
| Câble USB-C ou Micro | 1 | Programmation et alimentation |
| Breadboard & Fils | 1 set | Connexions |

## Phase 1 — Câblage et FastLED (45min)

**1. Le Câblage :**
Contrairement à une simple LED RGB, la bande WS2812B n'utilise qu'une seule broche de données (DIN). Chaque LED possède sa propre puce et transmet l'information à la suivante.

* **Bande LED :** 5V sur `VIN` (ou 5V), GND sur `GND`. Branche la résistance en série entre la broche `GPIO 16` et le fil `DIN` de la bande.
* **Boutons :** Câble une patte de chaque bouton sur le `GND`. L'autre patte en diagonale sur les broches : `25` (Vert), `26` (Bleu), `27` (Rouge) et `23` (Level Up). Pas besoin de résistances matérielles, nous utiliserons `INPUT_PULLUP`.

**2. Initialiser FastLED :**
Installe la bibliothèque `FastLED` via le gestionnaire de librairie Arduino.
Écris ton sketch de base pour allumer le pixel 0 (ta base) :

```cpp
#include <FastLED.h>

#define NUM_LEDS 140
#define LED_PIN 16
#define GREEN_BTN 25
#define RED_BTN 26
#define BLUE_BTN 27
#define LEVEL_UP_BTN 23

CRGB leds[NUM_LEDS];       // Tableau représentant l'écran

void setup() {
  // Initialisation avec sécurité de courant (5V, 500mA max)
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();

  leds[0] = CRGB(20, 20, 20); // Base allumée en blanc faible
  FastLED.show();
}

void loop()
{
    // Allumer la première led !
}
```
*Livrable intermédiaire : Ta base (pixel 0) est allumée.*

## Phase 2 — Le Moteur Asynchrone et les Inputs (45min)

Dans un jeu vidéo, on ne peut pas utiliser `delay()`. Si tu fais un `delay(100)` pour faire avancer un ennemi, l'ESP32 est aveugle aux boutons du joueur pendant 100ms.

**1. Lire les boutons avec anti-rebond (Debounce) :**
Déclare tes boutons en `INPUT_PULLUP`. Crée une fonction `handleButtons()` appelée dans le `loop()`. Utilise la fonction `millis()` (le chronomètre interne de l'ESP32) pour ignorer les rebonds physiques du métal pendant 50ms.

```cpp
// Exemple de logique non-bloquante pour un tir
if (millis() - lastDebounceTime > 50) {
    if (digitalRead(RED_BTN) == LOW && lastRedState == HIGH) {
      shoot(CRGB::Red); // Fonction à créer
      lastDebounceTime = millis();
    }
}
lastRedState = digitalRead(RED_BTN);
```

**2. Timers de mouvement :**
Dans le `loop()`, mets en place deux chronomètres `millis()` séparés : un pour faire avancer les tirs (très rapide), un pour faire tomber les ennemis (plus lent, calculé avec `1000 / (currentLevel + 1)`).

*Livrable intermédiaire : Le moniteur série affiche "Tir Rouge" ou "Tir Vert" instantanément quand tu presses un bouton, sans aucun bloquage.*

## Phase 3 — Logique de Jeu et Collisions (75min)

C'est ici que l'ingénierie logicielle opère.

**1. Les structures de données :**
Crée des `struct` pour modéliser tes entités (Position, Couleur, Statut Actif). Déclare deux tableaux de taille 40 : `enemies[40]` et `shots[40]`.

```cpp
struct FallingPixel { int pos; CRGB color; bool active; };
```

**2. Les fonctions principales :**
Développe les mécaniques du jeu :
* `spawnEnemies()` : Fait apparaître un pixel en haut (`NUM_LEDS - 1`) avec une couleur aléatoire (Rouge, Vert, Bleu).
* `moveShots()` et `moveEnemies()` : Modifient les positions. Si un ennemi atteint `pos <= 0`, c'est le Game Over (flash rouge complet et `resetGame()`).
* `checkCollisions()` : Parcourt les deux tableaux. Si `ennemi.pos == shot.pos` :
    * Même couleur ? Les deux meurent (`active = false`), le score augmente.
    * Mauvaise couleur ? Le tir meurt, l'ennemi change de couleur aléatoirement et continue de tomber.

*Livrable intermédiaire : Le jeu est 100% jouable hors-ligne.*

*Demander le code sans bonus au intervenant*

## Phase 4 — Bonus : Mode Multijoueur Asynchrone via MQTT (25min)

L'ESP32 possède le WiFi. Nous allons le connecter pour que le formateur (ou un autre groupe) puisse saboter ta partie ou t'aider depuis un Dashboard Node-RED.

Intègre les bibliothèques `WiFi.h` et `PubSubClient`.
Abonne-toi au topic `etna/arcade/groupeX/events`. (Met quelques choses de différents)

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "ETNA25 - Students";
const char *password = "etnawifi";
const char *mqtt_server = "10.1.164.174";
const char *mqtt_topic = "etna/edg/group1/events"; // Mettez autres choses pour éviter des problèmes de conflits entre les topics

WiFiClient espClient;
PubSubClient client(espClient);
```

Dans ton callback MQTT, réagis aux commandes externes :
```cpp
void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (msg == "SPAWN_BOSS") {
    // Fais spawner un ennemi qui descend 3x plus vite
  }
  if (msg == "LEVEL_UP") {
    levelUp(); // Déclenche l'animation de passage de niveau
  }
  if (msg == "SHOOT_RED"){

  }
}
```

*Livrable final : Le jeu tourne, et réagit en temps réel aux commandes réseau envoyées par le formateur.*

## Avant l'atelier

Ajouter le lien de téléchargment dans Preferences -> Additional boards manager URLs -> http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json

Dans le Board Manager installer "esp32" de Expressif Systems, puis sélectionner le board "DOIT ESP32 DEVKIT V1"

**En présentiel :** Installer Arduino IDE 2.x + support ESP32. Installer les bibliothèques `FastLED` et `PubSubClient` via le gestionnaire.
**À distance :** Créer un compte sur Wokwi (simulateur avancé gérant l'ESP32, FastLED et le WiFi). Tinkercad ne supportant pas le WiFi de l'ESP32, Wokwi est obligatoire.

## Pendant l'atelier

- Si la bande clignote de toutes les couleurs de manière erratique : revérifie ton fil de masse (GND). C'est le problème #1.
- Gère la limite de puissance de FastLED dès la phase 1. Un crash "Hard Resetting" à chaque `levelUp()` signifie que tu tires trop de courant sur l'USB.
- Gère bien l'anti-rebond matériel des boutons, sinon tu vas générer du "Bullet Stacking" (tirer 3 balles empilées sur le même pixel en une seule pression).

# Livrables

**`game.ino`** — Le code C++ complet (incluant FastLED, le moteur asynchrone, et la connexion MQTT).
**Capture vidéo** — Une courte démo de ton binôme en train de défendre sa base, avec l'intervenant qui déclenche un "Level Up" via MQTT.

# Conseils

- Sépare ton affichage de ta logique. Toutes tes fonctions modifient l'état des tableaux en mémoire. La bande LED n'est mise à jour qu'une seule fois à la fin de ta boucle `loop()` avec `FastLED.show()`.
- Utilise la documentation de FastLED pour les couleurs (`CRGB::Red`, `CRGB::Blue`, etc.).

## Ressources

- Arduino IDE : https://www.arduino.cc/en/software
- Documentation FastLED : https://github.com/FastLED/FastLED/wiki
- PubSubClient (MQTT) : https://pubsubclient.knolleary.net/
- Wokwi ESP32 Simulator : https://wokwi.com/
