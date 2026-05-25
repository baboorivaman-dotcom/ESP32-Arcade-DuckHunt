/*
=========================================================
      DUCK HUNT GAME USING ESP32 DEVKIT V1
=========================================================

This project is a small duck hunt style game made using:
- ESP32 DevKit V1
- OLED display
- Joystick module
- LEDs and buzzer

Game features:
- smooth aim movement using joystick
- easy, medium and hard levels
- multiple ducks
- hit and miss indication using leds
- game over after 3 misses
- separate highest score for each level
=========================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// oled screen size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// creating oled object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// oled i2c address
#define OLED_ADDR 0x3C

// joystick pins connected to esp32
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32

// output pins
#define BUZZER 25
#define GREEN_LED 26
#define RED_LED 27

// crosshair starting position
int crossX = 64;
int crossY = 32;

// arrays used for multiple ducks
int duckX[3];
int duckY[3];
bool duckDir[3];

// game variables
int duckCount = 1;
int score = 0;

// variables for menu and speed
bool gameStarted = false;
int selectedLevel = 1;
int speedDelay = 60;

// counts number of misses
int misses = 0;

// highest scores for each difficulty level
int easyHigh = 0;
int mediumHigh = 0;
int hardHigh = 0;

// setup function runs once
void setup() {

  // joystick button input
  pinMode(JOY_SW, INPUT_PULLUP);

  // outputs
  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.begin(115200);

  // starting oled display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  display.clearDisplay();
  display.setTextColor(WHITE);

  // random values for duck movement
  randomSeed(millis());

  // assigning random starting positions for ducks
  for (int i = 0; i < 3; i++) {

    duckX[i] = random(10, 118);
    duckY[i] = random(15, 55);

    duckDir[i] = true;
  }
}

// function to display menu screen
void showMenu() {

  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(15, 0);
  display.println("DUCK");

  display.setCursor(15, 18);
  display.println("HUNT");

  display.setTextSize(1);

  // showing difficulty options
  display.setCursor(10, 45);
  display.print(selectedLevel == 1 ? "> EASY" : "  EASY");

  display.setCursor(10, 55);
  display.print(selectedLevel == 2 ? "> MEDIUM" : "  MEDIUM");

  display.setCursor(75, 55);
  display.print(selectedLevel == 3 ? "> HARD" : "  HARD");

  display.display();
}

// function used to select level using joystick
void handleMenu() {

  int yVal = analogRead(JOY_Y);

  // moving through menu options
  if (yVal < 1000) {
    selectedLevel--;
    delay(180);
  }

  if (yVal > 3000) {
    selectedLevel++;
    delay(180);
  }

  selectedLevel = constrain(selectedLevel, 1, 3);

  // pressing joystick button starts game
  if (digitalRead(JOY_SW) == LOW) {

    gameStarted = true;

    score = 0;
    misses = 0;

    // setting duck count and speed for each level
    if (selectedLevel == 1) {
      duckCount = 1;
      speedDelay = 60;
    }

    if (selectedLevel == 2) {
      duckCount = 2;
      speedDelay = 45;
    }

    if (selectedLevel == 3) {
      duckCount = 3;
      speedDelay = 30;
    }

    delay(300);
  }
}

// function to draw aiming crosshair
void drawCrosshair() {

  display.drawCircle(crossX, crossY, 5, WHITE);

  display.drawLine(crossX - 7, crossY,
                   crossX + 7, crossY, WHITE);

  display.drawLine(crossX, crossY - 7,
                   crossX, crossY + 7, WHITE);
}

// function to draw duck on oled screen
void drawDuck(int x, int y) {

  // duck body
  display.fillCircle(x, y, 4, WHITE);

  // duck head
  display.fillCircle(x + 5, y - 3, 2, WHITE);

  // duck beak
  display.drawPixel(x + 8, y - 3, WHITE);

  // duck wing
  display.drawLine(x - 2, y,
                   x - 6, y - 3, WHITE);

  // duck tail
  display.drawPixel(x - 5, y + 1, WHITE);

  // duck eye
  display.drawPixel(x + 5, y - 4, BLACK);
}

// function to move ducks across screen
void moveDucks() {

  for (int i = 0; i < duckCount; i++) {

    // moving ducks left and right
    if (duckDir[i]) {
      duckX[i] += 2;
    }
    else {
      duckX[i] -= 2;
    }

    // adding small random vertical movement
    duckY[i] += random(-1, 2);

    duckY[i] = constrain(duckY[i], 15, 55);

    // changing direction at screen edges
    if (duckX[i] >= 124)
      duckDir[i] = false;

    if (duckX[i] <= 4)
      duckDir[i] = true;
  }
}

// game over buzzer sound
void gameOverTheme() {

  tone(BUZZER, 700);
  delay(500);

  tone(BUZZER, 500);
  delay(500);

  tone(BUZZER, 300);
  delay(500);

  tone(BUZZER, 200);
  delay(700);

  noTone(BUZZER);
}

// function to display game over screen
void gameOver() {

  // saving highest score for each level
  if (selectedLevel == 1 && score > easyHigh)
    easyHigh = score;

  if (selectedLevel == 2 && score > mediumHigh)
    mediumHigh = score;

  if (selectedLevel == 3 && score > hardHigh)
    hardHigh = score;

  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(10, 10);
  display.println("GAME");

  display.setCursor(10, 30);
  display.println("OVER");

  // displaying final score
  display.setTextSize(1);

  display.setCursor(10, 55);
  display.print("Score:");
  display.print(score);

  display.display();

  // playing game over sound
  gameOverTheme();

  delay(2000);

  // returning back to menu
  gameStarted = false;
}

// function for shooting ducks
void shoot() {

  bool hit = false;

  // checking whether player hit the duck
  for (int i = 0; i < duckCount; i++) {

    if (abs(crossX - duckX[i]) < 8 &&
        abs(crossY - duckY[i]) < 8) {

      hit = true;

      score++;

      // reset misses after successful hit
      misses = 0;

      // assigning new duck position
      duckX[i] = random(10, 118);
      duckY[i] = random(15, 55);
    }
  }

  // if duck is hit
  if (hit) {

    digitalWrite(GREEN_LED, HIGH);

    // buzzer sound for hit
    tone(BUZZER, 1200);

    delay(100);

    noTone(BUZZER);

    digitalWrite(GREEN_LED, LOW);
  }

  // if player misses
  else {

    misses++;

    digitalWrite(RED_LED, HIGH);

    // miss sound
    tone(BUZZER, 300);

    delay(120);

    noTone(BUZZER);

    digitalWrite(RED_LED, LOW);

    // game over after 3 misses
    if (misses >= 3) {
      gameOver();
    }
  }
}

// main loop runs continuously
void loop() {

  // showing menu before game starts
  if (!gameStarted) {

    showMenu();

    // showing highest score of selected level
    display.setCursor(80, 45);

    if (selectedLevel == 1) {
      display.print("H:");
      display.print(easyHigh);
    }

    if (selectedLevel == 2) {
      display.print("H:");
      display.print(mediumHigh);
    }

    if (selectedLevel == 3) {
      display.print("H:");
      display.print(hardHigh);
    }

    display.display();

    handleMenu();

    return;
  }

  // reading joystick values
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  // joystick controls for aiming

  // left and right movement
  if (yVal < 1500)
    crossX += 4;

  if (yVal > 2500)
    crossX -= 4;

  // up and down movement
  if (xVal < 1500)
    crossY -= 4;

  if (xVal > 2500)
    crossY += 4;

  // limiting crosshair inside screen
  crossX = constrain(crossX, 5, 123);
  crossY = constrain(crossY, 10, 59);

  // shooting when joystick button is pressed
  if (digitalRead(JOY_SW) == LOW) {

    shoot();

    delay(150);
  }

  // moving ducks continuously
  moveDucks();

  // clearing display for next frame
  display.clearDisplay();

  // drawing all ducks
  for (int i = 0; i < duckCount; i++) {
    drawDuck(duckX[i], duckY[i]);
  }

  // drawing crosshair
  drawCrosshair();

  // showing current score
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("S:");
  display.print(score);

  // showing miss count as X marks
  display.setCursor(90, 0);

  for (int i = 0; i < misses; i++) {
    display.print("X");
  }

  // showing selected level
  display.setCursor(40, 0);

  if (selectedLevel == 1)
    display.print("Easy");

  if (selectedLevel == 2)
    display.print("Medium");

  if (selectedLevel == 3)
    display.print("Hard");

  display.display();

  delay(speedDelay);
}