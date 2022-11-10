/* Using the WebDAV server
	From windows - 
		Run: \\HOSTNAME\DavWWWRoot
		or Map Network Drive -> Connect to a Website
*/

#include <ESP8266WiFi.h>
#include <ESPWebDAV.h>
#include <WiFiManager.h>

#define HOSTNAME	"ESPWebDAV"
#define SERVER_PORT	80
#define SD_CS		15



ESPWebDAV dav;
String statusMessage;
bool initFailed = false;


// ------------------------
void setup() {
// ------------------------
	Serial.begin(115200);
	WiFi.hostname(HOSTNAME);
	  // ----- WIFI -------
  // Set hostname first
 WiFiManager wifiManager;
  //wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();


	
	Serial.println("");

	// Wait for connection
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to "); Serial.println(ssid);
	Serial.print ("IP address: "); Serial.println(WiFi.localIP());
	Serial.print ("RSSI: "); Serial.println(WiFi.RSSI());
	Serial.print ("Mode: "); Serial.println(WiFi.getPhyMode());
	
	// start the SD DAV server
	if(!dav.init(SD_CS, SPI_FULL_SPEED, SERVER_PORT))		{
		statusMessage = "Failed to initialize SD Card";
		Serial.print("ERROR: "); Serial.println(statusMessage);
		initFailed = true;
	}

	Serial.println("WebDAV server started");
}



// ------------------------
void loop() {
// ------------------------
	if(dav.isClientWaiting())	{
		if(initFailed)
			return dav.rejectClient(statusMessage);

		// call handle if server was initialized properly
		dav.handleClient();
	}
}


