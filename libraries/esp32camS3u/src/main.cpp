/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>

#include <TukurutchEsp.h>
#include "main.h"
#include <TukurutchEspCamera.h>

#include <TukurutchEspCamera.cpp.h>

WebsocketsServer wsServer;

void onConnect(String ip)
{
	wsServer.listen(PORT_WEBSOCKET);
	espcamera_onConnect();
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	espcamera_setup();

	initWifi(ver, false, onConnect);
}

void _loop(void)
{
	espcamera_loop();
	delay(40);
}
