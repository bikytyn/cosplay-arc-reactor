#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "web.h"

// Arc reactor: 24 LED ring + 3 LED uprostřed (zapojené za ringem), data na D6.
// Vlastní Wi-Fi "MK2", ovládání na http://mk2.local (nebo http://192.168.4.1).
// Sériové příkazy (115200):  b <0-255> = jas,  g = přehrát boot sekvenci

#define LED_PIN      D6
#define RING         24
#define CENTER       3
#define LED_COUNT    (RING + CENTER)

#define AP_SSID      "MK2"
#define AP_PASS      "jarvis2008"
#define HOSTNAME     "mk2"

// Boot sekvence [ms]
#define T_IGNITE   1200
#define T_FLASH     250
#define T_SETTLE   1200

// Unibeam [ms]
#define U_CHARGE   1600
#define U_BLAST     700
#define U_FADE      900

enum Mode { M_REACTOR, M_SPIN, M_PULSE, M_RAINBOW, M_RAVE, M_OVERLOAD, M_COUNT };

struct Settings {
  uint32_t magic;
  uint8_t mode, bri, speed, power;
  uint32_t ring, center;
  uint8_t bootSec;        // délka roztáčení při bootu [s]
} cfg;
const uint32_t MAGIC_V1 = 0xA4C0FF02;   // starší verze bez bootSec
const uint32_t MAGIC = 0xA4C0FF03;

unsigned long spinMs() { return cfg.bootSec * 1000UL; }
unsigned long bootMs() { return T_IGNITE + spinMs(); }

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);
DNSServer dns;

struct RGB { float r, g, b; };
RGB buf[LED_COUNT];
RGB ringC, hotC, centerC;

unsigned long bootT0, lastFrame, uniT0 = 0, lastChange = 0;
bool uniActive = false, dirty = false;
float headPos = 0, ph = 0, master = 1;
// Rave
float beatPh = 0; uint8_t ravePat = 0; uint16_t raveHue = 0; uint32_t beatNo = 0;
// Overload
unsigned long dropUntil = 0;
String line;

// ---------- pomocné ----------
static inline RGB mix(RGB a, RGB b, float k) {
  return {a.r + (b.r - a.r) * k, a.g + (b.g - a.g) * k, a.b + (b.b - a.b) * k};
}
static inline void add(int i, RGB c, float k) {
  buf[i].r += c.r * k; buf[i].g += c.g * k; buf[i].b += c.b * k;
}
static inline float frand() { return random(1000) / 1000.0f; }
static inline RGB hex2rgb(uint32_t h) {
  return {((h >> 16) & 255) / 255.0f, ((h >> 8) & 255) / 255.0f, (h & 255) / 255.0f};
}
static inline RGB hsv(uint16_t hue) {
  uint32_t c = strip.ColorHSV(hue, 255, 255);
  return hex2rgb(c);
}
float speedK() { return powf(2.0f, (cfg.speed - 50) / 25.0f); }  // 0.25x .. 4x

void applyColors() {
  ringC = hex2rgb(cfg.ring);
  centerC = hex2rgb(cfg.center);
  hotC = mix(ringC, {1, 1, 1}, 0.7f);
}

// ---------- uložení nastavení ----------
void defaults() {
  cfg = {MAGIC, M_REACTOR, 80, 50, 1, 0x2a7bff, 0xbfe8ff, 4};
}
void loadCfg() {
  EEPROM.begin(64);
  EEPROM.get(0, cfg);
  if (cfg.magic == MAGIC_V1) { cfg.magic = MAGIC; cfg.bootSec = 4; }   // převzít staré nastavení
  if (cfg.magic != MAGIC || cfg.mode >= M_COUNT) defaults();
  cfg.bootSec = constrain(cfg.bootSec, 2, 30);
  applyColors();
}
void markDirty() { dirty = true; lastChange = millis(); }
void saveIfNeeded() {
  if (dirty && millis() - lastChange > 3000) {   // šetříme flash: ukládá se až 3 s po poslední změně
    EEPROM.put(0, cfg);
    EEPROM.commit();
    dirty = false;
  }
}

// ---------- efekty ----------
void comet(float head, float tail, float power, RGB col, RGB hot) {
  for (int i = 0; i < RING; i++) {
    float d = head - i;
    while (d < -1) d += RING;
    while (d >= RING - 1) d -= RING;
    float k;
    if (d < 0) k = 1 + d;
    else if (d < tail) k = 1 - d / tail;
    else continue;
    k = k * k * power;
    add(i, mix(col, hot, k * k), k);
  }
}

void bootFrame(unsigned long t, float dt) {
  if (t < T_IGNITE) {
    float p = (float)t / T_IGNITE;
    for (int i = 0; i < CENTER; i++)
      if (frand() < 0.15f + 0.4f * p) add(RING + i, hotC, frand() * (0.2f + 0.6f * p));
    return;
  }
  t -= T_IGNITE;
  float p = (float)t / spinMs();
  float rps = 0.4f + 6.6f * p * p;
  headPos += rps * RING * dt;
  while (headPos >= RING) headPos -= RING;
  float tail = 3 + 14 * p;
  comet(headPos, tail, 0.6f + 0.4f * p, ringC, hotC);
  if (p > 0.45f) comet(headPos + RING / 2.0f, tail, (p - 0.45f) / 0.55f, ringC, hotC);
  if (p > 0.7f) {
    float fill = (p - 0.7f) / 0.3f;
    for (int i = 0; i < RING; i++) add(i, ringC, fill * fill * 0.5f);
  }
  float ts = t / 1000.0f;
  for (int i = 0; i < CENTER; i++)
    add(RING + i, mix(ringC, centerC, p), 0.15f + 0.6f * p + 0.15f * sinf(ts * rps * 6.28f) + 0.1f * frand());
}

void modeReactor() {
  for (int i = 0; i < RING; i++) {
    float wave = 0.5f + 0.5f * sinf(ph * 2.2f - i * (6.2832f / RING) * 2);
    add(i, mix(ringC, hotC, 0.25f * wave), 0.70f + 0.18f * wave + 0.06f * frand());
  }
  float breath = 0.85f + 0.15f * sinf(ph * 1.6f);
  for (int i = 0; i < CENTER; i++) add(RING + i, centerC, breath * (0.93f + 0.07f * frand()));
}

void modeSpin(float dt) {
  headPos += 1.6f * speedK() * RING * dt;
  while (headPos >= RING) headPos -= RING;
  for (int i = 0; i < RING; i++) add(i, ringC, 0.12f);
  comet(headPos, 9, 1, ringC, hotC);
  comet(headPos + RING / 2.0f, 9, 1, ringC, hotC);
  for (int i = 0; i < CENTER; i++) add(RING + i, centerC, 0.8f + 0.2f * sinf(ph * 6));
}

void modePulse() {
  // Tlukot srdce: dva údery a pauza
  float x = fmodf(ph, 1.2f);
  float b = expf(-sq((x - 0.05f) * 18)) + 0.7f * expf(-sq((x - 0.30f) * 18));
  float k = 0.12f + 0.88f * b;
  for (int i = 0; i < RING; i++) add(i, mix(ringC, hotC, b * 0.5f), k);
  for (int i = 0; i < CENTER; i++) add(RING + i, centerC, 0.25f + 0.75f * b);
}

void modeRainbow() {
  uint16_t base = (uint16_t)(ph * 12000);
  for (int i = 0; i < RING; i++) add(i, hsv(base + i * 65536L / RING), 0.9f);
  for (int i = 0; i < CENTER; i++) add(RING + i, mix(hsv(base + 32768), {1, 1, 1}, 0.4f), 0.9f);
}

void modeRave(float dt) {
  beatPh += dt * (128.0f / 60.0f) * speedK();   // 128 BPM * rychlost
  if (beatPh >= 1) {
    beatPh -= floorf(beatPh);
    beatNo++;
    raveHue += 12000 + random(30000);
    if (beatNo % 4 == 0) ravePat = random(5);
  }
  float env = expf(-beatPh * 5);
  RGB c = hsv(raveHue);
  RGB c2 = hsv(raveHue + 32768);
  for (int i = 0; i < RING; i++) {
    float k = 0;
    switch (ravePat) {
      case 0: k = env; break;                                              // celé bliknutí
      case 1: k = ((i + beatNo) % 2) ? env : 0; break;                     // ob jeden
      case 2: k = (((i / 3) + beatNo) % 2) ? env : env * 0.1f; break;      // segmenty
      case 3: k = (random(100) < 30) ? 1 : 0; break;                       // třpyt
      case 4: {                                                            // rotující výseč
        int h = (int)(beatPh * RING + beatNo * 6) % RING;
        int d = (i - h + RING) % RING;
        k = d < 8 ? 1 - d / 8.0f : 0; break;
      }
    }
    add(i, (i % 2 && ravePat != 3) ? c2 : c, k);
  }
  float s = env * env * env;
  for (int i = 0; i < CENTER; i++) add(RING + i, {1, 1, 1}, s);
}

void modeOverload() {
  unsigned long now = millis();
  if (now < dropUntil) return;                      // krátký výpadek
  if (random(1000) < 6) { dropUntil = now + 30 + random(90); return; }
  RGB red = {1, 0.08f, 0}, orange = {1, 0.4f, 0.05f};
  for (int i = 0; i < RING; i++) {
    float k = 0.35f + 0.65f * frand() * frand();
    add(i, mix(red, orange, frand()), k);
    if (random(1000) < 8) add(i, {1, 0.9f, 0.7f}, 1);   // jiskra
  }
  for (int i = 0; i < CENTER; i++) add(RING + i, mix(orange, {1, 1, 0.8f}, frand()), 0.4f + 0.6f * frand());
}

void unibeamOverlay(unsigned long t) {
  if (t < U_CHARGE) {
    float p = (float)t / U_CHARGE;
    // Ring se plní od spodu nahoru z obou stran, střed se nabíjí a chvěje
    float fillTo = p * (RING / 2.0f);
    for (int i = 0; i < RING; i++) {
      int d = min(i, RING - i);
      if (d <= fillTo) buf[i] = mix(buf[i], hotC, 0.6f + 0.4f * frand() * p);
    }
    for (int i = 0; i < CENTER; i++) buf[RING + i] = mix(buf[RING + i], {1, 1, 1}, p * (0.7f + 0.3f * frand()));
  } else if (t < U_CHARGE + U_BLAST) {
    for (int i = 0; i < LED_COUNT; i++) buf[i] = {1, 1, 1};
  } else if (t < U_CHARGE + U_BLAST + U_FADE) {
    float f = 1 - (float)(t - U_CHARGE - U_BLAST) / U_FADE;
    for (int i = 0; i < LED_COUNT; i++) buf[i] = mix(buf[i], {1, 1, 1}, f * f);
  } else {
    uniActive = false;
  }
}

void show() {
  for (int i = 0; i < LED_COUNT; i++) {
    uint8_t r = strip.gamma8(constrain(buf[i].r * master, 0, 1) * 255);
    uint8_t g = strip.gamma8(constrain(buf[i].g * master, 0, 1) * 255);
    uint8_t b = strip.gamma8(constrain(buf[i].b * master, 0, 1) * 255);
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

void frame(float dt) {
  memset(buf, 0, sizeof(buf));
  unsigned long t = millis() - bootT0;
  ph += dt * speedK();

  // Plynulé zapnutí/vypnutí
  float target = cfg.power ? 1 : 0;
  master += (target - master) * min(1.0f, dt * 6);

  if (t < bootMs()) { bootFrame(t, dt); return; }

  switch (cfg.mode) {
    case M_REACTOR:  modeReactor(); break;
    case M_SPIN:     modeSpin(dt); break;
    case M_PULSE:    modePulse(); break;
    case M_RAINBOW:  modeRainbow(); break;
    case M_RAVE:     modeRave(dt); break;
    case M_OVERLOAD: modeOverload(); break;
  }

  // Záblesk na konci boot sekvence
  t -= bootMs();
  if (t < T_FLASH + T_SETTLE) {
    float f = t < T_FLASH ? 1.0f : 1.0f - (float)(t - T_FLASH) / T_SETTLE;
    for (int i = 0; i < LED_COUNT; i++) buf[i] = mix(buf[i], {1, 1, 1}, f * f);
  }
  if (uniActive) unibeamOverlay(millis() - uniT0);
}

void startBoot() { bootT0 = millis(); headPos = 0; cfg.power = 1; master = 1; }

// ---------- web ----------
void sendState() {
  char b[160];
  snprintf(b, sizeof(b),
           "{\"mode\":%u,\"bri\":%u,\"speed\":%u,\"power\":%u,\"ring\":\"%06x\",\"center\":\"%06x\",\"boot\":%u}",
           cfg.mode, cfg.bri, cfg.speed, cfg.power, cfg.ring, cfg.center, cfg.bootSec);
  server.send(200, "application/json", b);
}

void handleSet() {
  if (server.hasArg("mode"))   { cfg.mode = constrain(server.arg("mode").toInt(), 0, M_COUNT - 1); headPos = 0; }
  if (server.hasArg("bri"))    { cfg.bri = constrain(server.arg("bri").toInt(), 5, 255); strip.setBrightness(cfg.bri); }
  if (server.hasArg("boot"))   cfg.bootSec = constrain(server.arg("boot").toInt(), 2, 30);
  if (server.hasArg("speed"))  cfg.speed = constrain(server.arg("speed").toInt(), 1, 100);
  if (server.hasArg("power"))  cfg.power = server.arg("power").toInt() ? 1 : 0;
  if (server.hasArg("ring"))   cfg.ring = strtoul(server.arg("ring").c_str(), nullptr, 16) & 0xFFFFFF;
  if (server.hasArg("center")) cfg.center = strtoul(server.arg("center").c_str(), nullptr, 16) & 0xFFFFFF;
  applyColors();
  markDirty();
  sendState();
}

void setupWeb() {
  IPAddress ip(192, 168, 4, 1);
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASS);

  dns.start(53, "*", ip);                    // captive portal: telefon po připojení sám otevře stránku
  MDNS.begin(HOSTNAME);
  MDNS.addService("http", "tcp", 80);

  server.on("/", [] { server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/api/state", sendState);
  server.on("/api/set", handleSet);
  server.on("/api/boot", [] { startBoot(); markDirty(); sendState(); });
  server.on("/api/unibeam", [] { uniActive = true; uniT0 = millis(); cfg.power = 1; sendState(); });
  server.onNotFound([] {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}

// ---------- hlavní ----------
void setup() {
  Serial.begin(115200);
  randomSeed(ESP.getCycleCount());
  loadCfg();
  cfg.power = 1;                              // po zapnutí vždy naběhne
  strip.begin();
  strip.setBrightness(cfg.bri);
  strip.clear();
  strip.show();
  setupWeb();
  startBoot();
  lastFrame = millis();
  Serial.printf("\nARC REACTOR ONLINE  wifi=%s  http://%s.local  http://192.168.4.1\n", AP_SSID, HOSTNAME);
}

void handleSerial(String s) {
  s.trim();
  if (!s.length()) return;
  if (s[0] == 'b') { cfg.bri = constrain(s.substring(1).toInt(), 0, 255); strip.setBrightness(cfg.bri); markDirty(); }
  else if (s[0] == 'g') startBoot();
  else { Serial.println("ERR"); return; }
  Serial.println("OK " + s);
}

void loop() {
  dns.processNextRequest();
  server.handleClient();
  MDNS.update();
  saveIfNeeded();

  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') { handleSerial(line); line = ""; }
    else line += ch;
  }

  unsigned long now = millis();
  if (now - lastFrame >= 16) {                // ~60 fps, ať zbyde čas na Wi-Fi
    float dt = (now - lastFrame) / 1000.0f;
    lastFrame = now;
    frame(dt);
    show();
  }
}
