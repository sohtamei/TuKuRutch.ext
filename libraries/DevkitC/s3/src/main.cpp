/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <TukurutchEsp.h>
#include "main.h"

WebsocketsServer wsServer;

void onConnect(String ip)
{
	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
}
