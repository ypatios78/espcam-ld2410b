#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Camera model selection - AI Thinker ESP32-CAM
#define CAMERA_MODEL_AI_THINKER

// WiFi credentials
const char* ssid = "";    // name of your ssid
const char* password = ""; // wifi password 

// LD2410B OUT pin - Simple digital presence detection
#define PRESENCE_PIN 14  // LD2410B OUT -> ESP32 GPIO 14

// Web server
WebServer server(80);

// Sensor data
bool humanDetected = false;
String sensorStatus = "Initializing...";
unsigned long lastPresenceChange = 0;
unsigned long presenceDuration = 0;

// Camera pin definitions for AI_THINKER model
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Global variables for sensor readings
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 500; // Read every 500ms

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize LD2410B OUT pin as input
  pinMode(PRESENCE_PIN, INPUT);
  Serial.println("LD2410B OUT pin initialized on GPIO 14");
  Serial.println("Simple presence detection mode - no UART needed!");

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Web server routes
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/sensor", handleSensorData);
  server.on("/stream", handleStream);
  
  server.begin();
  Serial.println("HTTP server started");
  
  sensorStatus = "LD2410B OUT pin ready";
}

void loop() {
  server.handleClient();
  
  // Read sensor data periodically
  if (millis() - lastSensorRead > sensorInterval) {
    readLD2410BOutPin();
    lastSensorRead = millis();
  }
}

void readLD2410BOutPin() {
  bool currentPresence = digitalRead(PRESENCE_PIN);
  
  // Detect presence state change
  if (currentPresence != humanDetected) {
    humanDetected = currentPresence;
    lastPresenceChange = millis();
    
    if (humanDetected) {
      Serial.println("👤 Human presence detected!");
      sensorStatus = "Human presence detected";
    } else {
      Serial.println("❌ No human presence");
      sensorStatus = "No human presence";
    }
  }
  
  // Calculate presence duration
  if (humanDetected) {
    presenceDuration = millis() - lastPresenceChange;
  } else {
    presenceDuration = 0;
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32-CAM Human Presence Monitor - OUT Pin Mode</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      margin: 0; 
      padding: 20px; 
      color: white;
    }
    .container { 
      max-width: 800px; 
      margin: 0 auto; 
      background: rgba(255,255,255,0.1); 
      padding: 30px; 
      border-radius: 15px;
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 32px rgba(0,0,0,0.3);
    }
    h1 { 
      text-align: center; 
      margin-bottom: 30px;
      font-size: 2.5em;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
    }
    .sensor-panel {
      background: rgba(255,255,255,0.2);
      padding: 20px;
      border-radius: 10px;
      margin: 20px 0;
      text-align: center;
    }
    .presence-indicator {
      font-size: 4em;
      margin: 20px 0;
      transition: all 0.3s ease;
    }
    .present { color: #ff4444; text-shadow: 0 0 20px #ff4444; }
    .absent { color: #44ff44; text-shadow: 0 0 20px #44ff44; }
    .status-display {
      font-size: 1.5em;
      margin: 10px 0;
      padding: 15px;
      border-radius: 8px;
      background: rgba(255,255,255,0.2);
    }
    .human-detected { background-color: rgba(255,0,0,0.7) !important; }
    .no-human { background-color: rgba(0,255,0,0.7) !important; }
    .duration-display {
      font-size: 1.2em;
      margin: 15px 0;
      padding: 10px;
      background: rgba(255,255,255,0.15);
      border-radius: 5px;
    }
    button {
      background: linear-gradient(45deg, #FF6B6B, #4ECDC4);
      color: white;
      border: none;
      padding: 15px 30px;
      margin: 10px;
      border-radius: 25px;
      cursor: pointer;
      font-size: 16px;
      transition: transform 0.3s;
    }
    button:hover { transform: scale(1.05); }
    .camera-frame {
      text-align: center;
      margin: 20px 0;
    }
    img { 
      max-width: 100%; 
      border-radius: 10px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.3);
    }
    .info-box {
      background: rgba(255,255,255,0.1);
      padding: 15px;
      border-radius: 8px;
      margin: 15px 0;
      font-size: 0.9em;
    }
  </style>
  <script>
    function updateSensorData() {
      fetch('/sensor')
        .then(response => response.json())
        .then(data => {
          // Update presence indicator
          const indicator = document.getElementById('presence-indicator');
          const status = document.getElementById('status');
          const duration = document.getElementById('duration');
          
          if (data.humanDetected) {
            indicator.textContent = '👤';
            indicator.className = 'presence-indicator present';
            status.className = 'status-display human-detected';
            duration.textContent = 'Present for: ' + formatDuration(data.presenceDuration);
          } else {
            indicator.textContent = '🚫';
            indicator.className = 'presence-indicator absent';
            status.className = 'status-display no-human';
            duration.textContent = 'No presence detected';
          }
          
          status.textContent = data.status;
        })
        .catch(error => console.error('Error:', error));
    }
    
    function formatDuration(ms) {
      const seconds = Math.floor(ms / 1000);
      const minutes = Math.floor(seconds / 60);
      const hours = Math.floor(minutes / 60);
      
      if (hours > 0) {
        return hours + 'h ' + (minutes % 60) + 'm ' + (seconds % 60) + 's';
      } else if (minutes > 0) {
        return minutes + 'm ' + (seconds % 60) + 's';
      } else {
        return seconds + 's';
      }
    }
    
    function capturePhoto() {
      window.open('/capture', '_blank');
    }
    
    function startStream() {
      window.open('/stream', '_blank');
    }
    
    // Update sensor data every 1 second
    setInterval(updateSensorData, 1000);
    updateSensorData(); // Initial call
  </script>
</head>
<body>
  <div class="container">
    <h1>🏠 ESP32-CAM Human Presence Monitor</h1>
    
    <div class="sensor-panel">
      <h2>📡 LD2410B OUT Pin Mode</h2>
      <div class="presence-indicator absent" id="presence-indicator">🚫</div>
      <div class="status-display no-human" id="status">Initializing...</div>
      <div class="duration-display" id="duration">No presence detected</div>
    </div>
    
    <div class="info-box">
      <strong>📌 Simple Mode:</strong> Using LD2410B OUT pin for basic presence detection.<br>
      <strong>🔌 Wiring:</strong> LD2410B OUT → ESP32 GPIO 14, VCC → 5V, GND → GND<br>
      <strong>✅ Advantages:</strong> No UART issues, reliable detection, simple setup
    </div>
    
    <div class="camera-frame">
      <h2>📷 Camera Controls</h2>
      <button onclick="capturePhoto()">📸 Capture Photo</button>
      <button onclick="startStream()">🎥 Live Stream</button>
    </div>
    
    <div style="text-align: center; margin-top: 30px; opacity: 0.8;">
      <small>ESP32-CAM AI Thinker + LD2410B Human Presence Sensor (OUT Pin Mode)</small><br>
      <small>WiFi: Connected | Sensor: Digital OUT | 24GHz Radar Technology</small>
    </div>
  </div>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleSensorData() {
  String json = "{";
  json += "\"humanDetected\":";
  json += (humanDetected ? "true" : "false");
  json += ",";
  json += "\"presenceDuration\":" + String(presenceDuration) + ",";
  json += "\"status\":\"" + sensorStatus + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleCapture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

void handleStream() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32-CAM Live Stream</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background: #000; 
      margin: 0; 
      padding: 20px; 
      color: white; 
      text-align: center;
    }
    img { 
      max-width: 100%; 
      height: auto; 
      border: 2px solid #4ECDC4;
      border-radius: 10px;
    }
    h1 { color: #4ECDC4; }
  </style>
</head>
<body>
  <h1>📡 ESP32-CAM Live Stream</h1>
  <p>Streaming from your ESP32-CAM device</p>
  <img src="data:image/jpeg;base64," id="stream" />
  
  <script>
    function refreshImage() {
      fetch('/capture')
        .then(response => response.blob())
        .then(blob => {
          const url = URL.createObjectURL(blob);
          document.getElementById('stream').src = url;
        });
    }
    
    setInterval(refreshImage, 2000);
    refreshImage();
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}
