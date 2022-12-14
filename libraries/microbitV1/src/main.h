#ifndef main_h
#define main_h

void _setup(const char* ver);
void _loop(void);

void _displayText(char* text);
void _displayLed(uint32_t bitmap);
void _getSendData(void);
int _getTilt(uint8_t xy);

void _RadioEnable(int group, int freq, int power);
void _RadioSend(uint8_t* buf, int size);
void _RadioRecv(void);

// robot_pcmode.ino
void sendBin(uint8_t* buf, uint8_t num);
void callOK();
void bleStop();

#endif  // main_h
