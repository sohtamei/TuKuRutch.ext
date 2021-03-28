#include <WiFi.h>
#include <AsyncUDP.h>
#include <Preferences.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "TukurutchEsp.h"

#define PORT  54321
#define DPRINT(a) // _Serial.println(a) // for debug

char g_ssid[4+32+1] = {0};		// length(4)+SSID

WiFiServer server(PORT);
WiFiClient client;
AsyncUDP udp;
Preferences preferences;
char strBuf[256];

enum {
    CONNECTION_NONE = 0,
    CONNECTION_CONNECTING,
    CONNECTION_WIFI,
    CONNECTION_TCP,
};
uint8_t connection_status = CONNECTION_NONE;
uint32_t connection_start = 0;
const char* mVersion = NULL;
void(*_connectedCB)(String localIP) = NULL;

static uint8_t waitWifi(void)
{
	for(int i=0;i<80;i++) {
		delay(100);
		if(WiFi.status()==WL_CONNECTED) break;
	}
	return WiFi.status();
}

void initWifi(const char* ver, int _waitWifi, void(*connectedCB)(String localIP))
{
	_connectedCB = connectedCB;
	mVersion = ver;
	preferences.begin("nvs.net80211", true);
	preferences.getBytes("sta.ssid", g_ssid, sizeof(g_ssid));
//	preferences.getBytes("sta.pswd", g_pass, sizeof(g_pass));
	Serial.print("Connecting to ");  Serial.println(g_ssid+4);

	WiFi.mode(WIFI_STA);
	if(g_ssid[4]) {
		WiFi.begin();
		connection_status = CONNECTION_CONNECTING;
		connection_start = millis();
		if(_waitWifi) {
			if(waitWifi() == WL_CONNECTED) {
				if(_connectedCB) _connectedCB(WiFi.localIP().toString());
				DPRINT(WiFi.localIP());
				connection_status = CONNECTION_WIFI;
			} else {
				WiFi.disconnect();
				connection_status = CONNECTION_NONE;
			}
		}
	}
}

uint8_t connectWifi(char* ssid, char*pass)
{
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass);
	connection_status = CONNECTION_CONNECTING;
	connection_start = millis();
	return waitWifi();
}

char* statusWifi(void)
{
	preferences.getBytes("sta.ssid", g_ssid, sizeof(g_ssid));
	memset(strBuf, 0, sizeof(strBuf));
    
	if(WiFi.status() == WL_CONNECTED) {
		IPAddress ip = WiFi.localIP();
		snprintf(strBuf,sizeof(strBuf)-1,"%d\t%s\t%d.%d.%d.%d", WiFi.status(), g_ssid+4, ip[0],ip[1],ip[2],ip[3]);
	} else {
		snprintf(strBuf,sizeof(strBuf)-1,"%d\t%s", WiFi.status(), g_ssid+4);
	}
	return strBuf;
}

char* scanWifi(void)
{
	memset(strBuf, 0, sizeof(strBuf));
    
	int n = WiFi.scanNetworks();
	for(int i = 0; i < n; i++) {
		if(i == 0) {
			snprintf(strBuf, sizeof(strBuf)-1, "%s", WiFi.SSID(i).c_str());
		} else {
			int ofs = strlen(strBuf);
			snprintf(strBuf+ofs, sizeof(strBuf)-1-ofs, "\t%s", WiFi.SSID(i).c_str());
		}
	}
	return strBuf;
}

// connect		  : DISCONNECTED->(NO_SSID_AVAIL)->IDLE->CONNECTED
// disconnect     : DISCONNECTED->(NO_SSID_AVAIL)->IDLE->CONNECTED->DISCONNECTED
// no SSID		  : DISCONNECTED->NO_SSID_AVAIL (timeout)
// password error : DISCONNECTED (timeout)
uint32_t last_udp;
int readWifi(void)
{
	if(WiFi.status() != WL_CONNECTED) {
		switch(connection_status) {
		case CONNECTION_TCP:
		case CONNECTION_WIFI:
			WiFi.disconnect();
			connection_status = CONNECTION_NONE;
			break;
		case CONNECTION_CONNECTING:
			if(millis() - connection_start > 8000) {
				WiFi.disconnect();
				connection_status = CONNECTION_NONE;
			}
			break;
		}
		return -1;
	}
    
	uint32_t cur = millis();
	if(cur - last_udp > 2000) {
		last_udp = cur;
	  //  udp.broadcastTo(mVersion, PORT);
		uint32_t adrs = WiFi.localIP();
		uint32_t subnet = WiFi.subnetMask();
		udp.writeTo((uint8_t*)mVersion, strlen(mVersion), IPAddress(adrs|~subnet), PORT);
	}

	switch(connection_status) {
	case CONNECTION_NONE:
	case CONNECTION_CONNECTING:
		server.begin();
		if(_connectedCB) _connectedCB(WiFi.localIP().toString());
		DPRINT(WiFi.localIP());
		connection_status = CONNECTION_WIFI;

	case CONNECTION_WIFI:
		client = server.available();
		if(!client) {
			return -1;
		}
		DPRINT("connected");
		connection_status = CONNECTION_TCP;

	case CONNECTION_TCP:
		if(!client.connected()) {
			DPRINT("disconnected");
			client.stop();
			connection_status = CONNECTION_WIFI;
			return -1;
		}
		if(client.available()<=0) {
			return -1;
		}
		return client.read();
	}
}

void sendNotifyArduinoMode(void)
{
	uint32_t cur = millis();
	if(cur - last_udp > 2000) {
		last_udp = cur;
	  //  udp.broadcastTo(mVersion, PORT);
		uint32_t adrs = WiFi.localIP();
		uint32_t subnet = WiFi.subnetMask();
		char buf[32] = {0};
		snprintf(buf, sizeof(buf)-1, "%s(ArduinoMode)", mVersion);
		udp.writeTo((uint8_t*)buf, strlen(buf), IPAddress(adrs|~subnet), PORT);
	}
}

void writeWifi(uint8_t* dp, int count)
{
	client.write(dp, count);
}

void printlnWifi(char* mes)
{
	client.println(mes);
}

#define numof(a) (sizeof(a)/sizeof((a)[0]))
const struct {uint8_t port; uint8_t ch;} adc1Table[] = {{36, 0}, {37, 1}, {38, 2}, {39, 3}, {32, 4}, {33, 5}, {34, 6}, {35, 7}};
esp_adc_cal_characteristics_t adc_chars;
uint16_t getAdc1(uint8_t port, uint16_t count)
{
  int i;
  if(count == 0) count = 1;

  int ch = -1;
  for(i = 0; i < numof(adc1Table); i++) {
    if(adc1Table[i].port == port) {
      ch = adc1Table[i].ch;
      break;
    }
  }
  if(ch == -1) return 0;

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten((adc1_channel_t)ch, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100/*VREF*/, &adc_chars);

  uint32_t sum = 0;
  for(i = 0; i < count; i++)
    sum += adc1_get_raw((adc1_channel_t)ch);

  return esp_adc_cal_raw_to_voltage(sum/count, &adc_chars);
}

