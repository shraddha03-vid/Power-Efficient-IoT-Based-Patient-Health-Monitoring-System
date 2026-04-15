# Power-Efficient-IoT-Based-Patient-Health-Monitoring-System
Developed an embedded IoT system using Embedded C and microcontroller programming to monitor vital parameters  such as heart rate and body temperature. • Implemented low-power design with real-time data transmission to a remote platform for continuous health monitoring and alerts.

Methodology :

1. DHT22 Sensor (Room Temperature & Humidity):
This sensor measures the surrounding environmental temperature and humidity. These
readings help in understanding how the room’s condition might affect the patient’s
health.
2. MAX30102 Sensor (Heart Rate and SpO2):
The MAX30102 sensor measures the patient’s heart rate and blood oxygen saturation
(SpO2) using infrared and red LEDs. It provides essential physiological data for
continuous health monitoring.

3. DS18B20 Sensor (Body Temperature):
The DS18B20 sensor is used to measure the patient’s body temperature with high
accuracy. It operates on a 1-Wire communication protocol, ensuring reliable and noise-
free data transfer.

4. ESP32 (Wi-Fi + Processing Unit):
The ESP32 microcontroller serves as the central processing unit. It collects data from all
sensors, processes the readings, and utilizes its built-in Wi-Fi module to send this data to
a web server. Power efficiency is achieved using the deep sleep mode when not actively
sensing or transmitting data.

6. Web Server (on ESP32):
The ESP32 also functions as a local web server that hosts a dashboard. It updates the
health data in real time, making it easily accessible to users on the same network.

8. Real-time Dashboard (Smartphone/PC):
The dashboard displays all sensor readings—heart rate, SpO2, body temperature, and
room temperature/humidity—in an organized and user-friendly format. Users can
monitor the patient’s vital signs remotely through any Wi-Fi-enabled device such as a
smartphone or computer.
