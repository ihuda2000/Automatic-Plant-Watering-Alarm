# Automatic Plant Watering Alarm

A smart plant-care system built with an **ESP8266 (NodeMCU)**, a **capacitive soil moisture sensor**, and a **piezo buzzer**.  
The device monitors soil moisture in real time and sounds an alarm when the soil gets too dry.  
It also hosts a simple **web interface** where you can:

- View the current moisture reading  
- Change the dry-soil threshold  
- Enable/disable the alarm  
- Run a beep test  
- Persist your settings (stored in EEPROM)

---

## Features
- Real-time soil moisture monitoring (0–1023 analog values)  
- Audible alarm (buzzer) when soil is below threshold  
- Wi-Fi enabled — accessible via browser on your network  
- Web UI to configure threshold & alarm settings  
- Configuration saved to EEPROM (survives reset)  
- Simple wiring, USB-powered  

---

## Hardware Overview

### Parts
- ESP8266 NodeMCU (or compatible dev board)  
- Capacitive soil moisture sensor (analog output)  
- Piezo buzzer (passive) or small speaker  
- Breadboard, jumper wires, USB cable  
- *(Optional)* LED indicator and 3D-printed enclosure  

### Wiring Diagram
- NodeMCU / ESP8266
- Soil Sensor VCC  -> 3V3
- Soil Sensor GND  -> GND
- Soil Sensor AOUT -> A0

- Piezo Buzzer +   -> D5 (GPIO14)
- Piezo Buzzer -   -> GND

- LED   -> D4 (GPIO2) through 220Ω resistor to GND
