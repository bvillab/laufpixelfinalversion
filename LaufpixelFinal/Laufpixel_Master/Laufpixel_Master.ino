/*
 * V7: Clean the code. Erase no used functions on this code.
 * V6: If decision in order to turn on the LEDring for the MASter (this) and ESP32 Mac Address for 20 ESP32. Eliminates data[2] for sending, not useful in this project.
 v5: ESP32 MacAddress Matrix
 V4: Receive value with the # of pixel to turn on and the color
v3: Receive data from Web via WiFi and sends a specific case to a given ESP32 via ESPnow 
    It also turn on its own LEDring depending on the case. 
V2: Connect WEB-ESP master-ESP slave via WiFi and ESPnow protocol
V1: Receives Data from a Web through wifi websocket. Depending on the number received, the LEDring tunrs the color into red, blue and green.

Daniel Patiño.
Code taken from "schwimmende Pixels" and modified.
Original: 
*/



#include <esp_now.h>//ESPnow library

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <SPIFFSEditor.h>
#include <ArduinoOTA.h>
#include <FS.h>

/// Defines for OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    false // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void DisplayRefresh();

//----------ESPnow definitions
// Global copy of slave
#define NUMSLAVES 20
esp_now_peer_info_t slaves[NUMSLAVES] = {};
int SlaveCnt = 0;

#define CHANNEL_MASTER 3
#define CHANNEL_SLAVE 1
#define PRINTSCANRESULTS 0
#define DATASIZE 48
  

//----------------------------------------ESP Functions
// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");// Retry InitESPNow, add a counte and then restart?
    ESP.restart();
  }
}
//-------------MacAddress Matrix to send the color to.

   const char* ESPSlave[][6]={{"00","00","00","00","00","00"},
                       {"00","00","00","00","00","00"},//ESP32 Master(this)
                       {"3C","71","BF","84","B1","C5"},//2nd ESP
                       {"CC","50","E3","B5","CC","15"},//3rd ESP
                       {"80","7D","3A","B7","8A","95"},//4
                       {"80","7D","3A","B7","8B","B9"},//5
                       {"80","7D","3A","B7","96","85"},//6
                       {"00","00","00","00","00","00"},//7 Available
                       {"00","00","00","00","00","00"},//8 Available
                       {"00","00","00","00","00","00"},//9 Available
                       {"00","00","00","00","00","00"},//10 Available
                       {"00","00","00","00","00","00"},//11 Available
                       {"00","00","00","00","00","00"},//12 Available
                       {"00","00","00","00","00","00"},//13 Available
                       {"00","00","00","00","00","00"},//14 Available
                       {"00","00","00","00","00","00"},//15 Available
                       {"00","00","00","00","00","00"},//16 Available
                       {"00","00","00","00","00","00"},//17 Available
                       {"00","00","00","00","00","00"},//18 Available
                       {"00","00","00","00","00","00"},//19 Available
                       {"00","00","00","00","00","00"}//20 Available//Only accepts 20 ESP
                       };
                       
// Check if the slave is already paired with the master.
// If not, pair the slave with master
void manageSlave(int Addr) {
  char *ptr;
  uint8_t peer_addr1[6];

  peer_addr1[0]= strtoul(ESPSlave[Addr][0], &ptr, 16);
  peer_addr1[1]= strtoul(ESPSlave[Addr][1], &ptr, 16);
  peer_addr1[2]= strtoul(ESPSlave[Addr][2], &ptr, 16);
  peer_addr1[3]= strtoul(ESPSlave[Addr][3], &ptr, 16);
  peer_addr1[4]= strtoul(ESPSlave[Addr][4], &ptr, 16);
  peer_addr1[5]= strtoul(ESPSlave[Addr][5], &ptr, 16);

  for (int ii = 0; ii < 6; ++ii ) {
  slaves[1].peer_addr[ii] = peer_addr1[ii];
  }
   slaves[1].channel = CHANNEL_MASTER; // pick a channel
        slaves[1].encrypt = 0; // no encryption

      const esp_now_peer_info_t *peer = &slaves[1];
      const uint8_t *peer_addr = slaves[1].peer_addr;
      
      Serial.print(" Status: ");
      // check if the peer exists
      bool exists = esp_now_is_peer_exist(peer_addr1);
      if (exists) {
        // Slave already paired.
        Serial.println("Already Paired");
      } else {
        // Slave not paired, attempt pair
        esp_err_t addStatus = esp_now_add_peer(peer);
        if (addStatus == ESP_OK) {
          // Pair success
          Serial.println("Pair success");
        } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
          // How did we get so far!!
          Serial.println("ESPNOW Not Init");
        } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
          Serial.println("Add Peer - Invalid Argument");
        } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
          Serial.println("Peer list full");
        } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
          Serial.println("Out of memory");
        } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
          Serial.println("Peer Exists");
        } else {
          Serial.println("Not sure what happened");
        }
      }
}


 uint8_t data[DATASIZE]; //uint8_t
uint64_t pos=0;


// send data: a for the color number and Addr is the number of ESP32-MacAddress to send the data to.
uint8_t sendData(int a,int Addr) {
  char *ptr;
  uint8_t peer_addr2[6];

  peer_addr2[0]= strtoul(ESPSlave[Addr][0], &ptr, 16);
  peer_addr2[1]= strtoul(ESPSlave[Addr][1], &ptr, 16);
  peer_addr2[2]= strtoul(ESPSlave[Addr][2], &ptr, 16);
  peer_addr2[3]= strtoul(ESPSlave[Addr][3], &ptr, 16);
  peer_addr2[4]= strtoul(ESPSlave[Addr][4], &ptr, 16);
  peer_addr2[5]= strtoul(ESPSlave[Addr][5], &ptr, 16);

           data[1]= a;
        
      Serial.print("Sending: ");
      Serial.println(data[1]);
  
   
    esp_err_t result = esp_now_send(peer_addr2 , data, DATASIZE);
    Serial.print("Send Status: ");
    if (result == ESP_OK) {
      Serial.println("Success");
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }

    delay(50);
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



// config AP SSID
void configDeviceAP() {
  String Prefix = "ESPNOW:";
  String Mac = WiFi.macAddress();
  String SSID = Prefix + Mac;
  String Password = "123456789";
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL_SLAVE, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}


//---Defines for RGB-Ring
#define PIN 17
#define LEDS 24
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRBW + NEO_KHZ800);
void colorWipe(uint32_t c, uint8_t wait);



void noled();
bool website = 0;
bool akku = 0;
bool ip = 0;

/*
//Hochschul Wlan
const char* ssid = "HIT-FRITZBOX-7490";
const char* password = "63601430989011937932";*/

const char* ssid     = "FRITZ!Box 7590 VO";
const char* password = "91272756878874074534";

/*
//Haus Wlan
const char* ssid = "Drosselweg";
const char* password = "WGneureut2017";*/

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);


void setup(){

  Serial.begin(9600);
  Serial.println("Hello my Friend, starting ESP32 ");

  /// Verbinden von Oled Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Anzeigen von IP und Akku auf OLed Display
  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,0);             
  display.println(F("Hello my Friend, starting ESP32"));
  display.display();


  //Beginn LED-Ring
  strip.begin();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(5);


  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  }
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
      }
  }

  // Verbinden von WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {   // Solange versuchen zu verbinden bis erfolg

    delay(100);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());   //Anzeigen von Ip über Serial
  display.clearDisplay();             //Anzeigen von IP und Akku auf OLed Display
  DisplayRefresh();
  delay(2000);

  
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  //server.addHandler(&events);

  server.begin();

    //Set device in STA mode to begin with
  WiFi.mode(WIFI_MODE_APSTA);
  Serial.println("ESPNow/Multi-Slave/Master Example");
  // configure device AP mode
  
        data[1] = 0;
        data[2] = 0;
  
  configDeviceAP();
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
 // esp_now_register_recv_cb(OnDataRecv);

}
   
void loop(){
  
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    website = 1;
    DisplayRefresh();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    website = 0;
    DisplayRefresh();
    noled();
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      }
      else{
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());
      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    }
    else {        ////////////wird glaub nicht gebraucht
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }

   // ------Identifies the number of pixel to turn on
  int numPix;
  if(len==4){
    int UnitPart = data[3]-48;//converts char data to int
    int DecimalPart = data [2]-48;//converts char data to int
    numPix = (DecimalPart*10)+UnitPart;//get the number of Pixel
    Serial.print("The number of Pixel to turn on is: ");
    Serial.println(numPix);
  }else if(len==3){
    numPix = data[2]-48;
    Serial.print ("The number of Pixel to turn on is: ");
    Serial.println (numPix);
  }


  //Identifies the color 
  int farbe =  data[0]-48;//converts char data from webSocket to int
  int a,b;
  //If the number of pixel to turn on is equals 1, turns the LED from the master ESP32 (this)
 if(numPix==1){
  switch (farbe)   {
    case 0:
        {
        uint32_t co =strip.Color(0, 0, 0);//turn off
        colorWipe(co, 100);
        strip.show();
        delay(1);
         }
        break;
        case 1:
        {
        uint32_t co = strip.Color(255,   0,   0); // Red
        colorWipe(co, 100);
        strip.show();
        delay(1);
        }
        break;
        case 2:
        {
        uint32_t co = strip.Color(0,   255,   0); // Green
        colorWipe(co, 0);
        strip.show();
        delay(1);
        }
        break;
        case 3:
        {
        uint32_t co = strip.Color(0,   0,   255); // Blue
        colorWipe(co, 100);
        strip.show();
        delay(1);
        }
        break;
        case 4:
        {  
        strip.show();
        delay(1);
        }
        break;
        default:
        {
        }
        break;    
        }
//---------sends data to a given ESP32 in the ESP32-MacAddress-Matrix. 
 }else if(numPix>1 && numPix<20){
  
   switch (farbe)   { // switch-case to send the number of color 1=red,2=green,3=blue
    case 0:
    {
        a = 0;
             // Add slave as peer if it has not been added already
    manageSlave(numPix);// pair success or already paired
    sendData(a,numPix);// Send data to device
    }
      break;
    case 1:
    {
        a = 1;
     // Add slave as peer if it has not been added already
    manageSlave(numPix);// pair success or already paired
    sendData(a,numPix);// Send data to device
    }
      break;
    case 2:
    {
        a = 2;
     // Add slave as peer if it has not been added already
    manageSlave(numPix);// pair success or already paired
    sendData(a,numPix);// Send data to device
    }
      break;
    case 3:
    {
        
        a = 3;
     // Add slave as peer if it has not been added already
    manageSlave(numPix);// pair success or already paired
    sendData(a,numPix);// Send data to device
    }
      break;
    case 4:
    {
        a = 4;
             // Add slave as peer if it has not been added already
    manageSlave(numPix);// pair success or already paired
    sendData(a,numPix);// Send data to device
    }
      break;
    default:
    {
    }
       break;
   }
  }
 }
}


// Some Funktions for the LED-Ring
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
   // delay(wait);
  }
}


void noled() {
  for(int i=0; i<strip.numPixels()+1; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
    delay(1);
    }
}

void DisplayRefresh()
{
  String sakku;
  String swebsite;
  if(akku==1 ){ sakku = "connected";}
  else{ sakku = "Not connected";}
  if(website == 1 ){ swebsite = "connected";}
  else{ swebsite = "Not connected";}
  
  display.clearDisplay();             //Anzeigen von IP und Akku auf OLed Display
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,0);             
  display.println(F("IP:"));
  display.println(WiFi.localIP());
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,20);             
  display.println(F("Akku:"));
  display.print(sakku);
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,40);             
  display.println(F("Website:"));
  display.print(swebsite);
  display.display();
  delay(2000);
}
