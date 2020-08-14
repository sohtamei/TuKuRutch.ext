#ifndef quadCrawler_h
#define quadCrawler_h

#include <stdint.h>

// ポート定義
#define PORT_SW		0
#define PORT_IRRX	13
#define PORT_LED1	2

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
  stop = 0,

  // repeat
  fw,
  cw,
  ccw,
  rw,
  Rigt,
  Left,

  // normal
  all_up,
  all_dn,
  t_dn,
  h_dn,
  l_dn,
  r_dn,

  // repeat
  t_up_dn,
  l_r_up,
  all_up_dn,

  pose,
};

void quadCrawler_Walk(                          // ロボット動作、指定した動作を継続する。
        uint16_t speed,                         //   quadCrawler_xx
        uint8_t com);                           //   stop, 他
void quadCrawler_setSpeed(uint16_t speed);      // ロボット動作の速度を更新する。

enum {
  POSE_KEEP     = 0,
  POSE_NEUTRAL  = 1,
  POSE_UP       = 2,
  POSE_DOWN     = 3,
  POSE_DOWNMAX  = 4,

  POSE_REAR     = 2,
  POSE_FRONT    = 3,
};

void quadCrawler_setPose4(
  uint8_t rfk, uint8_t rfc, 
  uint8_t rrk, uint8_t rrc, 
  uint8_t lfk, uint8_t lfc, 
  uint8_t lrk, uint8_t lrc);

enum {
  FRONT_R   = 0,
  REAR_R    = 1,
  FRONT_L   = 2,
  REAR_L    = 3,
};
void quadCrawler_setPose1(                      // 指定した足の上下、前後の姿勢を設定する。
        uint8_t index,                          //   FRONT_R, 他
        uint8_t knee,                           //   POSE_xx
        uint8_t crach);                         //   POSE_xx

void quadCrawler_servoLoop(void);               // ロボット動作を経過時間に応じて更新する。


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

void quadCrawler_rainbow(uint8_t wait);         // LEDを７色に光らせる。
uint8_t quadCrawler_checkServoON(void);
#endif  // quadCrawler_h
