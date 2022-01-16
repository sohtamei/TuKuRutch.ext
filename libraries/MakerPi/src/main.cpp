/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include "main.h"

void onConnect(String ip)
{
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
}

void _loop(void)
{
}
