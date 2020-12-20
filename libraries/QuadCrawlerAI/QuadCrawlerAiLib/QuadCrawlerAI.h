#ifndef quadCrawler_h
#define quadCrawler_h

#include <stdint.h>

// ポート定義
#define P_IRRX	13

//動作速度定義
enum {
  quadCrawler_sslow = 2000,
  quadCrawler_slow  = 1000,
  quadCrawler_typical = 500,
  quadCrawler_fast  = 200,
  quadCrawler_high  = 100,
};

//制御ステート定義
enum {
  COM_STOP = 0,

  // repeat
  COM_FW,
  COM_CW,
  COM_CCW,
  COM_RW,
  COM_RIGHT,
  COM_LEFT,

  // normal
  COM_ALL_UP,
  COM_ALL_DOWN,
  COM_T_DOWN,
  COM_H_DOWN,
  COM_L_DOWN,
  COM_R_DOWN,

  // repeat
  COM_T_UPDOWN,
  COM_L_R_UP,
  COM_ALL_UPDOWN,

  COM_NEUTRAL,
  COM_POSE,
};

void quadCrawler_Walk(                          // ロボット動作、指定した動作を継続する。
        uint16_t speed,                         //   quadCrawler_xx
        uint8_t com);                           //   stop, 他
void quadCrawler_setSpeed(uint16_t speed);      // ロボット動作の速度を更新する。
void quadCrawler_setSpeed(uint16_t speed, int16_t x, int16_t y);

enum {
  FRONT_R   = 0,
  REAR_R    = 1,
  FRONT_L   = 2,
  REAR_L    = 3,
};
void quadCrawler_setPose1(                      // 指定した足の上下、前後の姿勢を設定する。
		uint8_t id,                             //   FRONT_R, 他
		int8_t knee,                            //   -128~+126
		int8_t crotch);                         //   -128~+126

enum {
	CALIB_INC = 0,
	CALIB_DEC,
	CALIB_GET,
	CALIB_RESET,
	CALIB_RESET_ALL,
};
int16_t _calibServo(uint8_t id, uint8_t cmd);

void quadCrawler_servoLoop(void);               // ロボット動作を経過時間に応じて更新する。

void _setPWM(uint8_t id, uint16_t value);


void quadCrawler_init(void);                    // 初期化処理、setup()で実行。

double quadCrawler_getSonner();                 // 超音波センサの値を取得する。

void quadCrawler_tone(int sound, int ms);       // ブザーを鳴らす。

void quadCrawler_colorWipe(uint8_t color);      // LEDを指定した色にする。COLOR_xx
enum {
  COLOR_OFF = 0,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_PURPLE,
  COLOR_LIGHTBLUE,
};

void quadCrawler_colorRed(uint8_t id);

void quadCrawler_rainbow(uint8_t wait);         // LEDを７色に光らせる。
uint8_t quadCrawler_checkServoON(void);

void quadCrawler_LED(uint8_t com);

int  quadCrawler_SW(void);

void quadCrawler_digitalWrite(uint8_t pin, uint8_t data);
uint8_t quadCrawler_digitalRead(uint8_t pin);

#endif  // quadCrawler_h
