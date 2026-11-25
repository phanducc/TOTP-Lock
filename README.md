# ESP32 TOTP Smart Door Lock
An ESP32-based access control system utilizing **TOTP (RFC 6238)** dynamic authentication (Google Authenticator compatible) with real-time cloud access logging to **Google Sheets** via **HTTPS**.
## Core Features & Tech Stack
- **Authentication:** TOTP (HMAC-SHA1, 30s time-step, 6-digit OTP, 80-bit key) with multi-user identification.
- **Time Sync:** NTP Client (`pool.ntp.org`, UDP/123) separating calculation timezone (`UTC+0`) and display timezone (`UTC+7`).
- **Cloud Logging:** HTTPS GET (`WiFiClientSecure`) → Google Apps Script Web App (`doGet`) → Google Sheets.
- **Actuator & UI:** PWM-driven RC Servo (50Hz, 500–2400µs), I2C 16x2 LCD (`0x27`), 4x4 Matrix Keypad, Exit Button (`INPUT_PULLUP`).
- **Hardware:** Custom 2-layer ESP32 expansion PCB & schematic (EasyEDA Standard) with independent `5V-2A` power distribution to prevent Brownout Resets.
## Repository Structure
```text
TOTP-Lock/
├── ESP32_TOTP_Lock/
│   └── ESP32_TOTP_Lock.ino                      # ESP32 firmware (Arduino IDE / C++)
├── GOOGLE_SCRIPT/
│   └── totp_lock.gs                             # Google Apps Script Web App endpoint
└── HW_TOTP_Lock/
    ├── Schematic_ESP32_TOTP_Lock_EasyEDA.json   # EasyEDA Standard schematic
    └── PCB_ESP32_TOTP_Lock_EasyEDA.json         # EasyEDA Standard 2-layer PCB layout
```
## Hardware & Pinout
| Module | Interface / Protocol | ESP32 GPIO | Power |
| :--- | :--- | :--- | :--- |
| **RC Servo (SG90/MG996R)** | PWM (50Hz) | `GPIO 13` | `5V`, `GND` |
| **LCD 16x2 (PCF8574)** | I2C (`0x27`) | `GPIO 21` (SDA), `GPIO 22` (SCL) | `5V`, `GND` |
| **Matrix Keypad 4x4** | Digital Matrix Scan | Rows: `19, 18, 17, 16`<br>Cols: `32, 33, 25, 26` | — |
| **Exit Button (NO)** | Digital Input (`INPUT_PULLUP`) | `GPIO 15` | `GND` |
> **Power Note:** Set the PCB voltage jumper to **5V** and supply an external `5V-2A` source. Connect the USB data cable directly to the ESP32 DevKit when flashing firmware.
## Quick Setup
### 1. Firmware (`ESP32_TOTP_Lock`)
- **Board:** `ESP32 Dev Module` (240MHz, Baudrate `115200`).
- **Libraries:** `LiquidCrystal_I2C`, `Keypad`, `ESP32Servo`, `NTPClient`, `TOTP` (`sha1.h` / Cryptosuite).
- Open `ESP32_TOTP_Lock/ESP32_TOTP_Lock.ino` and configure:
  - `ssid`, `password` (2.4GHz Wi-Fi credentials).
  - `GOOGLE_SCRIPT_ID` (Web App Deployment ID).
  - `hmacKey1`, `hmacKey2` (10-byte Hex arrays decoded from 16-character Base32 keys configured in Google Authenticator).
### 2. Cloud Logging (`GOOGLE_SCRIPT`)
1. Create a Google Sheet containing a worksheet named `Data` (columns: `Timestamp`, `User`, `Action`).
2. Go to **Extensions → Apps Script** and paste the code from `GOOGLE_SCRIPT/totp_lock.gs`.
3. Select **Deploy → New deployment → Web App** (*Execute as:* `Me`, *Who has access:* `Anyone`) and copy the **Deployment ID** into the firmware.
### 3. Hardware (`HW_TOTP_Lock`)
- Open `Schematic_ESP32_TOTP_Lock_EasyEDA.json` and `PCB_ESP32_TOTP_Lock_EasyEDA.json` in **EasyEDA Standard** (`File → Open → EasyEDA...`) to inspect the schematic, edit the layout, or export Gerber files.
