#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= LED + POT =================
#define LED_PIN 2
#define POT_PIN 34

#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// ================= WIFI =================
const char* ssid = "Sims43";
const char* password = "12345678";   // remplace ici par ton vrai mot de passe

WiFiServer server(80);

// ================= CLAVIER =================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Broches clavier validées
byte colPins[COLS] = {13, 12, 14, 27};
byte rowPins[ROWS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================= VARIABLES =================
bool wifiOK = false;
bool modeAuto = true;
int luminosite = 0;
int valeurPot = 0;
int pageOLED = 0;   // 0 = page infos, 1 = page commandes

// ================= WIFI =================
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiOK = true;
    Serial.println("\nConnecte !");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    server.begin();
  } else {
    Serial.println("\nEchec WiFi");
    wifiOK = false;
  }
}

// ================= OLED =================
void afficherPageInfos() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Maison connectee");

  display.setCursor(0, 12);
  display.print("WiFi: ");
  display.println(wifiOK ? "OK" : "NON");

  display.setCursor(0, 24);
  display.print("Pot: ");
  display.println(valeurPot);

  display.setCursor(0, 36);
  display.print("LED: ");
  display.println(luminosite);

  display.setCursor(0, 48);
  display.print("Mode: ");
  display.println(modeAuto ? "AUTO" : "MANUEL");

  display.display();
}

void afficherPageCommandes() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("Commandes");

  display.setCursor(0, 14);
  display.println("1 = LED ON");

  display.setCursor(0, 26);
  display.println("2 = LED OFF");

  display.setCursor(0, 38);
  display.println("3 = AUTO");

  display.setCursor(0, 50);
  display.println("A = Changer ecran");

  display.display();
}

void afficherOLED() {
  if (pageOLED == 0) {
    afficherPageInfos();
  } else {
    afficherPageCommandes();
  }
}

// ================= CLAVIER =================
void gererClavier() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Touche: ");
    Serial.println(key);

    if (key == '1') {
      modeAuto = false;
      luminosite = 255;
    }
    else if (key == '2') {
      modeAuto = false;
      luminosite = 0;
    }
    else if (key == '3') {
      modeAuto = true;
    }
    else if (key == 'A') {
      pageOLED = (pageOLED + 1) % 2;   // alterne entre 0 et 1
    }
  }
}

// ================= LED + POT =================
void gererLED() {
  valeurPot = analogRead(POT_PIN);

  if (modeAuto) {
    luminosite = map(valeurPot, 0, 4095, 0, 255);
  }

  ledcWrite(PWM_CHANNEL, luminosite);
}

// ================= WEB =================
void gererWeb() {
  if (!wifiOK) return;

  WiFiClient client = server.available();
  if (!client) return;

  unsigned long timeout = millis();
  while (client.connected() && !client.available() && millis() - timeout < 1000) {
    delay(1);
  }

  if (client.available()) {
    client.readStringUntil('\r');
    client.flush();
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html>");
  client.println("<html><head><meta charset='utf-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<meta http-equiv='refresh' content='2'>");
  client.println("<title>ESP32 Maison Connectee</title></head>");
  client.println("<body style='font-family:Arial; padding:20px;'>");

  client.println("<h1>ESP32 Maison Connectee</h1>");
  client.println("<p><b>Potentiometre :</b> " + String(valeurPot) + "</p>");
  client.println("<p><b>Luminosite LED :</b> " + String(luminosite) + "</p>");
  client.println("<p><b>Mode :</b> " + String(modeAuto ? "AUTO" : "MANUEL") + "</p>");
  client.println("<p><b>WiFi :</b> " + String(wifiOK ? "OK" : "NON") + "</p>");

  client.println("<h2>Commandes clavier</h2>");
  client.println("<ul>");
  client.println("<li>1 = LED ON</li>");
  client.println("<li>2 = LED OFF</li>");
  client.println("<li>3 = Mode AUTO</li>");
  client.println("<li>A = Changer ecran OLED</li>");
  client.println("</ul>");

  client.println("</body></html>");
  client.stop();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // PWM LED
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur OLED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println("Demarrage...");
  display.display();

  // WiFi
  connectWiFi();
}

// ================= LOOP =================
void loop() {
  gererClavier();
  gererLED();
  afficherOLED();
  gererWeb();

  delay(150);
}