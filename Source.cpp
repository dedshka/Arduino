#include <LiquidCrystal.h>
#include <avr/eeprom.h>

#define PLAYER1 0
#define PLAYER2 1
#define CACTUS 2
#define SCORE_ADDRES 0
#define BUTTON_PIN 3

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int topScore = 0;

byte player1[8] = {
  B01110,
  B01100,
  B01110,
  B01100,
  B01100,
  B01100,
  B01010,
  B01010
};

byte player2[8] = {
  B00111,
  B00110,
  B00111,
  B00110,
  B00110,
  B00110,
  B00101,
  B01001
};

byte obstacle[8] = {
  B00000,
  B00000,
  B00010,
  B00010,
  B01011,
  B01110,
  B00100,
  B00100
};

void drawMain() {
    lcd.setCursor(0, 0);
    lcd.print("PRESS BUTTON");
    lcd.setCursor(0, 1);
    lcd.print("TopScore:");
    lcd.print(topScore);
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    topScore = eeprom_read_word(SCORE_ADDRES);
    lcd.begin(16, 2);
    lcd.createChar(0, player1);
    lcd.createChar(1, player2);
    lcd.createChar(2, obstacle);
    lcd.setCursor(0, 1);

    drawMain();
}

void scoreCkeckAndUpdate(int curScore) {
    if (curScore > topScore) {
        eeprom_update_word(SCORE_ADDRES, curScore);
        topScore = curScore;
    }
}

int gdelay = 240;
byte jumpIt = 0;
byte cactusCount = 3;

int cactuses[3] = { 7,11,14 };

bool drawCactuses() {
    bool res = false;
    if (gdelay < 100) cactusCount = 2;
    //bubble sort
    for (int i = 0; i < cactusCount; ++i)
        for (int j = i; j < cactusCount; ++j) {
            int tmp = cactuses[i];
            cactuses[i] = cactuses[j];
            cactuses[j] = tmp;
        };
    //draw
    for (int i = 0; i < cactusCount; ++i) {
        lcd.setCursor(cactuses[i], 1);
        lcd.write(byte(CACTUS));
        lcd.print(' ');
        if (cactuses[i] == 0)
            res = true;
        --cactuses[i];
        if (cactuses[i] < 0)
            cactuses[i] = 16 + millis() % 9;//~random position
    }
    //removing gaps of size 1
    if (cactuses[0] - cactuses[1] == -2)
        cactuses[0] -= 2;
    if (cactuses[1] - cactuses[2] == -2)
        ++cactuses[2];
    return res;
}

void drawPlayer() {
    static byte playerType = PLAYER1;
    int buttonState = digitalRead(BUTTON_PIN);
    if (jumpIt == 0 && buttonState == LOW)
        jumpIt = 4;
    if (jumpIt != 0) {
        --jumpIt;
        lcd.setCursor(0, 1);
        lcd.print(' ');
        lcd.setCursor(0, 0);
    }
    else {
        lcd.setCursor(0, 0);
        lcd.print(' ');
        lcd.setCursor(0, 1);
    }
    lcd.write(byte(playerType));
    ++playerType;
    playerType %= 2;
}

void gameOverScreen(int score) {
    for (int i = 0; i < 3; ++i)
        cactuses[i] = -1;
    lcd.clear();
    lcd.print("Game Over");
    lcd.setCursor(0, 1);
    lcd.print("You Scored:");
    lcd.print(score);
    int buttonState = digitalRead(BUTTON_PIN);
    while (buttonState == HIGH) {
        buttonState = digitalRead(BUTTON_PIN);
        delay(50);
    }
    lcd.clear();
    drawMain();
    delay(800);
}

void gameLoop() {
    bool gameOver = false;
    int startTime = millis();
    int score = 0;
    int boostCount = 0;
    do {
        drawPlayer();
        bool cactusCame = drawCactuses();
        if (cactusCame && jumpIt == 0) gameOver = true;
        //score
        score = (millis() - startTime) / 50;
        //speedUp
        if (gdelay > 90 && score / 250 > boostCount) {
            ++boostCount;
            gdelay -= 30;
        }
        if (gdelay <= 90)
            cactusCount = 2;
        lcd.setCursor(10, 0);
        lcd.print(score);
        delay(gdelay);
    } while (!gameOver);
    scoreCkeckAndUpdate(score);
    gameOverScreen(score);
}

void loop() {
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW) {
        lcd.clear();
        delay(500);
        gameLoop();
    }
}