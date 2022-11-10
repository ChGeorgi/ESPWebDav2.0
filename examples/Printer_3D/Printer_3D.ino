// Using the WebDAV server with Rigidbot 3D printer.
// Printer controller is a variation of Rambo running Marlin firmware

#include <ESP8266WiFi.h>
#include <ESPWebDAV.h>
#include <WiFiManager.h>


#ifdef Serial.println
  #undef DBG_INIT
  #undef Serial.print
  #undef Serial.println
  // #define DBG_INIT(...)    { Serial1.begin(__VA_ARGS__); }
  // #define Serial.print(...)    { Serial1.print(__VA_ARGS__); }
  // #define Serial.println(...)  { Serial1.println(__VA_ARGS__); }
  #define DBG_INIT(...)   {}
  #define Serial.print(...)     {}
  #define Serial.println(...)   {}
#endif

// LED is connected to GPIO2 on this board
#define INIT_LED      {pinMode(2, OUTPUT);}
#define LED_ON        {digitalWrite(2, LOW);}
#define LED_OFF       {digitalWrite(2, HIGH);}

#define HOSTNAME    "3DPrinter"
#define SERVER_PORT   80
#define SPI_BLOCKOUT_PERIOD 20000UL

#define SD_CS   4
#define MISO    12
#define MOSI    13
#define SCLK    14
#define CS_SENSE  5


ESPWebDAV dav1;
String statusMessage;
bool initFailed = false;

volatile long spiBlockoutTime = 0;
bool weHaveBus = false;
void ICACHE_RAM_ATTR int1();


 void int1() {if(!weHaveBus)spiBlockoutTime = millis() + SPI_BLOCKOUT_PERIOD;}
// ------------------------
void setup() {
// ------------------------
  // ----- GPIO -------
  // Detect when other master uses SPI bus
  pinMode(CS_SENSE, INPUT);
  attachInterrupt(CS_SENSE,int1, FALLING);
  
  Serial.begin(115200);
  Serial.println("");
  INIT_LED;
  blink();
  
  // wait for other master to assert SPI bus first
  delay(SPI_BLOCKOUT_PERIOD);
WiFi.hostname(HOSTNAME);
  // ----- WIFI -------
  // Set hostname first
 WiFiManager wifiManager;
  //wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  // Wait for connection
  while(WiFi.status() != WL_CONNECTED) {
    blink();
    Serial.print(".");
  }

  Serial.println("");
  //Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Serial.print("RSSI: "); Serial.println(WiFi.RSSI());
  Serial.print("Mode: "); Serial.println(WiFi.getPhyMode());


  // ----- SD Card and Server -------
  // Check to see if other master is using the SPI bus
  while(millis() < spiBlockoutTime)
    blink();
  
  takeBusControl();
  
  // start the SD DAV server
  if(!dav1.init(SD_CS, SERVER_PORT))    {
    statusMessage = "Failed to initialize SD Card";
    Serial.print("ERROR: "); Serial.println(statusMessage);
    // indicate error on LED
    errorBlink();
    initFailed = true;
  }
  else
    blink();

  relenquishBusControl();
  Serial.println("WebDAV server started");
}



// ------------------------
void loop() {
// ------------------------
  if(millis() < spiBlockoutTime)
    blink();

  // do it only if there is a need to read FS
  if(dav1.isClientWaiting())  {
    if(initFailed)
      return dav1.rejectClient(statusMessage);
    
    // has other master been using the bus in last few seconds
    if(millis() < spiBlockoutTime)
      return dav1.rejectClient("Marlin is reading from SD card");
    
    // a client is waiting and FS is ready and other SPI master is not using the bus
    takeBusControl();
    dav1.handleClient();
    relenquishBusControl();
  }
}



// ------------------------
void takeBusControl() {
// ------------------------
  weHaveBus = true;
  LED_ON;
  pinMode(MISO, SPECIAL); 
  pinMode(MOSI, SPECIAL); 
  pinMode(SCLK, SPECIAL); 
  pinMode(SD_CS, OUTPUT);
}



// ------------------------
void relenquishBusControl() {
// ------------------------
  pinMode(MISO, INPUT); 
  pinMode(MOSI, INPUT); 
  pinMode(SCLK, INPUT); 
  pinMode(SD_CS, INPUT);
  LED_OFF;
  weHaveBus = false;
}




// ------------------------
void blink()  {
// ------------------------
  LED_ON; 
  delay(100); 
  LED_OFF; 
  delay(400);
}



// ------------------------
void errorBlink() {
// ------------------------
  for(int i = 0; i < 100; i++)  {
    LED_ON; 
    delay(50); 
    LED_OFF; 
    delay(50);
  }
}


