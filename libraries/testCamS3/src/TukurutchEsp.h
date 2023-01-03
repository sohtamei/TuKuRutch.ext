#ifndef TUKURUTCH_ESP
#define TUKURUTCH_ESP

#include <ArduinoWebsockets.h>
using namespace websockets;
#define PORT_WEBSOCKET 54323

extern char strBuf[256];

enum {
  T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
  T_C5=523, T_D5=587, T_E5=659, T_F5=698,
};

void initWifi(const char* ver, int _waitWifi, void(*connectedCB)(String localIP)=NULL);
uint8_t connectWifi(char* ssid, char*pass);
char* statusWifi(void);
char* scanWifi(void);

int readWifi(void);

uint16_t getAdc1(uint8_t port, uint16_t count);
#endif // TUKURUTCH_ESP
