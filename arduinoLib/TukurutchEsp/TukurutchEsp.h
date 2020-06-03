#ifndef TUKURUTCH_ESP
#define TUKURUTCH_ESP

void initWifi(const char* ver, void(*connectedCB)(String localIP)=NULL);
uint8_t connectWifi(char* ssid, char*pass);
uint8_t waitWifi(void);
char* statusWifi(void);
char* scanWifi(void);

int readWifi(void);
void writeWifi(uint8_t* dp, int count);
void printlnWifi(char* mes);
void sendNotifyArduinoMode(void);

class WifiRemote {
public:
  uint8_t  keys;  // BUTTON_xx
  int16_t  x;
  int16_t  y;

  WifiRemote() {
    initialized = false;
    keys = 0;
  }
  int checkRemoteKey(void);
  int isRemoteKey(uint8_t key) {
    return (keys==key);
  }
  void updateRemote(void);
private:
  uint8_t initialized;
};

#define REMOTE_ENABLE	// for robot_pcmode.ino.template
#endif // TUKURUTCH_ESP
