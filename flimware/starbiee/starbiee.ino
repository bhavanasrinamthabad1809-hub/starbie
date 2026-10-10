#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <Preferences.h>
#include <math.h>

// Pins (ESP32-C3 GPIO numbers)
const int I2C_SDA_PIN = 7;   // D5
const int I2C_SCL_PIN = 6;   // D4
const int DHT_PIN = 3;       // D1
const int BTN1_PIN = 4;      // D2 menu
const int BTN2_PIN = 5;      // D3 stats
const int BTN3_PIN = 21;     // D6 happy

const uint8_t OLED_ADDR = 0x3C;
const uint8_t MPU_ADDR = 0x68;

const int SCREEN_W = 128;
const int SCREEN_H = 64;
const int SPRITE_SIZE = 32;
const float GRAVITY = 9.80665f;

// Tuning
const float TILT_LIMIT = 6.0f;
const float DEADZONE = 0.8f;
const float SHAKE_LIMIT = 7.0f;
const float X_DIR = 1.0f;
const float Y_DIR = -1.0f;
const uint32_t NAP_MS = 48000;
const uint32_t HAPPY_MS = 2500;
const uint32_t HOP_MS = 400;
const int HOP_HEIGHT = 8;

const int PET_SPRITE_WIDTH = 32;
const int PET_SPRITE_HEIGHT = 32;

const uint8_t PROGMEM PET_SPRITE[] = {
  0x00, 0x00, 0xc0, 0x00,
  0x00, 0x07, 0xee, 0x00,
  0x00, 0x1f, 0xff, 0x00,
  0x00, 0x7f, 0xfe, 0x00,
  0x00, 0x3f, 0xff, 0x80,
  0x00, 0x7f, 0xff, 0x80,
  0x00, 0x7f, 0xff, 0xc0,
  0x00, 0xff, 0xff, 0xc0,
  0x00, 0xff, 0xff, 0xc0,
  0x01, 0xff, 0xff, 0xc0,
  0x00, 0xff, 0xff, 0xc0,
  0x00, 0xff, 0xff, 0xc0,
  0x00, 0xff, 0xff, 0xc0,
  0x00, 0x7d, 0xdf, 0xc0,
  0x00, 0xbd, 0xdf, 0x80,
  0x01, 0xdf, 0xff, 0x00,
  0x03, 0xff, 0xfe, 0x80,
  0x03, 0xff, 0xf3, 0x80,
  0x03, 0x07, 0xff, 0xc0,
  0x00, 0x07, 0xc3, 0xc0,
  0x00, 0x07, 0xc1, 0xc0,
  0x00, 0x07, 0xc0, 0x00,
  0x00, 0x07, 0xe0, 0x00,
  0x00, 0x07, 0xc0, 0x00,
  0x00, 0x0f, 0xe0, 0x00,
  0x01, 0xcf, 0xf0, 0x00,
  0x03, 0xc7, 0x9c, 0x00,
  0x03, 0xff, 0x9e, 0x00,
  0x03, 0xff, 0xfe, 0x00,
  0x01, 0xfc, 0xfe, 0x00,
  0x00, 0xf8, 0xfc, 0x00,
  0x00, 0x00, 0x78, 0x00,
};

enum Reaction { R_NAP, R_JUMP, R_HEARTS, R_RUN };
struct MenuItem { const char *label; int joy; int energy; int full; Reaction reaction; };

const MenuItem MENU[] = {
  {"NAP",   1,  18, -4, R_NAP},
  {"PLAY", 12,  -9, -5, R_RUN},
  {"FEED",  3,   2, 18, R_JUMP},
  {"PET",   7,   0,  0, R_HEARTS},
};
const int MENU_X[] = {64, 108, 64, 20};
const int MENU_Y[] = {12, 32, 53, 32};

enum View { VIEW_PET, VIEW_MENU, VIEW_STATS };
struct Button { int pin; bool stable; bool lastRaw; uint32_t changedAt; };

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
Adafruit_MPU6050 mpu;
DHT dht(DHT_PIN, DHT11);
Preferences prefs;

Button btn1 = {BTN1_PIN, HIGH, HIGH, 0};
Button btn2 = {BTN2_PIN, HIGH, HIGH, 0};
Button btn3 = {BTN3_PIN, HIGH, HIGH, 0};
View view = VIEW_PET;

int joy = 70, energy = 75, fullness = 65;
bool mpuOk = false;
float ax = 0, ay = 0, az = GRAVITY;
float tempC = NAN, hum = NAN;
float ballX = 64, ballY = 32, centerAx = 0, centerAy = 0;
int selected = -1;
int napX = 0;

uint32_t lastMpu = 0, lastDht = 0, lastShake = 0;
uint32_t shakeUntil = 0, jumpStart = 0, napStart = 0;
uint32_t heartsStart = 0, heartsDur = 0, runStart = 0, happyStart = 0;

bool active(uint32_t start, uint32_t duration) {
  return start != 0 && millis() - start < duration;
}
bool napping() { return active(napStart, NAP_MS); }
bool running() { return active(runStart, 1600); }
bool happy() { return active(happyStart, HAPPY_MS); }

void changeStats(int j, int e, int f) {
  joy = constrain(joy + j, 0, 100);
  energy = constrain(energy + e, 0, 100);
  fullness = constrain(fullness + f, 0, 100);
}

void loadStats() {
  prefs.begin("starbie", false);
  joy = prefs.getInt("joy", joy);
  energy = prefs.getInt("energy", energy);
  fullness = prefs.getInt("full", fullness);
}

void saveStats() {
  prefs.putInt("joy", joy);
  prefs.putInt("energy", energy);
  prefs.putInt("full", fullness);
}

void initButton(Button &b) {
  pinMode(b.pin, INPUT_PULLUP);
  b.stable = digitalRead(b.pin);
  b.lastRaw = b.stable;
  b.changedAt = millis();
}

bool pressed(Button &b) {
  bool raw = digitalRead(b.pin);
  uint32_t now = millis();
  if (raw != b.lastRaw) {
    b.lastRaw = raw;
    b.changedAt = now;
  }
  if (raw != b.stable && now - b.changedAt >= 30) {
    b.stable = raw;
    return raw == LOW;
  }
  return false;
}

void readMpu() {
  if (!mpuOk || millis() - lastMpu < 30) return;
  lastMpu = millis();
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;
}

void readDht() {
  if (millis() - lastDht < 2200) return;
  lastDht = millis();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) hum = h;
  if (!isnan(t)) tempC = t;
}

int walkX(uint32_t now) {
  const int maxX = SCREEN_W - SPRITE_SIZE;
  int step = (now / 70) % (maxX * 2);
  return step <= maxX ? step : maxX * 2 - step;
}

int runX(uint32_t now) {
  const int maxX = SCREEN_W - SPRITE_SIZE;
  float p = ((now - runStart) % 800) / 800.0f;
  return p < 0.5f ? (int)(p * 2 * maxX) : (int)((1 - p) * 2 * maxX);
}

void updateTimers() {
  if (napStart != 0 && !napping()) napStart = 0;
  if (runStart != 0 && !running()) runStart = 0;
  if (happyStart != 0 && !happy()) happyStart = 0;
}

void updateBall() {
  float x = (ax - centerAx) * X_DIR;
  float y = (ay - centerAy) * Y_DIR;
  x = constrain(x, -TILT_LIMIT, TILT_LIMIT);
  y = constrain(y, -TILT_LIMIT, TILT_LIMIT);
  ballX += (64 + (x / TILT_LIMIT) * 37 - ballX) * 0.2f;
  ballY += (32 + (y / TILT_LIMIT) * 22 - ballY) * 0.2f;

  if (fabsf(x) < DEADZONE && fabsf(y) < DEADZONE) selected = -1;
  else if (fabsf(x) > fabsf(y)) selected = x > 0 ? 1 : 3;
  else selected = y > 0 ? 2 : 0;
}

void checkShake() {
  if (!mpuOk || view != VIEW_PET) return;
  float mag = sqrtf(ax * ax + ay * ay + az * az);
  uint32_t now = millis();
  if (fabsf(mag - GRAVITY) >= SHAKE_LIMIT && now - lastShake >= 650) {
    lastShake = now;
    napStart = 0;
    shakeUntil = now + 350;
    changeStats(5, -2, -1);
    saveStats();
  }
}

void openMenu() {
  view = VIEW_MENU;
  selected = -1;
  ballX = 64;
  ballY = 32;
  centerAx = ax;
  centerAy = ay;
}

void chooseItem() {
  if (selected == -1) return;
  const MenuItem &item = MENU[selected];
  changeStats(item.joy, item.energy, item.full);
  saveStats();

  uint32_t now = millis();
  napStart = 0;
  heartsStart = 0;
  runStart = 0;
  happyStart = 0;
  jumpStart = 0;

  if (item.reaction == R_NAP) {
    napX = walkX(now);
    napStart = now;
  } else if (item.reaction == R_RUN) {
    runStart = now;
    heartsStart = now;
    heartsDur = 1600;
  } else {
    jumpStart = now;
    if (item.reaction == R_HEARTS) {
      heartsStart = now;
      heartsDur = 1600;
    }
  }
  view = VIEW_PET;
}

void startHappy() {
  napStart = 0;
  jumpStart = 0;
  runStart = 0;
  heartsStart = 0;
  happyStart = millis();
  changeStats(10, 0, 0);
  saveStats();
  view = VIEW_PET;
}

void handleButtons() {
  if (pressed(btn1)) {
    if (view == VIEW_MENU) chooseItem();
    else openMenu();
  }
  if (pressed(btn2)) {
    view = (view == VIEW_STATS) ? VIEW_PET : VIEW_STATS;
  }
  if (pressed(btn3) && view != VIEW_MENU) {
    startHappy();
  }
}

void drawHeart(int x, int y) {
  display.fillRect(x - 2, y, 2, 2, SSD1306_WHITE);
  display.fillRect(x + 1, y, 2, 2, SSD1306_WHITE);
  display.fillRect(x - 3, y + 2, 7, 2, SSD1306_WHITE);
  display.fillRect(x - 2, y + 4, 5, 1, SSD1306_WHITE);
  display.fillRect(x - 1, y + 5, 3, 1, SSD1306_WHITE);
  display.drawPixel(x, y + 6, SSD1306_WHITE);
}

void drawSparkle(int x, int y) {
  display.drawPixel(x, y - 2, SSD1306_WHITE);
  display.drawPixel(x - 1, y - 1, SSD1306_WHITE);
  display.drawPixel(x + 1, y - 1, SSD1306_WHITE);
  display.drawPixel(x - 2, y, SSD1306_WHITE);
  display.drawPixel(x, y, SSD1306_WHITE);
  display.drawPixel(x + 2, y, SSD1306_WHITE);
  display.drawPixel(x - 1, y + 1, SSD1306_WHITE);
  display.drawPixel(x + 1, y + 1, SSD1306_WHITE);
  display.drawPixel(x, y + 2, SSD1306_WHITE);
}

void drawPet() {
  display.clearDisplay();
  uint32_t now = millis();
  int x = napping() ? napX : walkX(now);
  if (running()) x = runX(now);
  int y = SCREEN_H - SPRITE_SIZE;

  if (now < shakeUntil) x += (int)(sinf(now / 18.0f) * 3.0f);

  if (!napping() && jumpStart != 0) {
    uint32_t age = now - jumpStart;
    if (age < 230) {
      x += (int)(sinf(now / 16.0f) * 3.0f);
    } else if (age < 660) {
      float p = (age - 230) / 430.0f;
      y -= (int)(sinf(p * PI) * 16);
    } else {
      jumpStart = 0;
    }
  }

  if (happy()) {
    float age = (float)(now - happyStart);
    y -= (int)(fabsf(sinf(age * PI / HOP_MS)) * HOP_HEIGHT);
  }

  x = constrain(x, 0, SCREEN_W - SPRITE_SIZE);
  display.drawBitmap(x, y, PET_SPRITE, SPRITE_SIZE, SPRITE_SIZE, SSD1306_WHITE);

  if (napping()) {
    int rise = (now / 300) % 15;
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x + 23, y - 2 - rise);
    display.print("z");
    display.setCursor(x + 28, y - 8 - rise / 2);
    display.print("z");
  }

  if (active(heartsStart, heartsDur)) {
    float p = (float)(now - heartsStart) / heartsDur;
    int rise = (int)(p * 18);
    drawHeart(x + 10, y - 3 - rise);
    drawHeart(x + 22, y - 9 - rise / 2);
  }

  if (happy()) {
    for (int i = 0; i < 3; i++) {
      float p = ((now + i * 500UL) % 1500UL) / 1500.0f;
      drawHeart(x + 6 + i * 9, y - 2 - (int)(p * 20));
    }
    if ((now / 200) % 2 == 0) {
      drawSparkle(x - 4, y + 10);
      drawSparkle(x + SPRITE_SIZE + 3, y + 16);
    } else {
      drawSparkle(x - 3, y + 22);
      drawSparkle(x + SPRITE_SIZE + 2, y + 6);
    }
  }
}

void drawMenu() {
  display.clearDisplay();
  display.drawLine(0, 0, SCREEN_W - 1, SCREEN_H - 1, SSD1306_WHITE);
  display.drawLine(0, SCREEN_H - 1, SCREEN_W - 1, 0, SSD1306_WHITE);

  for (int i = 0; i < 4; i++) {
    int bx = MENU_X[i] - 16;
    int by = MENU_Y[i] - 6;
    if (i == selected) {
      display.fillRoundRect(bx, by, 33, 12, 3, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.drawRoundRect(bx, by, 33, 12, 3, SSD1306_WHITE);
      display.setTextColor(SSD1306_WHITE);
    }
    display.setTextSize(1);
    display.setCursor(MENU_X[i] - strlen(MENU[i].label) * 3, MENU_Y[i] - 3);
    display.print(MENU[i].label);
  }

  display.setTextColor(SSD1306_WHITE);
  display.drawCircle(64, 32, 11, SSD1306_WHITE);
  display.fillCircle((int)ballX, (int)ballY, 4, SSD1306_WHITE);
}

void drawBar(int y, const char *label, int value) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, y);
  display.print(label);
  display.drawRect(48, y, 57, 8, SSD1306_WHITE);
  display.fillRect(49, y + 1, map(value, 0, 100, 0, 55), 6, SSD1306_WHITE);
  display.setCursor(109, y);
  display.print(value);
}

void drawStats() {
  display.clearDisplay();
  drawBar(5, "JOY", joy);
  drawBar(20, "ENERGY", energy);
  drawBar(35, "FULL", fullness);

  display.setCursor(4, 53);
  if (!isnan(tempC)) {
    display.print("TEMP ");
    display.print((int)tempC);
    display.print("C");
  } else {
    display.print("TEMP --");
  }

  display.setCursor(76, 53);
  if (!isnan(hum)) {
    display.print("H ");
    display.print((int)hum);
    display.print("%");
  } else {
    display.print("H --");
  }
}

void draw() {
  if (view == VIEW_MENU) drawMenu();
  else if (view == VIEW_STATS) drawStats();
  else drawPet();
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  initButton(btn1);
  initButton(btn2);
  initButton(btn3);
  loadStats();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found");
    while (true) delay(10);
  }

  mpuOk = mpu.begin(MPU_ADDR, &Wire);
  if (mpuOk) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  } else {
    Serial.println("MPU6050 not found");
  }

  dht.begin();
  draw();
}

void loop() {
  readMpu();
  readDht();
  updateTimers();
  handleButtons();

  if (view == VIEW_MENU) updateBall();
  else checkShake();

  draw();
  delay(16);
}