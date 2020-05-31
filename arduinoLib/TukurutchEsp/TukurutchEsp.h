void initWifi(const char* ver, void(*connectedCB)(String localIP)=NULL);
uint8_t connectWifi(char* ssid, char*pass);
uint8_t waitWifi(void);
char* statusWifi(void);
char* scanWifi(void);

int readWifi(void);
void writeWifi(uint8_t* dp, int count);
void printlnWifi(char* mes);
