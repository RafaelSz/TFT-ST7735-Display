# ESP32 Zegar z Wyświetlaczem ST7735

Program Arduino dla płytki ESP32 wyświetlający bieżącą datę, godzinę i dzień tygodnia na małym kolorowym wyświetlaczu ST7735 (128x160 pikseli). Czas jest synchronizowany z serwerem NTP poprzez połączenie WiFi.

## Funkcje

- ✅ **Wyświetlanie godziny** — w formacie HH:MM:SS (biały tekst, duża czcionka)
- ✅ **Wyświetlanie daty** — w formacie DD.MM.YYYY (żółty tekst)
- ✅ **Wyświetlanie dnia tygodnia** — w pełnej polskiej nazwie (Poniedziałek, Wtorek, itd.)
- ✅ **Synchronizacja czasu** — poprzez NTP (serwer: pool.ntp.org)
- ✅ **Połączenie WiFi** — automatyczne łączenie się z sieciąWiFi przy starcie
- ✅ **Diagnostyka WiFi** — wypisanie dostępnych sieci, statusu połączenia, siły sygnału (RSSI)
- ✅ **Podświetlenie sterowane** — GPIO32 kontroluje LED wyświetlacza
- ✅ **Sprzętowe SPI** — komunikacja z wyświetlaczem przez VSPI z ręcznym resetem

## Wymagane komponenty

### Sprzęt
- **Płytka**: ESP32 DOIT DEVKIT V1
- **Wyświetlacz**: ST7735, 128x160 pikseli, inicjalizacja GREENTAB
- **Kabel USB**: do programowania i zasilania
- **Zasilanie**: 3.3V (z USB lub external)

### Biblioteki
- `Adafruit ST7735 and ST7789 Library` (v1.10.0+)
- `Adafruit GFX Library` (v1.11.5+)
- `SPI` (wbudowana w Arduino Framework)
- `WiFi` (wbudowana w Arduino Framework)

## Schemat Połączeń

Wyświetlacz ST7735 podłączyć do ESP32 następująco:

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

**Uwaga**: Wszystkie piny GPIO powinny być na poziomie 3.3V. Wyświetlacz wymaga zasilania 3.3V, **nie 5V**!

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
- Konwersja do czytelnego formatu (DD.MM.YYYY, HH:MM:SS)
- Wyświetlenie na ekranie:
  - **Wiersz 1-2**: "GODZINA:" + godzina (biały, duża czcionka)
  - **Wiersz 3-4**: "DATA:" + data (żółty, duża czcionka)
  - **Wiersz 5**: Dzień tygodnia (cyan, mała czcionka)

#### Funkcja: `setup()`
- Inicjalizacja UART (115200 baud)
- Włączenie podświetlenia (GPIO32)
- Inicjalizacja magistrali SPI (VSPI)
- Ręczny reset wyświetlacza
- Inicjalizacja wyświetlacza (INITR_GREENTAB)
- Test kolorów (RED, GREEN, BLUE, WHITE, BLACK)
- Połączenie z WiFi
- Synchronizacja czasu NTP

#### Funkcja: `loop()`
- Pobranie bieżącego czasu
- Wyświetlenie na ekranie
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

## Konsola Szeregowa — Przykładowe Wyjście

```
Inicjalizacja ST7735...
Używam INIT: GREENTAB
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
Bieżący czas: Thu Dec 11 21:15:30 2025
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

## Licencja

Projekt wykorzystuje biblioteki open-source:
- Adafruit (Adafruit ST7735, Adafruit GFX)
- Arduino Framework

## Autor

Projekt stworzony dla ESP32 DOIT DEVKIT V1 z wyświetlaczem ST7735 128x160.

---

**Ostatnia aktualizacja**: 11 grudnia 2025
