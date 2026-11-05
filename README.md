# Cosplay Arc Reactor (MK II)

Arc reactor pro cosplay Iron Mana na ESP8266 (NodeMCU) s WS2812B LEDkami a ovládáním přes webovou stránku z mobilu.

## Hardware

- NodeMCU ESP8266 (USB převodník CP2102 – je potřeba ovladač Silicon Labs CP210x)
- WS2812B ring 24 LED + 3 LED uprostřed, zapojené za ringem (indexy 0–23 ring, 24–26 střed)
- Data LEDek na pinu **D6**, pořadí barev GRB
- 3D díly (STL) jsou ve složce [`3D files`](3D%20files)

## Funkce

- **Boot sekvence po zapnutí** – jiskření ve středu, plazma se roztáčí kolem ringu a zrychluje, záblesk, naskočení. Délka roztáčení je nastavitelná (2–30 s).
- **Režimy:** Reactor (klidový běh), Spin, Pulse (tlukot srdce), Rainbow, Rave (128 BPM stroboskop), Overload
- **Akce:** znovu přehrát boot, Unibeam (nabití + výboj)
- **Nastavení:** barva ringu a středu (+ předvolby), rychlost animací, jas
- Nastavení se ukládá do flash paměti a vydrží vypnutí

## Ovládání

ESP vytváří vlastní Wi-Fi síť (router není potřeba):

| | |
|---|---|
| Wi-Fi | `MK2` |
| Heslo | `jarvis2008` (změň v `src/main.cpp` – `AP_PASS`) |
| Stránka | http://mk2.local nebo http://192.168.4.1 |

Na telefonu se stránka po připojení většinou otevře sama (captive portal).

Přes sériovou linku (115200 baud) jde poslat `b <0-255>` (jas) a `g` (přehrát boot), např. `python send.py g` (port COM9 je v `send.py`).

## Build a nahrání

Projekt používá [PlatformIO](https://platformio.org/):

```
pip install platformio
pio run -t upload
```

Při vysokém jasu bere 27 LEDek až ~1,6 A – USB z PC to spolehlivě neutáhne a baterie rychle ubývá, doporučený jas je do ~150.

## Struktura

- `src/main.cpp` – firmware (animace, Wi-Fi AP, web server, mDNS, ukládání nastavení)
- `src/web.h` – ovládací webová stránka
- `send.py` – posílání sériových příkazů z PC
- `platformio.ini` – konfigurace buildu
