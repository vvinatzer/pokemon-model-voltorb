#include <WiFi.h>
#include <esp_now.h>

// Motor pins
#define AIN1 22
#define AIN2 23
#define BIN1 19
#define BIN2 21

// PWM setup
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// Motor control variables
volatile int motorASpeed = 0;
volatile int motorBSpeed = 0;

// Structure of incoming data
typedef struct struct_message {
  int motorA;
  int motorB;
} struct_message;

struct_message incomingData;

// Motor control task
void motorTask(void *pvParameters) {
  while (true) {
    // Apply motor A
    if (motorASpeed > 0) {
      ledcWrite(PWM_CHANNEL_A, motorASpeed);
      digitalWrite(AIN2, LOW);
    } else if (motorASpeed < 0) {
      ledcWrite(PWM_CHANNEL_A, abs(motorASpeed));
      digitalWrite(AIN1, LOW);
    } else {
      ledcWrite(PWM_CHANNEL_A, 0);
    }

    // Apply motor B
    if (motorBSpeed > 0) {
      ledcWrite(PWM_CHANNEL_B, motorBSpeed);
      digitalWrite(BIN2, LOW);
    } else if (motorBSpeed < 0) {
      ledcWrite(PWM_CHANNEL_B, abs(motorBSpeed));
      digitalWrite(BIN1, LOW);
    } else {
      ledcWrite(PWM_CHANNEL_B, 0);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Callback
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  motorASpeed = incomingData.motorA;
  motorBSpeed = incomingData.motorB;
  Serial.printf("Received: A=%d, B=%d\n", motorASpeed, motorBSpeed);
}

void setup() {
  Serial.begin(115200);

  // Setup motor pins
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // PWM setup
  ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(AIN1, PWM_CHANNEL_A);

  ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BIN1, PWM_CHANNEL_B);

  // WiFi in station mode
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  // Start motorTask
  xTaskCreatePinnedToCore(
    motorTask,      // task function
    "Motor Task",   // task name
    4096,           // stack size
    NULL,           // params
    1,              // priority
    NULL,           // task handle
    1               // run on core 1
  );
}

void loop() {
}

