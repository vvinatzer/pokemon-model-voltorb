#include <WiFi.h>
#include <esp_now.h>

// Joystick pins
const int JOY_X = 32;
const int JOY_Y = 33;

// Structure of outgoing data
typedef struct struct_message {
  int motorASpeed;
  int motorBSpeed;
} struct_message;

struct_message outgoingData;

// Receiver MAC address
uint8_t receiverMac[] = {0x78, 0x42, 0x1C, 0x65, 0x3F, 0x20};

// Send callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);

  // Init Wi-Fi in STA mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Read joystick values
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  // Map joystick to motor speeds
  outgoingData.motorASpeed = map(joyY + joyX, 0, 4095 * 2, -255, 255);
  outgoingData.motorBSpeed = map(joyY - joyX, 0, 4095 * 2, -255, 255);

  // Send data
  esp_now_send(receiverMac, (uint8_t *)&outgoingData, sizeof(outgoingData));

  delay(50);
}
