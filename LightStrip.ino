#define FASTLED_INTERNAL
#include <ArduinoOTA.h>
#include <platforms.h>
#include <pixeltypes.h>
#include <pixelset.h>
#include <noise.h>
#include <lib8tion.h>
#include <led_sysdefs.h>
#include <hsv2rgb.h>
#include <fastspi_types.h>
#include <fastspi_ref.h>
#include <fastspi_nop.h>
#include <fastspi_dma.h>
#include <fastspi_bitbang.h>
#include <fastspi.h>
#include <fastpin.h>
#include <fastled_progmem.h>
#include <fastled_delay.h>
#include <fastled_config.h>
#include <FastLED.h>
#include <dmx.h>
#include <cpp_compat.h>
#include <controller.h>
#include <colorutils.h>
#include <colorpalettes.h>
#include <color.h>
#include <chipsets.h>
#include <bitswap.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include "Light.h"
#include "WIFI-Secret.h"

#define TOPIC "Light/Kontor/Strib/"
#define TOPIC_DEBUG "Light/Kontor/Strib/Debug"


int Brightness_Top = 0;
int Mode_Top = 10;
int Brightness_But = 0;
int Mode_But = 10;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

IPAddress ip(192, 168, 1, 213);
IPAddress gw(192, 168, 1, 1);
IPAddress mask(255, 255, 255, 0);
WiFiClient ethClient;

IPAddress MQTTServer(192, 168, 1, 4);
PubSubClient MQTTclient(ethClient);


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
	ArduinoOTA.setHostname("LightStrib");
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
		//Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
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
	PowerON(TOP, 10);
	PowerON(BUT, 10);
}



void callback(char* topic, byte* payload, unsigned int length) {
	char *Command;
	char value[20];
	int i;

	Serial.print("Message arrived - ");
	for (i = 0; (i < length) && (i < 20); i++) {
		value[i] = payload[i];
	}
	value[i] = '\0';

	if (strlen(topic) > sizeof(TOPIC)) {
		Command = &topic[sizeof(TOPIC) - 1];

		Serial.print("Command = ");
		Serial.print(Command);
		Serial.print(" , Value = ");
		Serial.println(value);

		if (!strcasecmp(Command, "brightnesstop")) {
			int b = atoi(value);
			Brightness_Top = b;
			SetBrightness(TOP, b);
		} else
		if (!strcasecmp(Command, "brightnessbut")) {
			int b = atoi(value);
			Brightness_But = b;
			SetBrightness(BUT, b);
		} else

		if (!strcasecmp(Command, "poweron")) {
			int b = atoi(value);
			Brightness_But = b;
			Brightness_Top = b;
			PowerON(BUT, b);
			PowerON(TOP, b);
		} else
		if (!strcasecmp(Command, "poweroff")) {
			Brightness_But = 0;
			Brightness_Top = 0;
			PowerOFF(BUT, 0);
			PowerOFF(TOP, 0);
		}
	}
}

void reconnect() {
	int c = 0;
	// Loop until we're reconnected
	while (!MQTTclient.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (MQTTclient.connect("KontorLightStrib")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			MQTTclient.publish(TOPIC_DEBUG, "hello world");
			// ... and resubscribe
			char s[sizeof(TOPIC) + 3];
			strcpy(s, TOPIC);
			Serial.println(s);
			strcat(s, "#");
			Serial.println(s);
			MQTTclient.subscribe(s);
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
		reconnect();
	}
	MQTTclient.loop();

	ArduinoOTA.handle();

	delay(20);
//	SetLight(Mode_Top, Brightness_Top, Mode_But, Brightness_But);
}
