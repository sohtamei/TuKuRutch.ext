#ifndef TUKURUTCH_ESP
#define TUKURUTCH_ESP

#define PORT_WEBSOCKET 54323

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
