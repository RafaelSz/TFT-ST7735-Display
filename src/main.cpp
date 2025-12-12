#include <Arduino.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>
#include <DHT.h>

// Jeśli chcesz pominąć test i używać WiFi/NTP ustaw 0
#define DISABLE_WIFI 0

// ST7735 module pin mapping (module labels -> ESP32 GPIO)
// Module pins: LED, SCK, SDA, A0 (DC), RESET, CS, GND, VCC
// Set `TFT_LED` to the GPIO connected to the module's LED/backlight pin
// or -1 if the LED pin is tied to VCC or not controlled by the MCU.
#define TFT_LED    32   // LED/backlight (set to your GPIO, e.g. 2). Currently set to GPIO32
#define TFT_SCK    18   // SCK  -> GPIO18 (VSPI SCLK)
#define TFT_SDA    23   // SDA  -> GPIO23 (VSPI MOSI)
#define TFT_A0     2    // A0   -> GPIO2 (D/C). Zmieniono z 16 na 2 do testów
#define TFT_RESET  4    // RESET-> GPIO4
#define TFT_CS     15   // CS   -> GPIO15

// Default init for 128x160 modules: try different types if artifacts appear
// Options: INITR_GREENTAB, INITR_BLACKTAB, INITR_144GREENTAB, INITR_MINI160x80
#define TFT_INIT INITR_BLACKTAB
#define TFT_LED    32   // LED/backlight

// DHT11 sensor configuration
#define DHTPIN 5        // DHT11 data pin -> GPIO5
#define DHTTYPE DHT11   // DHT 11

// WiFi credentials - zmień na swoje
const char* ssid = "247-rx-24-fritz";
const char* password = "CFDpass1.123.456";

// NTP Server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 1 * 3600;           // UTC+1 (zmień odpowiednio)
const int daylightOffset_sec = 0;              // DST offset

// Polish day names (without diacritics for basic ASCII font)
const char* dayNamesPolish[] = {
  "Niedziela", "Poniedzialek", "Wtorek", "Sroda", 
  "Czwartek", "Piatek", "Sobota"
};

// Function to transliterate Polish characters to ASCII
String removePolishDiacritics(const char* text) {
  String result = "";
  for (int i = 0; text[i] != '\0'; i++) {
    unsigned char c = text[i];
    // UTF-8 encoded Polish characters (2-byte sequences)
    if (c == 0xC4 && text[i+1] == 0x85) { result += 'a'; i++; } // ą
    else if (c == 0xC4 && text[i+1] == 0x84) { result += 'A'; i++; } // Ą
    else if (c == 0xC4 && text[i+1] == 0x87) { result += 'c'; i++; } // ć
    else if (c == 0xC4 && text[i+1] == 0x86) { result += 'C'; i++; } // Ć
    else if (c == 0xC4 && text[i+1] == 0x99) { result += 'e'; i++; } // ę
    else if (c == 0xC4 && text[i+1] == 0x98) { result += 'E'; i++; } // Ę
    else if (c == 0xC5 && text[i+1] == 0x82) { result += 'l'; i++; } // ł
    else if (c == 0xC5 && text[i+1] == 0x81) { result += 'L'; i++; } // Ł
    else if (c == 0xC5 && text[i+1] == 0x84) { result += 'n'; i++; } // ń
    else if (c == 0xC5 && text[i+1] == 0x83) { result += 'N'; i++; } // Ń
    else if (c == 0xC3 && text[i+1] == 0xB3) { result += 'o'; i++; } // ó
    else if (c == 0xC3 && text[i+1] == 0x93) { result += 'O'; i++; } // Ó
    else if (c == 0xC5 && text[i+1] == 0x9B) { result += 's'; i++; } // ś
    else if (c == 0xC5 && text[i+1] == 0x9A) { result += 'S'; i++; } // Ś
    else if (c == 0xC5 && text[i+1] == 0xBA) { result += 'z'; i++; } // ź
    else if (c == 0xC5 && text[i+1] == 0xB9) { result += 'Z'; i++; } // Ź
    else if (c == 0xC5 && text[i+1] == 0xBC) { result += 'z'; i++; } // ż
    else if (c == 0xC5 && text[i+1] == 0xBB) { result += 'Z'; i++; } // Ż
    else { result += (char)c; }
  }
  return result;
}

// Use hardware SPI (VSPI) on ESP32 and Adafruit constructor for HW SPI (cs, dc, rst)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_A0, TFT_RESET);

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void connectToWiFi() {
  Serial.println("----- WiFi diagnostic start -----");

  // Scan available networks first (helpful to see if SSID is visible)
  Serial.println("Skanowanie pobliskich sieci WiFi...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("Brak sieci w zasięgu (0 wyników)");
  } else {
    Serial.printf("Znaleziono %d sieci:\n", n);
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (RSSI: %d) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");
      delay(10);
    }
  }

  // Prepare station mode and attempt connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  Serial.print("Łączenie z: "); Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  const int maxAttempts = 60; // ~30s
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print('.');
    attempts++;
  }
  Serial.println();

  wl_status_t st = WiFi.status();
  if (st == WL_CONNECTED) {
    Serial.println("Połączono z WiFi!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    Serial.print("Sygnał (RSSI): "); Serial.println(WiFi.RSSI());
    Serial.print("BSSID: "); Serial.println(WiFi.BSSIDstr());
  } else {
    Serial.println("Nie udało się połączyć z WiFi");
    Serial.printf("Status WiFi: %d\n", st);
    Serial.println("Możliwe przyczyny: niepoprawne SSID/hasło, SSID ukryty, sieć 5GHz lub filtr MAC/router blokuje urządzenia.");
  }

  Serial.println("----- WiFi diagnostic end -----");
}

void setupTime() {
  // Synchronizacja czasu z serwerem NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("Synchronizacja czasu...");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }
  Serial.println();
  Serial.print("Bieżący czas: ");
  Serial.println(ctime(&now));
}

void displayDateTime() {
  // Pobierz bieżący czas
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  // Formatuj datę
  char dateStr[32];
  strftime(dateStr, sizeof(dateStr), "%d.%m.%Y", timeinfo);
  
  // Formatuj godzinę
  char timeStr[32];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
  
  // Use Polish day name
  const char* polishDay = dayNamesPolish[timeinfo->tm_wday];
  
  // Odczytaj dane z DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Wyczyść ekran
  tft.fillScreen(ST7735_BLACK);
  
  // Time value (duża czcionka, u góry)
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(3);
  tft.setCursor(8, 8);
  tft.println(timeStr);

  // Date value
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(8, 40);
  tft.println(dateStr);

  // Dzień tygodnia
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(1);
  tft.setCursor(8, 68);
  tft.println(polishDay);
  
  // Temperatura
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.setCursor(8, 88);
  if (!isnan(temperature)) {
    tft.print("Temp: ");
    tft.print(temperature, 1);
    tft.println(" C");
  } else {
    tft.println("Temp: --");
  }
  
  // Wilgotność
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.setCursor(8, 104);
  if (!isnan(humidity)) {
    tft.print("Wilg: ");
    tft.print(humidity, 1);
    tft.println(" %");
  } else {
    tft.println("Wilg: --");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nInicjalizacja ST7735...");
  // Backlight / LED control: if TFT_LED >= 0 we control module LED pin
  if (TFT_LED >= 0) {
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH); // zwykle HIGH włącza podświetlenie; jeśli odwrotnie, ustaw LOW
  }
  // Initialize SPI bus (VSPI) with our pins: SCLK, MISO, MOSI, SS
  SPI.begin(TFT_SCK, 19, TFT_SDA, TFT_CS);

  // Manual reset pulse (ensure proper hardware reset timing)
  pinMode(TFT_RESET, OUTPUT);
  digitalWrite(TFT_RESET, LOW);
  delay(50);
  digitalWrite(TFT_RESET, HIGH);
  delay(50);

  // Inicjalizacja wyświetlacza dla 128x160 (GREENTAB)
  Serial.print("Używam INIT: "); Serial.println("GREENTAB");
  tft.initR(TFT_INIT);
  
  // Fix display offset/artifacts (set proper column/row offsets)
  tft.setRotation(1);          // Rotacja (0-3)
  tft.setAddrWindow(0, 0, 159, 127); // Reset address window
  tft.fillScreen(ST7735_BLACK);

  // Wyświetl dłuższy test kolorów aby łatwiej było zauważyć efekt
  tft.fillScreen(ST7735_RED);   delay(1000);
  tft.fillScreen(ST7735_GREEN); delay(1000);
  tft.fillScreen(ST7735_BLUE);  delay(1000);
  tft.fillScreen(ST7735_WHITE); delay(1000);
  tft.fillScreen(ST7735_BLACK); delay(500);
  // Wyświetl komunikat
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("Uruchamianie...");
  
  // Inicjalizacja sensora DHT11
  Serial.println("Inicjalizacja DHT11...");
  dht.begin();
  
  #if !defined(DISABLE_WIFI) || (DISABLE_WIFI == 0)
  // Połączenie z WiFi
  connectToWiFi();
  
  // Synchronizacja czasu
  setupTime();
  #else
    Serial.println("WiFi wyłączone (DISABLE_WIFI=1) — pomijam synchronizację czasu.");
  #endif
}

void loop() {
  // Aktualizuj wyświetlacz co 1 sekundę
  displayDateTime();
  delay(1000);
}