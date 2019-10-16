/**
  V2: cleaner Code
  V1: Receives data from the Master and idetifies the case. Turns the LED acording the case  
*/

#include <Adafruit_NeoPixel.h>// library for LEDRing
#include <esp_now.h>
#include <WiFi.h>


#define LED_PIN    17   // Pin in ESP32
#define LED_COUNT  24   // Number of leds on the ring
#define BRIGHTNESS 5    // NeoPixel brightness, 0 (min) to 255 (max)

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800); // Declare our NeoPixel strip object:

 uint32_t strip_red = strip.Color(255,   0,   0); // Red
 uint32_t strip_green = strip.Color(0,   255,   0); // Green
 uint32_t strip_blue = strip.Color(0,   0,   255); // Blue
 uint32_t strip_color = strip.Color(0,   0,   0); // Color Variable

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
   // Serial.println("ESPNow Init Success");
  }
  else {
   // Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
     InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];

         switch(data[1]){
            case 1: 
              strip_color= strip_red  ; 
              break;
            case 2: 
              strip_color = strip_green;
              break;
            case 3: 
             strip_color = strip_blue;
              break; 
         }
  
     colorWipe(strip_color , 0 );

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("\t\tLast Packet Recv from: "); Serial.println(macStr);
  Serial.print("\t\tLast Packet Recv Data: "); Serial.print(data[1]); Serial.print("  "); Serial.println(data[2]);
}



void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_MODE_APSTA); //Set device in STA mode to begin with
   
  InitESPNow();  // Init ESPNow with a fallback logic. Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
 
  esp_now_register_recv_cb(OnDataRecv);
  strip.begin();
  strip.show();
  strip.setBrightness(5);
}

void loop() {
}

 void colorWipe(uint32_t color,int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
   }
 }
