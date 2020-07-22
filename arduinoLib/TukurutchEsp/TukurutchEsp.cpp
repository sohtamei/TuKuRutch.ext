#include <WiFi.h>
#include <AsyncUDP.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include "TukurutchEsp.h"

#define PORT  54321
#define REMOTE_PORT 54322
#define DPRINT(a) // _Serial.println(a) // for debug

char g_ssid[4+32+1] = {0};		// length(4)+SSID
//char g_pass[66] = {0};

WiFiServer server(PORT);
WiFiClient client;
AsyncUDP udp;
WiFiUDP remoteUdp;
Preferences preferences;
char buf[256];

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

void initWifi(const char* ver, int _waitWifi, void(*connectedCB)(String localIP))
{
	_connectedCB = connectedCB;
	mVersion = ver;
	preferences.begin("nvs.net80211", true);
	preferences.getBytes("sta.ssid", g_ssid, sizeof(g_ssid));
//	preferences.getBytes("sta.pswd", g_pass, sizeof(g_pass));
	Serial.print("Connecting to ");  Serial.println(g_ssid+4);

	//WiFi.mode(WIFI_STA);
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
	WiFi.begin(ssid, pass);
	connection_status = CONNECTION_CONNECTING;
	connection_start = millis();
	return waitWifi();
}

uint8_t waitWifi(void)
{
	for(int i=0;i<80;i++) {
		delay(100);
		if(WiFi.status()==WL_CONNECTED) break;
	}
	return WiFi.status();
}

char* statusWifi(void)
{
	preferences.getString("ssid", g_ssid, sizeof(g_ssid));
	memset(buf, 0, sizeof(buf));
    
	if(WiFi.status() == WL_CONNECTED) {
		IPAddress ip = WiFi.localIP();
		snprintf(buf,sizeof(buf)-1,"%d\t%s\t%d.%d.%d.%d", WiFi.status(), g_ssid+4, ip[0],ip[1],ip[2],ip[3]);
	} else {
		snprintf(buf,sizeof(buf)-1,"%d\t%s", WiFi.status(), g_ssid+4);
	}
	return buf;
}

char* scanWifi(void)
{
	memset(buf, 0, sizeof(buf));
    
	int n = WiFi.scanNetworks();
	for(int i = 0; i < n; i++) {
		if(i == 0) {
			snprintf(buf, sizeof(buf)-1, "%s", WiFi.SSID(i).c_str());
		} else {
			int ofs = strlen(buf);
			snprintf(buf+ofs, sizeof(buf)-1-ofs, "\t%s", WiFi.SSID(i).c_str());
		}
	}
	return buf;
}

// connect	  : DISCONNECTED->(NO_SSID_AVAIL)->IDLE->CONNECTED
// disconnect     : DISCONNECTED->(NO_SSID_AVAIL)->IDLE->CONNECTED->DISCONNECTED
// no SSID	  : DISCONNECTED->NO_SSID_AVAIL (timeout)
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

int WifiRemote::checkRemoteKey(void) {
	if(!initialized) {
		remoteUdp.begin(REMOTE_PORT);
		initialized = true;
	}
	return keys;
}

void WifiRemote::updateRemote(void) {
	if(!initialized) return;
	int rlen = remoteUdp.parsePacket();
	if(rlen>=5) {
		uint8_t buf[16];
		if(rlen >= sizeof(buf)) rlen = sizeof(buf);
		remoteUdp.read(buf, rlen);
		keys = buf[0];
		x = buf[1]|(buf[2]<<8);
		y = buf[3]|(buf[4]<<8);
	//	snprintf((char*)buf,sizeof(buf),\"%d,%d,%d\",keys,x,y);
	//	Serial.println((char*)buf);
	}
};
