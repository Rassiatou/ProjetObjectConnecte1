#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ====== PINS ======
#define LED_PIN 2
#define POT_PIN 34

// OLED I2C par defaut ESP32
// SDA = 21
// SCL = 22

// ====== PWM ======
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8   // 0 a 255

// ====== WIFI ======
const char* ssid = "Sims43";
const char* password = "12345678";   // remplace ici

WiFiServer server(80);

// ====== OLED ======
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ====== CLAVIER ======
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte colPins[COLS] = {19, 18, 32, 33};  // C1 C2 C3 C4 
byte rowPins[ROWS] = {25, 26, 27, 14};  // R1 R2 R3 R4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ====== VARIABLES ======
bool wifiConnected = false;
bool ledManuelle = false;       // true = mode manuel
int ledManualValue = 0;         // 0 ou 255
int oledPage = 0;               // 0 ou 1

int valeurPot = 0;
int luminosite = 0;
String modeActuel = "AUTO";

void afficherPage0() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Maison connectee");

  display.setCursor(0, 15);
  display.print("WiFi: ");
  display.println(wifiConnected ? "OK" : "...");

  display.setCursor(0, 28);
  display.print("Pot: ");
  display.println(valeurPot);

  display.setCursor(0, 41);
  display.print("LED: ");
  display.println(luminosite);

  display.setCursor(0, 54);
  display.print("Mode: ");
  display.println(modeActuel);

  display.display();
}

void afficherPage1() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Commandes clavier");

  display.setCursor(0, 14);
  display.println("1 = LED ON");

  display.setCursor(0, 26);
  display.println("2 = LED OFF");

  display.setCursor(0, 38);
  display.println("3 = Mode AUTO");

  display.setCursor(0, 50);
  display.println("A = Changer page");

  display.display();
}

void mettreAJourOLED() {
  if (oledPage == 0) {
    afficherPage0();
  } else {
    afficherPage1();
  }
}

void gererClavier() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Touche: ");
    Serial.println(key);

    if (key == '1') {
      ledManuelle = true;
      ledManualValue = 255;
      luminosite = 255;
      modeActuel = "MANUEL ON";
      ledcWrite(PWM_CHANNEL, luminosite);
    }
    else if (key == '2') {
      ledManuelle = true;
      ledManualValue = 0;
      luminosite = 0;
      modeActuel = "MANUEL OFF";
      ledcWrite(PWM_CHANNEL, luminosite);
    }
    else if (key == '3') {
      ledManuelle = false;
      modeActuel = "AUTO";
    }
    else if (key == 'A') {
      oledPage = (oledPage + 1) % 2;
    }
  }
}

void gererPotentiometreEtLED() {
  valeurPot = analogRead(POT_PIN);

  if (!ledManuelle) {
    luminosite = map(valeurPot, 0, 4095, 0, 255);
    ledcWrite(PWM_CHANNEL, luminosite);
  } else {
    luminosite = ledManualValue;
    ledcWrite(PWM_CHANNEL, luminosite);
  }
}

void connecterWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("WiFi connecte !");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    server.begin();
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("Echec connexion WiFi");
  }
}

void repondrePageWeb(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset='utf-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<meta http-equiv='refresh' content='2'>");
  client.println("<title>Maison connectee ESP32</title>");
  client.println("</head>");
  client.println("<body style='font-family:Arial; padding:20px;'>");

  client.println("<h1>Maison connectee ESP32</h1>");
  client.println("<p><b>WiFi :</b> OK</p>");
  client.println("<p><b>Potentiometre :</b> " + String(valeurPot) + "</p>");
  client.println("<p><b>Luminosite LED :</b> " + String(luminosite) + "</p>");
  client.println("<p><b>Mode :</b> " + modeActuel + "</p>");

  client.println("<h2>Commandes clavier</h2>");
  client.println("<ul>");
  client.println("<li>1 = LED ON</li>");
  client.println("<li>2 = LED OFF</li>");
  client.println("<li>3 = Mode AUTO</li>");
  client.println("<li>A = Changer page OLED</li>");
  client.println("</ul>");

  client.println("</body>");
  client.println("</html>");
}

void gererServeurWeb() {
  if (!wifiConnected) return;

  WiFiClient client = server.available();
  if (!client) return;

  unsigned long timeout = millis();
  while (client.connected() && !client.available() && millis() - timeout < 1000) {
    delay(1);
  }

  if (client.available()) {
    client.readStringUntil('\r');
    client.flush();
    repondrePageWeb(client);
  }

  client.stop();
}

void setup() {
  Serial.begin(115200);

  // PWM LED
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur OLED");
    for (;;) {}
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Demarrage...");
  display.display();

  connecterWiFi();
  mettreAJourOLED();
}

void loop() {
  gererClavier();
  gererPotentiometreEtLED();
  mettreAJourOLED();
  gererServeurWeb();

  Serial.print("Pot: ");
  Serial.print(valeurPot);
  Serial.print(" | LED: ");
  Serial.print(luminosite);
  Serial.print(" | Mode: ");
  Serial.print(modeActuel);
  Serial.print(" | WiFi: ");
  Serial.println(wifiConnected ? "OK" : "NON");

  delay(150);
}