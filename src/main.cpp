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
const char* password = "12345678";   // remplace ici

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

byte colPins[COLS] = {13, 12, 14, 27};
byte rowPins[ROWS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================= VARIABLES =================
bool wifiOK = false;
bool modeAuto = true;
int luminosite = 0;
int valeurPot = 0;
int pageOLED = 0;

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

// ================= COMMANDES =================
void ledOn() {
  modeAuto = false;
  luminosite = 255;
}

void ledOff() {
  modeAuto = false;
  luminosite = 0;
}

void modeAutoOn() {
  modeAuto = true;
}

void changerEcranOLED() {
  pageOLED = (pageOLED + 1) % 2;
}

// ================= CLAVIER =================
void gererClavier() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Touche: ");
    Serial.println(key);

    if (key == '1') {
      ledOn();
    }
    else if (key == '2') {
      ledOff();
    }
    else if (key == '3') {
      modeAutoOn();
    }
    else if (key == 'A') {
      changerEcranOLED();
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

// ================= PAGE WEB =================
String genererPageHTML() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<title>ESP32 Maison Connectee</title>";
  html += "<style>";
  html += "body{font-family:Arial; padding:20px; background:#f5f5f5;}";
  html += "h1{color:#222;}";
  html += ".card{background:white; padding:16px; border-radius:12px; margin-bottom:16px; box-shadow:0 2px 8px rgba(0,0,0,0.1);}";
  html += ".btn{display:inline-block; padding:12px 18px; margin:6px; text-decoration:none; border-radius:8px; color:white; font-weight:bold;}";
  html += ".on{background:#28a745;}";
  html += ".off{background:#dc3545;}";
  html += ".auto{background:#007bff;}";
  html += ".oled{background:#6f42c1;}";
  html += "</style>";
  html += "</head><body>";

  html += "<h1>ESP32 Maison Connectee</h1>";

  html += "<div class='card'>";
  html += "<p><b>Potentiometre :</b> " + String(valeurPot) + "</p>";
  html += "<p><b>Luminosite LED :</b> " + String(luminosite) + "</p>";
  html += "<p><b>Mode :</b> " + String(modeAuto ? "AUTO" : "MANUEL") + "</p>";
  html += "<p><b>WiFi :</b> " + String(wifiOK ? "OK" : "NON") + "</p>";
  html += "<p><b>Page OLED :</b> " + String(pageOLED == 0 ? "Infos" : "Commandes") + "</p>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>Commandes</h2>";
  html += "<a class='btn on' href='/led/on'>LED ON</a>";
  html += "<a class='btn off' href='/led/off'>LED OFF</a>";
  html += "<a class='btn auto' href='/mode/auto'>MODE AUTO</a>";
  html += "<a class='btn oled' href='/oled/change'>CHANGER ECRAN OLED</a>";
  html += "</div>";

  html += "</body></html>";
  return html;
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

  if (!client.available()) {
    client.stop();
    return;
  }

  String request = client.readStringUntil('\r');
  client.flush();

  Serial.println("Requete HTTP:");
  Serial.println(request);

  bool action = false;

if (request.indexOf("GET /led/on") >= 0) {
  ledOn();
  action = true;
}
else if (request.indexOf("GET /led/off") >= 0) {
  ledOff();
  action = true;
}
else if (request.indexOf("GET /mode/auto") >= 0) {
  modeAutoOn();
  action = true;
}
else if (request.indexOf("GET /oled/change") >= 0) {
  changerEcranOLED();
  action = true;
}

if (action) {
  client.println("HTTP/1.1 303 See Other");
  client.println("Location: /");
  client.println("Connection: close");
  client.println();
  client.stop();
  return;
}

String html = genererPageHTML();

client.println("HTTP/1.1 200 OK");
client.println("Content-type:text/html; charset=utf-8");
client.println("Connection: close");
client.println();
client.println(html);

client.stop();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);

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