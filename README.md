# Smart Watering Level Monitor

A smart water-level monitoring system built with ESP32 that detects water level using float switches, displays status on an LED matrix, sounds an alarm, and sends updates through a Telegram bot.

This project is designed for monitoring water conditions in a tank or storage container and alerting users when the water level moves into warning or danger states.

## Features

- Water level detection using two float switches
- Three levels of status:
  - AMAN: normal
  - WASPADA: warning
  - BAHAYA: danger
- LED matrix display for status messages
- Buzzer alarm with different behavior for warning vs danger
- LED indication for danger state
- Telegram bot integration for remote monitoring
- Commands to check status and control alarm

## System Behavior

The system reads two input pins:

- `PIN_SEDANG` (middle sensor)
- `PIN_TINGGI` (high sensor)

The logic is:

- If the high-level sensor is triggered, the system marks the condition as `BAHAYA`
- Else if the middle sensor is triggered, the system marks the condition as `WASPADA`
- Else the water is considered `AMAN`

When the level changes, the display is updated and the alarm logic is re-evaluated.

## Hardware Components

- ESP32 development board
- Two float switches / water-level sensors
- 4-module MAX7219 LED matrix (MD_MAX72xx)
- Buzzer
- LED
- Resistors and wiring for switches and output devices

## Pin Configuration

| Component | Pin |
| --- | --- |
| `PIN_SEDANG` | 32 |
| `PIN_TINGGI` | 33 |
| `PIN_LED` | 26 |
| `PIN_BUZZER` | 25 |
| `PIN_DIN` (MAX7219) | 23 |
| `PIN_CLK` (MAX7219) | 18 |
| `PIN_CS` (MAX7219) | 5 |

## Libraries Used

Install the following libraries in Arduino IDE or PlatformIO:

- `WiFi`
- `WiFiClientSecure`
- `UniversalTelegramBot`
- `ArduinoJson`
- `MD_Parola`
- `MD_MAX72xx`

## Telegram Bot Setup

Before uploading the code, configure the following values in `levelwatermonitoring.ino`:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BOT_TOKEN = "YOUR_BOT_TOKEN";
const String CHAT_ID_DIIZINKAN = "YOUR_TELEGRAM_CHAT_ID";
```

Important:

- Replace the Wi-Fi SSID and password with your network credentials
- Create a Telegram bot with BotFather and paste the token here
- Use the Telegram user ID that should be allowed to manage the alarm

## Telegram Commands

The bot supports the following commands:

- `/start`  
  Shows a welcome message and confirms the bot is active

- `/status`  
  Returns the current water condition and status of the alarm

- `/alarm_on`  
  Enables the alarm. Only the authorized chat ID can run this

- `/alarm_off`  
  Disables the alarm. Only the authorized chat ID can run this

## Alarm Logic

The project uses two alarm states:

- Warning state (`WASPADA`):
  - buzzer activates for 5 seconds after entering warning mode
  - status stays monitored continuously

- Danger state (`BAHAYA`):
  - buzzer remains active
  - LED remains on
  - alarm can only stop if disabled by authorized user

## Display Messages

The matrix displays messages such as:

- `JAGALAH KEBERSIHAN` for normal state
- `WASPADA - PANTAU LEVEL AIR` for warning state
- `BAHAYA - SEGERA KURANGI AIR` for danger state

## Example Workflow

1. Connect the ESP32 to Wi-Fi.
2. Boot the system.
3. The LED matrix initializes and displays the initial status.
4. Float switches detect water level.
5. The status is updated and displayed.
6. If the level reaches warning or danger, the buzzer and optional LED are triggered.
7. Users can receive remote status updates via Telegram.

## File Structure

- `levelwatermonitoring.ino` — main Arduino sketch
- `README.md` — project documentation

## Notes

- Do not commit real Telegram tokens or personal chat IDs to public repositories.
- If the project is used in a real installation, keep credentials in a secure environment and update them as needed.
- The code uses `WiFiClientSecure` with `setInsecure()` for compatibility, so ensure the project matches your security requirements before deployment.

## License

This project is provided as-is for educational and personal use.

## Project Summary

This is a compact IoT water monitoring solution that combines hardware sensing, local visual feedback, and Telegram-based remote alerting. It is suitable for monitoring water storage conditions and notifying users when the tank approaches risky water levels.
