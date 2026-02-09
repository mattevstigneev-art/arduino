#include <esp_camera.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

const char* ROBOT_SSID = "AgriBot_Node_01";
const char* ROBOT_PASS = "agribot2024";

WebServer server(80);
WiFiServer tcpServer(1234);
bool isRobotMoving = false;
bool dataStreamActive = false;
int scanCount = 0;
String robotStatus = "IDLE";
String jsonData = "";

camera_config_t config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_GRAYSCALE,
    .frame_size = FRAMESIZE_96X96,
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM
};

void setup() {
    Serial.begin(115200);
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) return;
    
    WiFi.softAP(ROBOT_SSID, ROBOT_PASS);
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    
    server.on("/", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'><title>AgriBot Control</title>";
        html += "<style>body{font-family:Arial;text-align:center;margin:40px;}";
        html += ".btn{padding:15px 30px;margin:10px;font-size:18px;border:none;border-radius:10px;cursor:pointer;}";
        html += ".start{background:#4CAF50;color:white;}.stop{background:#f44336;color:white;}";
        html += ".scan{background:#2196F3;color:white;}.status{padding:20px;background:#f5f5f5;border-radius:10px;margin:20px;}";
        html += "</style></head><body><div style='max-width:800px;margin:0 auto;'>";
        html += "<h1>AgriBot Control</h1><div class='status'>";
        html += "<h3>Status: " + robotStatus + "</h3>";
        html += "<p>Scans: " + String(scanCount) + "</p>";
        html += "<p>IP: " + WiFi.softAPIP().toString() + "</p></div>";
        html += "<button class='btn start' onclick=\"fetch('/command?cmd=start')\">Start</button>";
        html += "<button class='btn stop' onclick=\"fetch('/command?cmd=stop')\">Stop</button><br>";
        html += "<button class='btn scan' onclick=\"fetch('/command?cmd=scan')\">Scan</button>";
        html += "<button class='btn scan' onclick=\"fetch('/command?cmd=stream_on')\">Stream On</button>";
        html += "<button class='btn stop' onclick=\"fetch('/command?cmd=stream_off')\">Stream Off</button>";
        html += "<div id='results' style='margin-top:30px;'></div></div></body></html>";
        server.send(200, "text/html", html);
    });
    
    server.on("/command", HTTP_GET, []() {
        String cmd = server.arg("cmd");
        String response = "Command: " + cmd + "\nTime: " + String(millis() / 1000) + "s\n";
        
        if (cmd == "start") {
            isRobotMoving = true;
            robotStatus = "MOVING";
            response += "Robot started\n";
        }
        else if (cmd == "stop") {
            isRobotMoving = false;
            robotStatus = "IDLE";
            response += "Robot stopped\n";
        }
        else if (cmd == "scan") {
            scanCount++;
            response += "Scan #" + String(scanCount) + ":\n";
