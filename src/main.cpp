/*
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

 M5Canvas canvas(&M5Cardputer.Display);
 String data = "> ";

//uint8_t receiverMAC[] = {0x34, 0xB7, 0xDA, 0x56, 0xCC, 0xF8}; // minu
uint8_t receiverMAC[] = {0x34, 0xB7, 0xDA, 0x56, 0xD6, 0x00}; // karl
 
uint8_t value_sent = 0;

#define MAX_LEN 64

typedef struct struct_message {
    int id;
    char value[MAX_LEN];
} struct_message;

struct_message myData;
struct_message incomingData;


void onReceive(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
    //M5Cardputer.Speaker.tone(1000, 100);
    memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
    canvas.print(incomingData.id);
    canvas.print(": ");
    canvas.print(incomingData.value);
    canvas.println();
    canvas.pushSprite(4, 4);
}

void setup() {
    auto cfg = M5.config();
    //M5.begin(cfg);
    M5Cardputer.begin(cfg, true);

    //ESP_NOW SETUP
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        M5.Log.println("ESP-NOW init failed");
        return;
    }

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


    //DISPLAY SETUP
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextSize(0.5);
    M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(),
                                 M5Cardputer.Display.height() - 28, GREEN);
    M5Cardputer.Display.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);

    M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 4,
                                 M5Cardputer.Display.width(), 4, GREEN);

    canvas.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);
    canvas.setTextSize(0.5);
    canvas.createSprite(M5Cardputer.Display.width() - 8,
                        M5Cardputer.Display.height() - 36);
    canvas.setTextScroll(true);
    canvas.println("Press Key and Enter to Input Text");
    canvas.pushSprite(4, 4);
    M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);

}

void loop() {
    M5Cardputer.update();
    if(M5Cardputer.Keyboard.isChange()){
        if(M5Cardputer.Keyboard.isPressed()){
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

            for (auto i : status.word){
                data += i;
            }
            
            if (status.del){
                if(data.length() > 2){
                    data.remove(data.length() - 1);
                }
            }

            if(status.enter) {
                data.remove(0, 2);
                strncpy(myData.value, data.c_str(), MAX_LEN - 1);
                myData.value[MAX_LEN - 1] = '/0';

                canvas.print(myData.id);
                canvas.print(": ");
                canvas.print(data);
                canvas.println();
                canvas.pushSprite(4, 4);

                esp_now_send(receiverMAC, (uint8_t *) &myData, sizeof(myData));

                data = "> ";
                myData.value[0] = '\0';
            }

            M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                M5Cardputer.Display.width(), 25, BLACK);

            M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);
        }
    }
}