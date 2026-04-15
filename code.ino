#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// ------------------ Sensor Pins ------------------
#define DHTTYPE DHT22 
#define DHTPIN 18        // DHT22 on GPIO18
#define DS18B20 4       // DS18B20 on GPIO5
#define REPORTING_PERIOD_MS 1000

// ------------------ Variables ------------------
float temperature = 0;
float humidity = 0;
float BPM = 0;
float SpO2 = 0;
float bodytemperature = 0;
String patientStatus = "Normal";
String patientColor = "green";

/* Wi-Fi credentials */
const char* ssid = "Shraddha's S24";     
const char* password = "s_vidhate"; 

DHT dht(DHTPIN, DHTTYPE); 
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
WebServer server(80);

// MAX30100
PulseOximeter pox;
uint32_t tsLastReport = 0;

// ------------------ Beat Detection ------------------
void onBeatDetected() {
  Serial.println("💓 Beat detected!");
}

// ------------------ Patient Status Logic ------------------
void evaluateStatus() {
  int level = 0; // 0=Normal, 1=Mild, 2=Warning, 3=Severe

  // SpO₂ rules
  if (SpO2 < 85) level = max(level, 3);
  else if (SpO2 < 90) level = max(level, 2);
  else if (SpO2 < 95) level = max(level, 1);

  // BPM rules
  if (BPM < 40 || BPM > 130) level = max(level, 3);
  else if ((BPM >= 40 && BPM < 50) || (BPM > 110 && BPM <= 130)) level = max(level, 2);
  else if ((BPM >= 50 && BPM < 60) || (BPM > 100 && BPM <= 110)) level = max(level, 1);

  // Body Temp rules
  if (bodytemperature > 40 || bodytemperature < 34) level = max(level, 3);
  else if (bodytemperature >= 39) level = max(level, 2);
  else if (bodytemperature >= 38) level = max(level, 1);

  // Room Temperature rules
  if (temperature > 35 || temperature < 15) level = max(level, 3);
  else if ((temperature >= 30 && temperature <= 35) || (temperature >= 15 && temperature < 18)) level = max(level, 2);
  else if ((temperature >= 28 && temperature < 30) || (temperature >= 18 && temperature < 20)) level = max(level, 1);

  // Humidity rules
  if (humidity < 20 || humidity > 80) level = max(level, 3);
  else if ((humidity >= 20 && humidity < 30) || (humidity > 70 && humidity <= 80)) level = max(level, 2);
  else if ((humidity >= 30 && humidity < 40) || (humidity > 60 && humidity <= 70)) level = max(level, 1);

  // Map to status
  if (level == 0) { patientStatus = "🟢 Normal"; patientColor = "#28a745"; }
  else if (level == 1) { patientStatus = "🟡 Mild"; patientColor = "#ffc107"; }
  else if (level == 2) { patientStatus = "🟠 Warning"; patientColor = "#fd7e14"; }
  else { patientStatus = "🔴 Severe"; patientColor = "#dc3545"; }
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
  pinMode(19, OUTPUT);

  dht.begin();
  sensors.begin();

  // WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(1000); Serial.print("."); }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  Wire.begin(21, 22);

  // MAX30100
  Serial.println("Initializing MAX30100...");
  if (!pox.begin()) {
    Serial.println("FAILED to initialize MAX30100. Check wiring!");
    while (1);
  }
  Serial.println("SUCCESS! MAX30100 detected.");
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

// ------------------ Loop ------------------
void loop() {
  server.handleClient();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    sensors.requestTemperatures();
    float dsTemp = sensors.getTempCByIndex(0);
    if (dsTemp != -127.0) bodytemperature = dsTemp;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;

    // Evaluate patient status
    evaluateStatus();

    // Debug print
    Serial.println("---- Patient Data ----");
    Serial.printf("Room Temp: %.2f °C\n", temperature);
    Serial.printf("Humidity: %.2f %%\n", humidity);
    Serial.printf("Heart Rate: %.2f BPM\n", BPM);
    Serial.printf("SpO2: %.2f %%\n", SpO2);
    Serial.printf("Body Temp: %.2f °C\n", bodytemperature);
    Serial.println("Overall Status: " + patientStatus);
    Serial.println("----------------------");

    tsLastReport = millis();
  }
}

// ------------------ Web Handlers ------------------
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// ------------------ HTML Dashboard ------------------
String SendHTML() {
 String html = "<!DOCTYPE html><html><head>";
 html += "<meta charset='UTF-8'>";
 html += "<title>Patient Health Monitoring</title>";
 html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";

 html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css'>";
 html += "<style>";
 html += "body { background: linear-gradient(135deg, #e3f2fd, #ffffff); font-family: Arial, sans-serif; text-align: center; }";
 html += ".card { display:inline-block; margin:15px; padding:20px; width:250px; background:#fff; border-radius:15px; box-shadow:0 4px 12px rgba(0,0,0,0.1); }";
 html += ".card h2 { font-size:1.2rem; margin-bottom:10px; color:#555; }";
 html += ".value { font-size:2rem; font-weight:bold; margin:10px 0; }";
 html += ".status { margin-top:20px; padding:20px; border-radius:12px; font-size:1.5rem; font-weight:bold; color:#fff; }";
 html += "</style></head><body>";
 html += "<h1 style='color:#008080;'>Health Monitoring Dashboard</h1>";

 html += "<div class='card'><h2><i class='fas fa-thermometer-half'></i> Room Temp</h2><div class='value'>" + String((int)temperature) + "°C</div></div>";
 html += "<div class='card'><h2><i class='fas fa-tint'></i> Humidity</h2><div class='value'>" + String((int)humidity) + "%</div></div>";
 html += "<div class='card'><h2><i class='fas fa-heartbeat'></i> Heart Rate</h2><div class='value'>" + String((int)BPM) + " BPM</div></div>";
 html += "<div class='card'><h2><i class='fas fa-burn'></i> SpO₂</h2><div class='value'>" + String((int)SpO2) + "%</div></div>";
 html += "<div class='card'><h2><i class='fas fa-thermometer-full'></i> Body Temp</h2><div class='value'>" + String((int)bodytemperature) + "°C</div></div>";

 html += "<div class='status' style='background:" + patientColor + ";'>" + patientStatus + "</div>";

 // Increased refresh interval to 10 seconds
 html += "<script>setInterval(()=>{location.reload();},10000);</script>";
 html += "</body></html>";
 return html;
}
