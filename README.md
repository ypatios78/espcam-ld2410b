# espcam-ld2410b
ESP32CAM(ai-thinker)-LD2410B-HumanPresence
ESP32-CAM + LD2410B Project Documentation
==========================================

PROJECT STATUS:  WORKING FINE
Date: June 24, 2025

HARDWARE SETUP:
- ESP32-CAM AI-Thinker
- LD2410B Human Presence Sensor
- External 5V power supply

WIRING THAT WORKS:
LD2410B OUT → ESP32-CAM GPIO 14
LD2410B VCC → ESP32-CAM 5V  
LD2410B GND → ESP32-CAM GND

WHAT DOESN'T WORK:
- UART communication (GPIO 14/15, 12/13, 16/17 all failed)

- LD2410 library approach

- USB power (insufficient current)

WHAT WORKS :
 OUT pin digital detection

 Simple pinMode(14, INPUT) approach

 Real-time web interface

 Camera functionality

 WiFi connectivity




LESSONS LEARNED:
- Sometimes simple solutions are better than complex ones
- OUT pin bypasses all UART communication issues
- External 5V power is essential
- GPIO 14 is safe and reliable for digital input

FUTURE IMPROVEMENTS:
- Could add motion-triggered photo capture
- Email notifications on presence detection
- Data logging to SD card
- Integration with home automation systems








![image](https://github.com/user-attachments/assets/b1727d0e-1b40-4d62-8c7f-732de361ca80)








![image](https://github.com/user-attachments/assets/2cae7e8a-8c13-49e1-b742-196b6c7ab4b7)



















