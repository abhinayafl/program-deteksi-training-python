#include "esp_camera.h"
#include <WiFi.h>
#include "FS.h"
#include "SPIFFS.h"
#include <WebServer.h>



// Konfigurasi pin kamera
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

// Ganti sesuai jaringan Anda
const char* ssid = "wifi-iot";
const char* password = "password-iot";

WebServer server(80);
#define BUTTON_PIN 13
#define BUZZER 12
unsigned long lastAutoCapture = 0;
const unsigned long captureInterval = 15000;  // setiap 15 detik


void debug(String message, int row = 0, int clear = 1) {
  Serial.println(message);
  Serial.flush();
}



void startCamera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 20;  // Kualitas lebih rendah untuk hemat memori
  config.fb_count = 1;        // Double frame buffer

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera error: %s\n", esp_err_to_name(err));
    Serial.println("Restarting ESP32...");
    delay(1000);
    ESP.restart();
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
s->set_vflip(s, 1);   // Vertical flip
s->set_hmirror(s, 1); // Horizontal mirror



  Serial.println("[✓] Kamera berhasil diinisialisasi");
}

void takePhotoAndSave() {
  Serial.printf("Free heap before capture: %u bytes\n", ESP.getFreeHeap());
  camera_fb_t* fb = esp_camera_fb_get();
  fb = esp_camera_fb_get();
  fb = esp_camera_fb_get();
  fb = esp_camera_fb_get();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[!] Gagal mengambil frame kamera");
    Serial.println("Mencoba inisialisasi ulang kamera...");
    esp_camera_deinit();
    startCamera();
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[!] Gagal lagi, periksa koneksi kamera atau daya");
      return;
    }
  }

  File file = SPIFFS.open("/photo.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("[!] Gagal menyimpan ke SPIFFS");
    esp_camera_fb_return(fb);
    return;
  }

  file.write(fb->buf, fb->len);
  file.close();
  esp_camera_fb_return(fb);
  Serial.println("[✓] Foto disimpan sebagai /photo.jpg");
}

void handleRoot() {
  File file = SPIFFS.open("/photo.jpg", "r");
  if (!file) {
    server.send(404, "text/plain", "Foto tidak ditemukan");
    return;
  }

  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(file.size()));
  server.streamFile(file, "image/jpeg");
  file.close();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);



  if (!SPIFFS.begin(true)) {
    Serial.println("[!] SPIFFS gagal dimulai");
    return;
  }
  Serial.printf("[✓] Total SPIFFS: %u bytes, Used: %u bytes\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("[✓] Terhubung ke WiFi, IP: " + WiFi.localIP().toString());
  debug("Terhubung ke WiFi");
  for (int a = 0; a < 10; a++) {
    debug(WiFi.localIP().toString() + " ------");
    delay(1000);
  }

  startCamera();

  server.on("/", handleRoot);

  server.begin();
  Serial.println("[✓] Web server dimulai");
  takePhotoAndSave();
}

void loop() {
  server.handleClient();
  int kondisi = 0;

  if (Serial.available()) {
    String data = Serial.readString();
    data.trim();  // Menghapus spasi di awal dan akhir, termasuk '\r' dan '\n'

    Serial.println(data);
    if (data == "0") {
      takePhotoAndSave();

      debug("Gambar Tersimpan");

      delay(1000);
    }
  }

  //else {
  //delay(500);
  //debug(WiFi.localIP().toString());
  //}



  // if (millis() - lastAutoCapture > captureInterval) {
  //   takePhotoAndSave();
  //   lastAutoCapture = millis();
  // }
}