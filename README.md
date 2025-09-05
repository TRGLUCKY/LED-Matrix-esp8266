# LED-Matrix-esp8266                        
Wi-Fi Controlled LED Matrix with Real-Time Clock & Web Interface

Turn your ESP8266 into a wireless LED display system that shows custom messages, live time, and ambient temperature on an 8-device MAX7219 matrix. This project includes a sleek web interface for updates—no code changes required!

🚀 Features
🧠 Displays time and temperature using RTC DS3231

💬 Scrolls custom messages across LED matrix

🌐 Wi-Fi access point with captive DNS portal

🕹️ Web UI to update text, brightness, scroll speed, time format

💡 Adjustable display intensity and speed

⏰ Supports 12/24 hour format switching

📦 Hardware Required
Component	Description
ESP8266 (NodeMCU)	Microcontroller with Wi-Fi
MAX7219 x 8	LED dot matrix display driver
DS3231 RTC	Real-time clock module
Jumper wires	For connections
Breadboard	Optional, for prototyping
🛠️ Setup Instructions
Wire connections:

LED Matrix to ESP8266:
VCC -> 5V
GND -> GND
D8 (CS_PIN) -> D8 (GPIO15)
D7 (MOSI) -> D7 (GPIO13)
D5 (CLK) -> D5 (GPIO14)
DS3231 RTC Module to ESP8266:
VCC -> 3.3V or 5V
GND -> GND
SDA -> D2 (GPIO4)
SCL -> D1 (GPIO5)


Flash the ESP8266:

Upload the sketch using Arduino IDE with required libraries:

MD_Parola

MD_MAX72XX

ESP8266WebServer

DNSServer

RTClib

Connect:

ESP creates an AP: LUCKY Led Display (Password: 12345678)

Open any web browser and navigate to any page—you’ll be redirected to the settings UI

Control Your Display:

Type a message

Set speed and brightness

Switch between clock and message modes

🖥️ Web UI Preview
The interface is modern, responsive, and built with embedded HTML/CSS. Users can:

Type messages up to 64 characters

Adjust scroll speed (10–200)

Change display brightness (0–15)

Toggle between time/message display

Switch 12/24-hour format

Set the RTC time manually

📷 Example Output
Time: 02:25 PM  Temp: 28.5C
Or scrolls:

HELLO LUCKY
🤖 Future Improvements
Add support for multiple messages or scheduling

Host over existing Wi-Fi network

MQTT integration

Save settings to EEPROM

Button input for mode switching

📚 License
This project is open-source and free to modify. Feel lucky and build something awesome! 🍀

