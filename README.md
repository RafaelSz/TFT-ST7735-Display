# ESP32 Zegar z Wyświetlaczem ST7735 i Czujnikiem DHT11

Program Arduino dla płytki ESP32 wyświetlający bieżącą datę, godzinę, dzień tygodnia, temperaturę i wilgotność na małym kolorowym wyświetlaczu ST7735 (128x160 pikseli). Czas jest synchronizowany z serwerem NTP poprzez połączenie WiFi, a dane o temperaturze i wilgotności są odczytywane z czujnika DHT11.

## Funkcje

- ✅ **Wyświetlanie godziny** — w formacie HH:MM:SS (biały tekst, duża czcionka)
- ✅ **Wyświetlanie daty** — w formacie DD.MM.YYYY z jednoliterową nazwą dnia tygodnia (żółty tekst)
- ✅ **Temperatura** — odczyt z czujnika DHT11, wyświetlanie jako "TEMP: XX.X C" (czerwony tekst, duża czcionka)
- ✅ **Wilgotność** — odczyt z czujnika DHT11, wyświetlanie jako "WILG: XX.X %" (cyjan tekst, duża czcionka)
- ✅ **Synchronizacja czasu** — poprzez NTP (serwer: pool.ntp.org)
- ✅ **Połączenie WiFi** — automatyczne łączenie się z siecią WiFi przy starcie
- ✅ **Diagnostyka WiFi** — wypisanie dostępnych sieci, statusu połączenia, siły sygnału (RSSI)
- ✅ **Podświetlenie sterowane** — GPIO32 kontroluje LED wyświetlacza
- ✅ **Sprzętowe SPI** — komunikacja z wyświetlaczem przez VSPI z ręcznym resetem
- ✅ **Optymalizacja wyświetlania** — selektywne odświeżanie tylko zmienionych wartości (bez migotania ekranu)
- ✅ **Serwer webowy** — wyświetlanie danych w przeglądarce internetowej z auto-odświeżaniem co 5 sekund
- ✅ **API JSON** — endpoint `/api` zwracający dane w formacie JSON

## Wymagane komponenty

### Sprzęt
- **Płytka**: ESP32 DOIT DEVKIT V1
- **Wyświetlacz**: ST7735, 128x160 pikseli, inicjalizacja BLACKTAB
- **Czujnik**: DHT11 (temperatura i wilgotność)
- **Kabel USB**: do programowania i zasilania
- **Zasilanie**: 3.3V (z USB lub external)

### Biblioteki
- `Adafruit ST7735 and ST7789 Library` (v1.10.0+)
- `Adafruit GFX Library` (v1.11.5+)
- `DHT sensor library` (v1.4.6+)
- `SPI` (wbudowana w Arduino Framework)
- `WiFi` (wbudowana w Arduino Framework)
- `WebServer` (wbudowana w Arduino Framework)

## Schemat Połączeń

### Wyświetlacz ST7735

| Moduł ST7735 | ESP32 Pin | Opis |
|---|---|---|
| **VCC** | 3.3V | Zasilanie (+3.3V) |
| **GND** | GND | Masa (Ground) |
| **SDA** | GPIO23 | MOSI (SPI data out) |
| **SCK** | GPIO18 | SCLK (SPI clock) |
| **A0 (DC)** | GPIO2 | Data/Command pin |
| **RESET** | GPIO4 | Resetowanie wyświetlacza |
| **CS** | GPIO15 | Chip Select |
| **LED** | GPIO32 | Podświetlenie (opcjonalne) |

### Czujnik DHT11

| DHT11 Pin | ESP32 Pin | Opis |
|---|---|---|
| **VCC** | 3.3V lub 5V | Zasilanie |
| **DATA** | GPIO5 | Pin danych czujnika |
| **GND** | GND | Masa (Ground) |

**Uwaga**: Wyświetlacz ST7735 wymaga zasilania 3.3V, **nie 5V**! Czujnik DHT11 może działać na 3.3V lub 5V.

## Instalacja i Konfiguracja

### 1. Pobranie projektu
```bash
git clone <URL-do-repozytorium>
cd TFT\ ST7735\ Display
```

### 2. Ustawienie danych WiFi
Otwórz plik `src/main.cpp` i zmień następujące linie (około linii 27-28):
```cpp
const char* ssid = "247-rx-24-fritz";        // Wpisz swoją sieć WiFi
const char* password = "CFDpass1.123.456";   // Wpisz swoje hasło
```

### 3. Ustawienie strefy czasowej
Jeśli nie jesteś w strefie UTC+1 (polska), zmień linię (około linii 32):
```cpp
const long gmtOffset_sec = 1 * 3600;  // Zamień 1 na odpowiednią liczbę godzin od UTC
// Np. dla UTC+2 (lato): gmtOffset_sec = 2 * 3600
// Np. dla UTC (zima): gmtOffset_sec = 0
```

### 4. Kompilacja i wgranie
```bash
# W katalogu projektu:
pio run -t upload -e esp32doit-devkit-v1 --upload-port /dev/ttyUSB0
```

### 5. Monitorowanie
Aby zobaczyć logi (diagnostyka WiFi, synchronizacja czasu):
```bash
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

### 6. Dostęp do serwera webowego
Po uruchomieniu urządzenia:
1. Sprawdź w monitorze szeregowym adres IP (np. `192.168.1.100`)
2. Otwórz przeglądarkę internetową
3. Wpisz adres: `http://[ADRES_IP]` (np. `http://192.168.1.100`)
4. Strona automatycznie odświeża się co 5 sekund

**API JSON**: Dostęp do danych w formacie JSON: `http://[ADRES_IP]/api`

Przykład odpowiedzi:
```json
{
  "time": "14:30:45",
  "date": "12.12.2025",
  "day": "P",
  "temperature": 22.5,
  "humidity": 45.3
}
```

## Struktura Programu

### Plik: `src/main.cpp`

#### Sekcja konfiguracji (linie 1-42)
- Definicje pinów GPIO
- Kredencjały WiFi
- Ustawienia NTP

#### Funkcja: `connectToWiFi()`
- Skanowanie dostępnych sieci WiFi
- Próba połączenia z siecią o podanym SSID
- Diagnostyka: wypisanie IP, RSSI (siła sygnału), BSSID routera

#### Funkcja: `setupTime()`
- Konfiguracja klienta NTP
- Synchronizacja czasu z serwerem `pool.ntp.org`
- Wypisanie bieżącego czasu do konsoli szeregowej

#### Funkcja: `displayDateTime()`
- Pobranie bieżącego czasu
- Odczyt temperatury i wilgotności z DHT11
- Konwersja do czytelnego formatu (DD.MM.YYYY, HH:MM:SS)
- **Optymalizacja**: Przechowywanie poprzednich wartości w zmiennych statycznych
- **Selektywne odświeżanie**: Aktualizacja tylko zmienionych elementów (eliminacja migotania)
- **Aktualizacja zmiennych globalnych**: Zapisanie wartości do zmiennych globalnych dla serwera webowego
- Wyświetlenie na ekranie:
  - **Wiersz 1**: Godzina HH:MM:SS (biały tekst, duża czcionka 3x)
  - **Wiersz 2**: Data DD.MM.YYYY + jednoliterowa nazwa dnia (żółty tekst, czcionka 2x)
  - **Wiersz 3**: TEMP: XX.X C (czerwony tekst, czcionka 2x)
  - **Wiersz 4**: WILG: XX.X % (cyjan tekst, czcionka 2x)

#### Funkcja: `handleRoot()`
- Generuje stronę HTML dla serwera webowego
- Responsywny design z gradientowym tłem
- Auto-odświeżanie co 5 sekund
- Wyświetla wszystkie dane z wyświetlacza (czas, data, temperatura, wilgotność)
- Kolorowe sekcje odpowiadające kolorom na wyświetlaczu

#### Funkcja: `handleAPI()`
- Zwraca dane w formacie JSON
- Endpoint: `/api`
- Umożliwia integrację z innymi systemami lub aplikacjami mobilnymi

#### Funkcja: `setup()`
- Inicjalizacja UART (115200 baud)
- Włączenie podświetlenia (GPIO32)
- Inicjalizacja magistrali SPI (VSPI)
- Ręczny reset wyświetlacza
- Inicjalizacja wyświetlacza (INITR_BLACKTAB)
- Test kolorów (RED, GREEN, BLUE, WHITE, BLACK)
- Inicjalizacja czujnika DHT11
- Połączenie z WiFi
- Synchronizacja czasu NTP
- Uruchomienie serwera webowego (nasłuchuje na porcie 80)
- Wyświetlenie adresu IP serwera w konsoli

#### Funkcja: `loop()`
- Obsługa żądań HTTP (server.handleClient())
- Selektywne odświeżanie wartości:
  - **Czas**: aktualizacja tylko gdy zmienią się sekundy
  - **Data**: aktualizacja tylko gdy zmieni się dzień
  - **Temperatura**: aktualizacja przy zmianie > 0.1°C
  - **Wilgotność**: aktualizacja przy zmianie > 0.1%
- Czyszczenie tylko konkretnych obszarów (bez migotania całego ekranu)
- Tekst rysowany z jawnym ustawieniem koloru tła (zapobiega nakładaniu się znaków)
- Aktualizacja co 1 sekundę

## Diagnostyka Problemów

### Problem: Program nie łączy się z WiFi
**Rozwiązanie**:
1. Sprawdź, czy wpisałeś poprawne SSID i hasło (wrażliwe na wielkość liter)
2. Upewnij się, że router transmituje w paśmie 2.4 GHz (ESP32 nie obsługuje 5 GHz)
3. Sprawdź, czy router nie ma filtrów MAC lub captive portal
4. Otwórz monitor szeregowy — zobaczysz listę dostępnych sieci i status połączenia

### Problem: Wyświetlacz pokazuje tylko kropki lub szum
**Rozwiązanie**:
1. Sprawdź wszystkie połączenia — szczególnie SDA (MOSI) i A0 (DC)
2. Upewnij się, że używasz GPIO2 dla pinu A0/DC (a nie innego pinu)
3. Sprawdź zasilanie — wyświetlacz musi mieć 3.3V na VCC
4. Spróbuj podłączyć pin LED bezpośrednio do 3.3V (zamiast GPIO32)

### Problem: Czas nie synchronizuje się
**Rozwiązanie**:
1. Upewnij się, że ESP32 jest połączony z WiFi
2. Sprawdź konsolę — komunikat "Bieżący czas:" powinien wyświetlić prawidłową datę
3. Jeśli WiFi nie łączy się, zmień serwer NTP w kodzie (np. na `"time.nist.gov"`)

## Pinowanie — Tablica Referencyjna

### Wyświetlacz ST7735
```
ESP32 Pin 18  ---- SCK  (VSPI SCLK)
ESP32 Pin 23  ---- SDA  (VSPI MOSI)
ESP32 Pin 2   ---- A0   (Data/Command)
ESP32 Pin 4   ---- RESET
ESP32 Pin 15  ---- CS   (Chip Select)
ESP32 Pin 32  ---- LED  (Backlight)
ESP32 GND     ---- GND
ESP32 3.3V    ---- VCC
```

### Czujnik DHT11
```
ESP32 Pin 5   ---- DATA (DHT11 data pin)
ESP32 GND     ---- GND
ESP32 3.3V    ---- VCC (lub 5V)
```

## Konsola Szeregowa — Przykładowe Wyjście

```
Inicjalizacja ST7735...
Używam INIT: GREENTAB
Inicjalizacja DHT11...
----- WiFi diagnostic start -----
Skanowanie pobliskich sieci WiFi...
Znaleziono 25 sieci:
1: 247-rx-24-fritz (RSSI: -59) SECURED
...
Łączenie z: 247-rx-24-fritz
Połączono z WiFi!
IP: 192.168.1.100
Sygnał (RSSI): -59
BSSID: AA:BB:CC:DD:EE:FF
----- WiFi diagnostic end -----
Synchronizacja czasu...
Bieżący czas: Thu Dec 12 11:00:00 2025
Serwer webowy uruchomiony!
Otwórz w przeglądarce: http://192.168.1.100
```

## Modyfikacje i Rozszerzenia

### Zmiana temperatury kolorów
W funkcji `displayDateTime()` możesz zmienić kolory tekstu:
```cpp
tft.setTextColor(ST7735_RED);      // Zmień kolor
```

Dostępne kolory:
- `ST7735_BLACK`, `ST7735_WHITE`
- `ST7735_RED`, `ST7735_GREEN`, `ST7735_BLUE`
- `ST7735_YELLOW`, `ST7735_CYAN`, `ST7735_MAGENTA`

### Zmiana rozmiaru tekstu
```cpp
tft.setTextSize(1);  // 1 = małe, 2 = średnie, 3 = duże
```

### Zmiana orientacji ekranu
```cpp
tft.setRotation(1);  // 0, 1, 2, 3 — 4 orientacje
```

## Notatki Techniczne

- **Frequency SPI**: ~10-20 MHz (zależy od ESP32 i modułu)
- **Pobór prądu**: ~50-100 mA (z podświetleniem LED)
- **Rozdzielczość**: 128x160 pikseli
- **Głębia kolorów**: 16-bit RGB565
- **Serwer webowy**: Port 80 (HTTP)
- **Auto-odświeżanie**: Strona odświeża się co 5 sekund
- **RAM**: Zużycie ~14.4% (47120 bajtów)
- **Flash**: Zużycie ~62.5% (819761 bajtów)

## Licencja

Projekt wykorzystuje biblioteki open-source:
- Adafruit (Adafruit ST7735, Adafruit GFX)
- Arduino Framework

## Autor

RafaelSz. Projekt stworzony dla ESP32 DOIT DEVKIT V1 z wyświetlaczem ST7735 128x160.

## Layout wyświetlacza

```
┌────────────────────────┐
│  HH:MM:SS              │  ← Godzina (biały, 3x)
│                        │
│  DD.MM.YYYY D          │  ← Data + dzień (żółty + cyan, 2x)
│                        │
│  TEMP: XX.X C          │  ← Temperatura (czerwony, 2x)
│                        │
│  WILG: XX.X %          │  ← Wilgotność (zielony, 2x)
│                        │
└────────────────────────┘
```

Gdzie `D` to jednoliterowa nazwa dnia tygodnia: N (Niedziela), P (Poniedziałek), W (Wtorek), S (Środa), C (Czwartek), P (Piątek), S (Sobota).

---

**Ostatnia aktualizacja**: 12 grudnia 2025
