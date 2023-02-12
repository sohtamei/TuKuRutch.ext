// 藤本壱氏 https://github.com/hajimef/tukurutch_esp32s3 をインポート 2022.09.28
#ifndef main_h
#define main_h

extern WebsocketsServer wsServer;
void _setup(const char* ver);
void _loop(void);
void esp32s3_tone(uint8_t port, int16_t freq, int16_t ms);
int esp32s3_analogRead(uint8_t port, uint16_t count);
void showLED(int r, int g, int h);
#endif  // main_h
