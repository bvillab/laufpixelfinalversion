/**
 * V2RGBV2: Cleaner code RGB slave
 * v2RGB: Alternative code when using just one LED RGB instead the LEDring
  V2: cleaner Code
  V1: Receives data from the Master and idetifies the case. Turns the LED acording the case  
*/

#include <Adafruit_NeoPixel.h>// library for LEDRing
#include <esp_now.h>
#include <WiFi.h>


#define LED_PIN    17   // Pin in ESP32
#define LED_COUNT  2   // Number of leds on the ring
//#define BRIGHTNESS 5    // NeoPixel brightness, 0 (min) to 255 (max)

//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800); // Declare our NeoPixel strip object:
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);//One LED RGB


 uint32_t colorRed = pixels.Color(255, 0, 0);
 uint32_t colorGreen = pixels.Color(0, 255, 0);
 uint32_t colorBlue = pixels.Color(0, 0, 255);
  uint32_t  pixel_color = pixels.Color(0,   0,   0); // Color Variable


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
            {
              pixel_color= colorRed  ; 
               Serial.println("Entró a este caso");
            }
              break;   
            case 2: 
            {
              pixel_color= colorGreen  ; 
            }
              break;         
            case 3: 
            {
             pixel_color= colorBlue  ; 
            }
              break;             
         }
      pixels.setPixelColor(0,  pixel_color);
      pixels.show();

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("\t\tLast Packet Recv from: "); Serial.println(macStr);
  Serial.print("\t\tLast Packet Recv Data: "); Serial.print(data[1]); Serial.print("  "); Serial.println(data[2]);
}



void setup() {
  pixels.begin();
   clearLEDs();
   pixels.setPixelColor(0,pixel_color);
   pixels.setBrightness(100);
  pixels.show();
  Serial.begin(115200);

  WiFi.mode(WIFI_MODE_APSTA); //Set device in STA mode to begin with
   
  InitESPNow();  // Init ESPNow with a fallback logic. Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
 
  esp_now_register_recv_cb(OnDataRecv);
   
}

void loop() {
}

 // Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs()
{
  for (int i=0; i<LED_COUNT; i++)
  {
    pixels.setPixelColor(i, 0);
  }
}
