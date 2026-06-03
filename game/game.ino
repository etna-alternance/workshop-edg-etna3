#include <FastLED.h>

#define NUM_LEDS 140
#define LED_PIN 16
#define GREEN_BTN 25
#define RED_BTN 26
#define BLUE_BTN 27
#define LEVEL_UP_BTN 23

CRGB leds[NUM_LEDS];
CRGB colors[3] = {CRGB::Red, CRGB::Blue, CRGB::Green};

int currentLevel = 1;
unsigned long lastShotMoveTime = 0;
unsigned long lastEnemyMoveTime = 0;

// Variables pour la gestion du debounce des boutons (éviter les lectures multiples lors d'une seule pression)
bool lastRedState = HIGH;
bool lastBlueState = HIGH;
bool lastGreenState = HIGH;
bool lastLevelUpState = HIGH;
const int debounceDelay = 50;
unsigned long lastDebounceTime = 0;

struct FallingPixel {
  int pos;
  CRGB color;
  bool active;
};

struct ShotPixel {
  int pos;
  CRGB color;
  bool active;
};

// Limite le nombre d'ennemis et de tirs actifs pour éviter les problèmes de performance
const int MAX_OBJECTS = 40;
ShotPixel shots[MAX_OBJECTS];
FallingPixel enemies[MAX_OBJECTS];

int nextSpawnDistance = 0;
int spawnDistanceCounter = 0;

// Initialisation du jeu
void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  pinMode(GREEN_BTN, INPUT_PULLUP);
  pinMode(RED_BTN, INPUT_PULLUP);
  pinMode(BLUE_BTN, INPUT_PULLUP);
  pinMode(LEVEL_UP_BTN, INPUT_PULLUP);

  resetGame();
}

// Boucle principale du jeu
void loop() {
  // On va éviter de griller le microcontrôleur ou le LED strip
  // C'est pourquoi je vous donne une base de code pour gérer les timings et les boutons, mais vous êtes libres de faire autrement si vous préférez
  handleButtons();

  int enemySpeedDelay = 1000 / (currentLevel + 1);
  int shotSpeedDelay = enemySpeedDelay / 2;

  bool needsDraw = false;

  if (millis() - lastShotMoveTime >= shotSpeedDelay) {
    moveShots();
    checkCollisions();
    lastShotMoveTime = millis();
    needsDraw = true;
  }

  if (millis() - lastEnemyMoveTime >= enemySpeedDelay) {
    moveEnemies();
    checkCollisions();
    spawnEnemies();
    lastEnemyMoveTime = millis();
    needsDraw = true;
  }

  if (needsDraw) {
    drawGame();
  }
}

void handleButtons() {
    // Handle button presses with debounce logic
}

void shoot(CRGB color) {
    // Shoot logic (activate a shot with the given color at position 1)
}

void spawnEnemies() {
    // Spawn enemies logic (take care of spawn distance and level)
}

void moveEnemies() {
    // Move enemies logic
}

void moveShots() {
    // Move shots logic
}

void checkCollisions() {
    // Check for collisions between shots and enemies
}

void levelUp() {
    // Level up logic if wanted
}

void resetGame() {
    // Reset game state
}

void drawGame() {
    // Draw game
}