#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ======================================================
// OLED CONFIG
// ======================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA_PIN 4
#define SCL_PIN 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ======================================================
// DS18B20 CONFIG
// ======================================================

#define ONE_WIRE_BUS 6

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ======================================================
// SENSOR PIN
// ======================================================

#define PH_PIN 15
#define TDS_PIN 3

#define TURBIDITY_PIN 10
#define FLOW_PIN 12
#define LUX_PIN 13

#define TRIG_PIN 17
#define ECHO_PIN 16

// ======================================================
// OUTPUT PIN
// ======================================================

// Relay Nutrisi
#define RELAY_TDS_A 7
#define RELAY_TDS_B 8

// Relay pH
#define RELAY_PH_UP 11
#define RELAY_PH_DOWN 9

// Grow Light LED
#define LED_LUX 36

// Buzzer
#define BUZZER_WATER 21
#define BUZZER_TURBIDITY 20
#define BUZZER_FLOW 19

// ======================================================
// VARIABLE
// ======================================================

float suhuAir;
float phValue;
float tdsValue;
float turbidityValue;
float flowValue;
float luxValue;
float distance;

// ======================================================
// TARGET PARAMETER
// ======================================================

int targetTDS = 900;

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // OLED
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal");
    while (true);
  }

  // DS18B20
  sensors.begin();

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Relay
  pinMode(RELAY_TDS_A, OUTPUT);
  pinMode(RELAY_TDS_B, OUTPUT);

  pinMode(RELAY_PH_UP, OUTPUT);
  pinMode(RELAY_PH_DOWN, OUTPUT);

  // LED
  pinMode(LED_LUX, OUTPUT);

  // Buzzer
  pinMode(BUZZER_WATER, OUTPUT);
  pinMode(BUZZER_TURBIDITY, OUTPUT);
  pinMode(BUZZER_FLOW, OUTPUT);

  // Default OFF
  digitalWrite(RELAY_TDS_A, LOW);
  digitalWrite(RELAY_TDS_B, LOW);

  digitalWrite(RELAY_PH_UP, LOW);
  digitalWrite(RELAY_PH_DOWN, LOW);

  digitalWrite(LED_LUX, LOW);

  noTone(BUZZER_WATER);
  noTone(BUZZER_TURBIDITY);
  noTone(BUZZER_FLOW);

  display.clearDisplay();
  display.display();
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  bacaSensor();

  kontrolTDS();

  kontrolPH();

  kontrolWaterLevel();

  kontrolTurbidity();

  kontrolFlow();

  kontrolGrowLight();

  tampilOLED();

  serialMonitor();

  delay(1000);
}

// ======================================================
// BACA SENSOR
// ======================================================

void bacaSensor() {

  // ================= SUHU AIR =================

  sensors.requestTemperatures();
  suhuAir = sensors.getTempCByIndex(0);

  // ================= pH =================

  int phRaw = analogRead(PH_PIN);

  phValue = map(phRaw, 0, 4095, 0, 140) / 10.0;

  // ================= TDS =================

  int tdsRaw = analogRead(TDS_PIN);

  tdsValue = map(tdsRaw, 0, 4095, 0, 2000);

  // ================= TURBIDITY =================

  int turbidityRaw = analogRead(TURBIDITY_PIN);

  turbidityValue = map(turbidityRaw, 0, 4095, 0, 100);

  // ================= FLOW =================

  int flowRaw = analogRead(FLOW_PIN);

  flowValue = map(flowRaw, 0, 4095, 0, 100);

  // ================= LUX =================

  int luxRaw = analogRead(LUX_PIN);

  luxValue = map(luxRaw, 0, 4095, 0, 1000);

  // ================= ULTRASONIC =================

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  distance = duration * 0.034 / 2;
}

// ======================================================
// KONTROL TDS
// ======================================================

void kontrolTDS() {

  if (tdsValue < 1000) {

    Serial.println("TDS rendah -> dosing nutrisi");

    // Nutrisi A ON
    digitalWrite(RELAY_TDS_A, HIGH);

    delay(2000);

    digitalWrite(RELAY_TDS_A, LOW);

    delay(1000);

    // Nutrisi B ON
    digitalWrite(RELAY_TDS_B, HIGH);

    delay(2000);

    digitalWrite(RELAY_TDS_B, LOW);

    // Tunggu pencampuran
    Serial.println("Menunggu pencampuran nutrisi...");

    delay(5000);
  }
}

// ======================================================
// KONTROL PH
// ======================================================

void kontrolPH() {

  // pH terlalu rendah
  if (phValue < 5.5) {

    Serial.println("pH rendah -> pH UP aktif");

    digitalWrite(RELAY_PH_UP, HIGH);
    digitalWrite(RELAY_PH_DOWN, LOW);
  }

  // pH terlalu tinggi
  else if (phValue > 7.5) {

    Serial.println("pH tinggi -> pH DOWN aktif");

    digitalWrite(RELAY_PH_DOWN, HIGH);
    digitalWrite(RELAY_PH_UP, LOW);
  }

  // pH normal
  else {

    digitalWrite(RELAY_PH_UP, LOW);
    digitalWrite(RELAY_PH_DOWN, LOW);
  }
}

// ======================================================
// WATER LEVEL
// ======================================================

void kontrolWaterLevel() {

  // Air rendah
  if (distance <100) {

    tone(BUZZER_WATER, 800);

    Serial.println("WARNING: AIR RENDAH!");

  } else {

    noTone(BUZZER_WATER);
  }
}

// ======================================================
// TURBIDITY
// ======================================================

void kontrolTurbidity() {

  // Air keruh
  if (turbidityValue > 70) {

    tone(BUZZER_TURBIDITY, 1200);

    Serial.println("WARNING: AIR KERUH!");

  } else {

    noTone(BUZZER_TURBIDITY);
  }
}

// ======================================================
// FLOW WATER
// ======================================================

void kontrolFlow() {

  // Flow terlalu rendah
  if (flowValue < 30) {

    tone(BUZZER_FLOW, 600);

    Serial.println("WARNING: FLOW RENDAH!");

  } else {

    noTone(BUZZER_FLOW);
  }
}

// ======================================================
// GROW LIGHT
// ======================================================

void kontrolGrowLight() {

  // Lux rendah
  if (luxValue < 700) {

    digitalWrite(LED_LUX, HIGH);

    Serial.println("Grow Light ON");

  } else {

    digitalWrite(LED_LUX, LOW);
  }
}

// ======================================================
// OLED DISPLAY
// ======================================================

void tampilOLED() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("SMART NFT SYSTEM");

  display.setCursor(0, 10);
  display.print("Temp : ");
  display.print(suhuAir);
  display.println(" C");

  display.setCursor(0, 20);
  display.print("pH   : ");
  display.println(phValue);

  display.setCursor(0, 30);
  display.print("TDS  : ");
  display.print(tdsValue);
  display.println(" ppm");

  display.setCursor(0, 40);
  display.print("Lux  : ");
  display.println(luxValue);

  display.setCursor(0, 50);
  display.print("Flow : ");
  display.println(flowValue);

  display.display();
}

// ======================================================
// SERIAL MONITOR
// ======================================================

void serialMonitor() {

  Serial.println("========= SMART NFT =========");

  Serial.print("Suhu Air : ");
  Serial.print(suhuAir);
  Serial.println(" C");

  Serial.print("pH       : ");
  Serial.println(phValue);

  Serial.print("TDS      : ");
  Serial.print(tdsValue);
  Serial.println(" ppm");

  Serial.print("Lux      : ");
  Serial.println(luxValue);

  Serial.print("Turbid   : ");
  Serial.println(turbidityValue);

  Serial.print("Flow     : ");
  Serial.println(flowValue);

  Serial.print("WaterLvl : ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.println();
}