#ifndef quadCrawler_h
#define quadCrawler_h

#include <stdint.h>

// ポート定義
enum {
  P_IRRX = 2,
  P_Sw1 = 3,
  P_Sw2 = 4,
  P_Sw3 = 5,
  P_Sw4 = 6,
  P_LED = 13,
};

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
	POSE_KEEP     = 0,
	POSE_NEUTRAL  = 1,
	POSE_UP       = 2,
	POSE_DOWN     = 3,
	POSE_DOWNMAX  = 4,

	POSE_REAR     = 2,
	POSE_FRONT    = 3,
};

enum {
	FRONT_R   = 0,
	REAR_R    = 1,
	FRONT_L   = 2,
	REAR_L    = 3,
};
void quadCrawler_setPose1(                      // 指定した足の上下、前後の姿勢を設定する。
		uint8_t id,                             //   FRONT_R, 他
		int8_t knee,                            //   -128~+126
		int8_t crach);                          //   -128~+126

void quadCrawler_setPose4(
		int8_t rfk, int8_t rfc,
		int8_t rrk, int8_t rrc,
		int8_t lfk, int8_t lfc,
		int8_t lrk, int8_t lrc);

enum {
	CALIB_INC = 0,
	CALIB_DEC,
	CALIB_GET,
	CALIB_RESET,
	CALIB_RESET_ALL,
};
int16_t _calibServo(uint8_t id, uint8_t cmd);

void quadCrawler_servoLoop(void);               // ロボット動作を経過時間に応じて更新する。


void quadCrawler_init(void);                    // 初期化処理、setup()で実行。

double quadCrawler_getSonner();                 // 超音波センサの値を取得する。
void quadCrawler_beep(int time);                // ブザーを鳴らす。

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

void quadCrawler_rainbow(uint8_t wait);         // LEDを７色に光らせる。
uint8_t quadCrawler_checkServoON(void);

#endif  // quadCrawler_h
