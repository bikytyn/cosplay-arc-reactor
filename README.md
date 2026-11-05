# Cosplay Arc Reactor (MK II)

An Iron Man arc reactor for cosplay, built on an ESP8266 (NodeMCU) with WS2812B LEDs and controlled from your phone through a web page.

## Hardware

- NodeMCU ESP8266 with a CP2102 USB-to-serial chip (you need the Silicon Labs CP210x driver)
- A 24-LED WS2812B ring, plus 3 center LEDs chained after it (indexes 0–23 are the ring, 24–26 are the center)
- LED data on pin **D6**, GRB color order
- 3D-printable parts (STL) are in [`3D files`](3D%20files)

## Features

- **Boot sequence on power-up:** sparks in the center, then plasma spins around the ring and speeds up, then a flash as the reactor comes online. You can set the spin-up length from 2 to 30 s.
- **Modes:**
  - Reactor (idle glow)
  - Spin
  - Pulse (heartbeat)
  - Rainbow
  - Rave (128 BPM strobe)
  - Overload
- **Actions:** replay the boot sequence, or fire the Unibeam (charge-up, then a blast)
- **Settings:** ring and center color (with presets), animation speed, brightness
- Settings are saved to flash, so they survive a power cycle.

## Control

The ESP creates its own Wi-Fi network, so you don't need a router:

| | |
|---|---|
| Wi-Fi | `MK2` |
| Password | `jarvis2008` (to change it, edit `AP_PASS` in `src/main.cpp`) |
| Web page | http://mk2.local or http://192.168.4.1 |

On most phones the page opens on its own after you connect (captive portal). Some phones don't support `mk2.local`. If it doesn't open, use `192.168.4.1`.

You can also send commands over serial (115200 baud):

- `b <0-255>` sets the brightness.
- `g` replays the boot sequence.

For example, `python send.py g`. The serial port is set in `send.py` (COM9).

## Build and flash

The project uses [PlatformIO](https://platformio.org/):

```
pip install platformio
pio run -t upload
```

At high brightness, the 27 LEDs can draw up to about 1.6 A. A PC USB port can't reliably supply that much, and it drains batteries fast. Keep the brightness at about 150 or lower.

## Project layout

- `src/main.cpp`: the firmware (animations, Wi-Fi access point, web server, mDNS, saved settings)
- `src/web.h`: the web control page
- `send.py`: sends serial commands from a PC
- `platformio.ini`: the build configuration
