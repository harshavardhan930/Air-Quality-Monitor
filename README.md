# 🌍 Smart Air Quality Monitoring System using ESP32, LVGL, and Bluetooth

This project is a **Smart IoT-based Air Quality Monitoring System** built using an **ESP32 microcontroller**, a **TFT ILI9341 touchscreen**, and multiple sensors.  
It monitors **temperature, humidity, CO (Carbon Monoxide), LPG gas, PM2.5 dust particles**, and **fire detection**, displaying all readings on a **dynamic LVGL-based GUI**.  
Additionally, the ESP32 transmits live sensor data over **Bluetooth** to a smartphone or PC for remote monitoring.

---

## ✨ Features

- 🌡️ Temperature & Humidity Monitoring (DHT11)  
- 💨 LPG and CO Gas Detection (MQ2 and MQ7 sensors)  
- 🌫️ Dust Density (PM2.5) Measurement (GP2Y1010 sensor)  
- 🧯 Fire Detection with Audible Alarm (IR sensor + Buzzer)  
- 📊 Real-time Graphical Dashboard using LVGL  
- 👆 Touchscreen Interface for Multi-Screen Navigation  
- 📱 Bluetooth Data Transmission to Mobile/PC  
- ⚠️ Visual + Sound Alerts for Dangerous Conditions  

---

## 🧩 Components Used

| Component | Description |
|------------|-------------|
| **ESP32 Dev Module** | Main controller (Wi-Fi + Bluetooth) |
| **DHT11 Sensor** | Measures temperature & humidity |
| **MQ2 Sensor** | Detects LPG gas |
| **MQ7 Sensor** | Detects carbon monoxide |
| **Dust Sensor (GP2Y1010)** | Measures dust / PM2.5 concentration |
| **IR Fire Sensor** | Detects flame presence |
| **Buzzer** | Provides audible alerts |
| **TFT ILI9341 Display** | 240×320 touchscreen display |
| **LVGL Library** | Used to design a modern graphical interface |

---

## ⚙️ Working Principle

1. **ESP32** collects data from sensors (DHT11, MQ2, MQ7, Dust, and Fire).
2. Readings are processed and displayed on the TFT touchscreen using **LVGL** components (labels, arcs, sliders).
3. If **fire or gas** is detected:
   - The **buzzer** sounds a warning.
   - The corresponding **alert panel** on the screen changes color.
4. The ESP32 simultaneously sends live data over **Bluetooth** in this format:
Temperature,Humidity,CO_ppm,DustDensity,FireStatus
Example: 28.7,65.4,2.4,0.12,Yes

yaml
Copy code
5. Data can be viewed using any **Bluetooth Terminal app** or a custom mobile application.

---

## 🔌 Pin Configuration

| Sensor/Module | ESP32 Pin |
|----------------|-----------|
| DHT11 | GPIO 27 |
| Fire Sensor | GPIO 14 |
| Buzzer | GPIO 33 |
| MQ2 (LPG Sensor) | GPIO 13 |
| MQ7 (CO Sensor) | GPIO 34 (Analog) |
| Dust Sensor LED | GPIO 13 |
| TFT Display | As per `TFT_eSPI` configuration |

---

## 📦 Libraries Required

Make sure the following libraries are installed in your Arduino IDE:

- [LVGL](https://github.com/lvgl/lvgl)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [BluetoothSerial (ESP32 Core)](https://github.com/espressif/arduino-esp32)

---

## 🖼️ Project Images

### 🧱 Hardware Setup
| ESP32 Setup | Sensor Connections |
|:-------------:|:------------------:|
| ![Setup](images/setup.jpg) | ![Sensors](images/sensors.jpg) |

### 📊 Display Screens
| Home Screen | Alert Mode |
|:-------------:|:------------:|
| ![Display Home](images/display_home.jpg) | ![Display Alert](images/display_alert.jpg) |

*(Make sure your extracted images are placed in the `images/` folder with these filenames.)*

---

## 🎥 Demo Video

🎬 [Watch the working demo here](videos/demo.mp4)  
*(Upload your demonstration video to a folder named `videos/` in your repo.)*


🧠 Applications
Smart Home Air Quality Monitoring

Industrial Gas and Fire Detection Systems

IoT-based Environmental Tracking

Greenhouse or Lab Safety Monitoring

Educational Embedded Projects

👨‍💻 Author
Harsha Vardhana Raju
🎓 Electrical & Electronics Engineering
Swarnandra Institute of Engineering and Technology

📧 harsha.vardhana@example.com
🌐 GitHub Profile
