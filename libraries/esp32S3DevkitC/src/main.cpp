// 藤本壱氏 https://github.com/hajimef/tukurutch_esp32s3 をインポート 2022.09.28
#include <Arduino.h>
#include <TukurutchEsp.h>
#include <FastLED.h>
#include "main.h"

WebsocketsServer wsServer;

#define LED_PIN 48
#define NUM_LEDS 1
#define LEDC_BUZZER_S3 7

CRGB leds[NUM_LEDS];

void onConnect(String ip)
{
	wsServer.listen(PORT_WEBSOCKET);
	Serial.println(ip);
}

void _setup(const char* ver)
{
	Serial.begin(115200);
	initWifi(ver, false, onConnect);
	FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
	leds[0] = CRGB::Black;
	FastLED.show();
}

void _loop(void)
{
}

void esp32s3_tone(uint8_t port, int16_t freq, int16_t ms)
{
    ledcAttachPin(port, LEDC_BUZZER_S3);
    ledcWriteTone(LEDC_BUZZER_S3, freq);
    delay(ms);
    ledcWriteTone(LEDC_BUZZER_S3, 0);
}

int esp32s3_analogRead(uint8_t port, uint16_t count)
{
    pinMode(port, INPUT);
    if(count == 0) count = 1;
    int i;
    uint32_t sum = 0;
    for(i = 0; i < count; i++)
      sum += analogRead(port);
    return ((sum / count) * 825UL) / 1024;
}

void showLED(int r, int g, int b)
{
	leds[0].r = r;
	leds[0].g = g;
	leds[0].b = b;
	FastLED.show();
}
