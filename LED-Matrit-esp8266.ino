#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

// --- LED Display Config ---
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN D8
MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// --- RTC ---
RTC_DS3231 rtc;

// --- Wi-Fi Config ---
const char* ssid = "LUCKY Led Display";
const char* password = "12345678";

// --- DNS/Web Server ---
const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer server(80);

// --- State Variables ---
String incomingMessage = "HELLO  LUCKY";
int scrollSpeed = 75;
int displayBrightness = 5;
bool showTime = true;
bool use12HourFormat = false;
bool startupGreetingShown = false;

// --- Web UI HTML ---
const char htmlPage[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>LUCKY LED Display</title><style>
body { font-family: 'Segoe UI', sans-serif; background: #121212; color: #fff; margin: 0; padding: 1rem; }
h2 { color: #03dac5; text-align: center; }
.container { max-width: 500px; margin: auto; }
.card { background: #1e1e1e; border-radius: 10px; padding: 1rem 1.5rem; margin-bottom: 1rem; box-shadow: 0 0 10px #0005; }
label { font-size: 0.9rem; display: block; margin-top: 0.5rem; }
input[type="text"], input[type="number"] { width: 100%; padding: 10px; margin-top: 5px; border: none; border-radius: 5px; font-size: 1rem; }
input[type="range"] { width: 100%; margin-top: 5px; }
.range-label { display: flex; justify-content: space-between; font-size: 0.9rem; }
.switch { display: flex; align-items: center; margin: 0.5rem 0; }
.switch input { margin-right: 0.5rem; }
button { background-color: #03dac5; color: #000; font-weight: bold; padding: 12px; width: 100%; border: none; border-radius: 5px; font-size: 1rem; cursor: pointer; margin-top: 1rem; }
button:hover { background-color: #028f82; }
.row { display: flex; gap: 0.5rem; }
.row input { flex: 1; }
</style></head><body>
<div class="container">
  <h2>LUCKY LED Display</h2>
  <div class="card">
    <label for="msg">Custom Message</label>
    <input id="msg" type="text" placeholder="Enter message" maxlength="64" />
    <div class="range-label">
      <label for="speed">Scroll Speed</label><span id="speedVal">75</span>
    </div>
    <input id="speed" type="range" min="10" max="200" value="75" oninput="speedVal.textContent = this.value" />
    <div class="range-label">
      <label for="brightness">Brightness</label><span id="brightnessVal">5</span>
    </div>
    <input id="brightness" type="range" min="0" max="15" value="5" oninput="brightnessVal.textContent = this.value" />
    <div class="switch"><input type="checkbox" id="mode" /><label for="mode">Show Custom Message</label></div>
    <div class="switch"><input type="checkbox" id="format" /><label for="format">Use 12-Hour Time</label></div>
    <button onclick="sendData()">Apply Settings</button>
  </div>
  <div class="card">
    <h3 style="text-align:center">Set Time</h3>
    <div class="row">
      <input id="hour" type="number" placeholder="Hour" min="0" max="23" />
      <input id="minute" type="number" placeholder="Minute" min="0" max="59" />
    </div>
    <button onclick="setTime()">Set Time</button>
  </div>
</div>
<script>
function sendData() {
  const msg = encodeURIComponent(document.getElementById("msg").value);
  const spd = document.getElementById("speed").value;
  const brt = document.getElementById("brightness").value;
  const mode = document.getElementById("mode").checked ? 1 : 0;
  const format = document.getElementById("format").checked ? 1 : 0;
  fetch(`/update?msg=${msg}&spd=${spd}&brt=${brt}&mode=${mode}&format=${format}`)
    .then(() => alert("Settings applied!"));
}
function setTime() {
  const h = document.getElementById("hour").value;
  const m = document.getElementById("minute").value;
  fetch(`/settime?hour=${h}&minute=${m}&second=0&year=2025&month=1&day=1`)
    .then(() => alert("Time updated!"));
}
</script>
</body></html>
)rawliteral";

// --- Server Handlers ---
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleUpdate() {
  if (server.hasArg("msg")) {
    incomingMessage = server.arg("msg");
    incomingMessage.replace("+", " ");
  }
  if (server.hasArg("spd")) scrollSpeed = constrain(server.arg("spd").toInt(), 10, 200);
  if (server.hasArg("brt")) {
    displayBrightness = constrain(server.arg("brt").toInt(), 0, 15);
    display.setIntensity(displayBrightness);
  }
  if (server.hasArg("mode")) {
    showTime = (server.arg("mode").toInt() == 0);
    display.displayClear();
    if (!showTime) display.displayScroll(incomingMessage.c_str(), PA_LEFT, PA_SCROLL_LEFT, scrollSpeed);
  }
  if (server.hasArg("format")) use12HourFormat = (server.arg("format").toInt() == 1);
  server.send(204);
}

void handleSetTime() {
  int hour = server.arg("hour").toInt();
  int minute = server.arg("minute").toInt();
  rtc.adjust(DateTime(2025, 1, 1, hour, minute, 0)); // Set time with fixed date
  display.displayClear();
  server.send(204);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  delay(100);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.on("/settime", handleSetTime);
  server.onNotFound(handleRoot);
  server.begin();

  display.begin();
  display.setIntensity(displayBrightness);
  display.displayClear();
  display.setTextAlignment(PA_CENTER);

  Wire.begin(D2, D1); // I2C for RTC
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  display.displayScroll(incomingMessage.c_str(), PA_LEFT, PA_SCROLL_LEFT, scrollSpeed);
}

// --- Loop ---
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (!startupGreetingShown && display.displayAnimate()) {
    startupGreetingShown = true;
    display.displayClear();
  }

  if (startupGreetingShown) {
    if (showTime) {
      static unsigned long lastUpdate = 0;
      if (millis() - lastUpdate > 10000) {
        lastUpdate = millis();
        DateTime now = rtc.now();
        float tempC = rtc.getTemperature();
        int h = now.hour();
        String ampm = "";
        if (use12HourFormat) {
          if (h == 0) h = 12;
          else if (h > 12) h -= 12;
          ampm = now.hour() >= 12 ? " PM" : " AM";
        }
        char buf[64];
        snprintf(buf, sizeof(buf), "Time: %02d:%02d%s  Temp: %.1fC", h, now.minute(), ampm.c_str(), tempC);
        display.displayClear();
        display.displayScroll(buf, PA_LEFT, PA_SCROLL_LEFT, scrollSpeed);
      }
      if (display.displayAnimate()) display.displayReset();
    } else {
      if (display.displayAnimate()) display.displayReset();
    }
  }
}
