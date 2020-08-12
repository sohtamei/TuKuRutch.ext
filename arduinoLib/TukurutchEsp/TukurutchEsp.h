#ifndef TUKURUTCH_ESP
#define TUKURUTCH_ESP

#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <ArduinoWebsockets.h>
using namespace websockets;
#define ENABLE_WEBSOCKET

#define PORT_WEBSOCKET 54323

enum {
  T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
  T_C5=523, T_D5=587, T_E5=659, T_F5=698,
};

void initWifi(const char* ver, int _waitWifi, void(*connectedCB)(String localIP)=NULL);
uint8_t connectWifi(char* ssid, char*pass);
uint8_t waitWifi(void);
char* statusWifi(void);
char* scanWifi(void);

int readWifi(void);
void writeWifi(uint8_t* dp, int count);
void printlnWifi(char* mes);
void sendNotifyArduinoMode(void);
#endif // TUKURUTCH_ESP
