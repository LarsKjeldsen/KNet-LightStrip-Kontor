#if __cplusplus >= 201703L
#define register // keyword 'register' is banned with c++17
#endif

#define FASTLED_INTERNAL
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include "Light.h"
#include "WIFI-Secret.h"
#include <string.h>

#define TOPIC        "Light/Kontor/Strip/Set/#"
#define TOPIC_Status "Light/Kontor/Strip/Status"
#define TOPIC_DEBUG  "Light/Kontor/Strip/Debug"
#define INTERVAL 10000  // 10 SEC
bool LightsOn = false;
unsigned long MillisPrev;

int Brightness_Top = 0;
int Mode_Top = 0;
int Brightness_But = 0;
int Mode_But = 0;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

IPAddress ip(192, 168, 1, 213);
// IPAddress ip(192, 168, 1, 202); // Debug
IPAddress gw(192, 168, 1, 1);
IPAddress mask(255, 255, 255, 0);
WiFiClient ethClient;

IPAddress MQTTServer(192, 168, 1, 22);
PubSubClient MQTTclient(ethClient);

void callback(char* topic, byte* payload, unsigned int length) 
{
	String Value;
	String Command;
	unsigned int i;

	Serial.print("Message arrived - ");
	bool v_split = false;
	
	for (i = 0; i < length; i++) {
		if (payload[i] == ',') {
			v_split = true;
			continue;
		}
		if (v_split)
			Value += (char)(payload[i]);
		else 
			Command += (char)(payload[i]);
	}
// MQTTclient.publish(TOPIC_DEBUG, Command.c_str());
// MQTTclient.publish(TOPIC_DEBUG, Value.c_str());

	if (Command == "brightnesstop") {
		LightsOn = SetBrightness(TOP, Value.toInt());
		SendStatus();
	} 
	else
	if (Command == "brightnessbut") {
		LightsOn = SetBrightness(BUT, Value.toInt());
		SendStatus();
	} 
	else
	if (Command == "on") {
		LightsOn = PowerON(BUT, 255);
		LightsOn = PowerON(TOP, 255);
		SendStatus();
	} else
	if (Command == "off") {
		LightsOn = PowerON(BUT, 0);
		LightsOn = PowerON(TOP, 0);
		SendStatus();
	}
}


void setup() {
	Serial.begin(115200);
	Serial.println("Starting");

	SetupLeds();

	Serial.println("Initialise WIFI - ");
	WiFi.mode(WIFI_STA);
	WiFi.config(ip, gw, mask);
	WiFi.begin(ssid, password);

	Serial.println("Connecting WIFI - ");
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}

	Serial.print(F("Starting OTA - "));
	ArduinoOTA.setHostname("LightStribKontor");
	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		}
		else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			Serial.println(F("Auth Failed"));
		}
		else if (error == OTA_BEGIN_ERROR) {
			Serial.println(F("Begin Failed"));
		}
		else if (error == OTA_CONNECT_ERROR) {
			Serial.println(F("Connect Failed"));
		}
		else if (error == OTA_RECEIVE_ERROR) {
			Serial.println(F("Receive Failed"));
		}
		else if (error == OTA_END_ERROR) {
			Serial.println(F("End Failed"));
		}
	});
	ArduinoOTA.begin();

	Serial.print(F("IP address: "));
	Serial.println(WiFi.localIP());

	Serial.print(F("Starting MQTT - "));
	String IP = WiFi.localIP().toString();
	MQTTclient.setServer(MQTTServer, 1883);
	MQTTclient.setCallback(callback);
	Serial.println(F("All initialised, starting loop"));

	// Start light at 10%
	LightsOn = PowerON(TOP, 10);
	LightsOn = PowerON(BUT, 10);
	SendStatus();
}




void reconnect() {
	int c = 0;
	// Loop until we're reconnected
	while (!MQTTclient.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (MQTTclient.connect("KontorLightStrib-1")) {
			Serial.println("connected");
			Serial.println(TOPIC);
			MQTTclient.subscribe(TOPIC);
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(MQTTclient.state());
			Serial.println(" try again in 5 seconds");
			// Wait 1 seconds before retrying
			delay(1000);
			if (c++ == 10)
				ESP.restart();
		}
	}
}




void loop() 
{
	if (!MQTTclient.connected()) {
		Serial.println("MQTT reconnecting");
		reconnect();
	}
	MQTTclient.loop();

	ArduinoOTA.handle();

	if (millis() - MillisPrev > INTERVAL)
	{
		MillisPrev = millis();
		SendStatus();
	}
	delay(20);
//	SetLight(Mode_Top, Brightness_Top, Mode_But, Brightness_But);
}

void SendStatus()
{
		if (LightsOn)
			MQTTclient.publish(TOPIC_Status, "on", true);
		else
			MQTTclient.publish(TOPIC_Status, "off", true);
}