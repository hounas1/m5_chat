/**
 * @file inputText.ino
 * @author SeanKwok (shaoxiang@m5stack.com)
 * @brief M5Cardputer input text test
 * @version 0.1
 * @date 2023-10-13
 *
 *
 * @Hardwares: M5Cardputer
 * @Platform Version: Arduino M5Stack Board Manager v2.0.7
 * @Dependent Library:
 * M5GFX: https://github.com/m5stack/M5GFX
 * M5Unified: https://github.com/m5stack/M5Unified
 */

 #include "M5Cardputer.h"
 #include "M5GFX.h"

 #include "WiFi.h"
 #include "esp_now.h"

//uint8_t receiverMAC[] = {0x34, 0xB7, 0xDA, 0x56, 0xCC, 0xF8}; // minu
uint8_t receiverMAC[] = {0x34, 0xB7, 0xDA, 0x56, 0xD6, 0x00}; // karl
 
uint8_t value_sent = 0;

typedef struct struct_message {
    int id;
    float value;
} struct_message;

struct_message myData;
struct_message incomingData;

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    M5.Log.printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Value sent: %.2f", myData.value);
}

void onReceive(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
    M5Cardputer.Speaker.tone(2000, 100);
    memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("Received ID: %d, Value : %.2f", incomingData.id, incomingData.value);
}

void setup() {
    auto cfg = M5.config();
    //M5.begin(cfg);
    M5Cardputer.begin(cfg, true);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        M5.Log.println("ESP-NOW init failed");
        return;
    }

    esp_now_register_send_cb(onSent);
    esp_now_register_recv_cb(onReceive);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_add_peer(&peerInfo)) {
        M5.Log.println("Peer added");
    }

    myData.id = 1;   //minu
    //myData.id = 2; //karl

}

void loop() {
    myData.value = random(0, 100) / 1.0;
    value_sent = myData.value;
    esp_now_send(receiverMAC, (uint8_t *) &myData, sizeof(myData));
    delay(5000);
}