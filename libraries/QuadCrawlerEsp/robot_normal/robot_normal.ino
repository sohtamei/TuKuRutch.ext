#define mVersion "QuadCrawlerEsp1.2"

#include "quadCrawlerEsp.h"
#include <analogRemote.h>
#include "TukurutchEsp.h"

WifiRemote remote1;

void funcLed(uint8_t onoff) { digitalWrite(PORT_LED1, onoff); }
analogRemote remote(MODE_XYKEYS_MERGE, /*port*/PORT_IRRX, funcLed);

void setup()
{
  // 初期化処理
  quadCrawler_init();
  quadCrawler_colorWipe(COLOR_PURPLE);
  quadCrawler_beep(100);
  Serial.begin(115200);

  initWifi(mVersion, true);
  Serial.println("Normal: " mVersion);

  startCameraServer();
}

static uint8_t lastkey = 0;
static uint8_t originAdj = 0;
static uint8_t lastSw4 = 1;
static uint32_t sonner_time = 0;

void loop()
{
  // リモコン処理：標準リモコン、アナログリモコンを受信し、キーコードに従ってロボットを動かす。
  #define A_DOWN_OFFSET  0x10
  remote.checkUpdated();            // リモコンコード受信
  uint8_t key = remote.keys;        // 受信したリモコンコード取得
  switch(key) {
  case BUTTON_A_XY:                 // アナログリモコン JOYSTICK操作のとき
    key = remote.xyKeys;
    break;
  case BUTTON_A_DOWN:
    key = remote.xyKeys + A_DOWN_OFFSET;  // アナログリモコン D ボタン押し
    delay(20);
    break;
  }

  if(key != lastkey) {
    lastkey = key;
    switch(key) {
      case BUTTON_POWER:
      case BUTTON_C:
      case BUTTON_B:
      case BUTTON_CENTER:
        break;
      case BUTTON_MENU:             // MENUボタン : ブザー3秒
        quadCrawler_beep(3000);
        break;
      case BUTTON_0:                // 0ボタン : LEDレインボー
        quadCrawler_beep(50);
        for (int n = 0; n < 5; n++) {
          quadCrawler_rainbow(5);
        }
        break;

      case XY_UP:                   // 上ボタン : 前進
      case BUTTON_UP:
        quadCrawler_colorWipe(COLOR_BLUE);
        quadCrawler_Walk(quadCrawler_fast, fw);
        break;
      case XY_LEFT:
      case BUTTON_LEFT:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, Left);
        break;
      case XY_RIGHT:
      case BUTTON_RIGHT:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, Rigt);
        break;
      case XY_DOWN:
      case BUTTON_DOWN:
        quadCrawler_colorWipe(COLOR_RED);
        quadCrawler_Walk(quadCrawler_fast, rw);
        break;
      case XY_UP_R:
      case XY_DOWN_L:
      case BUTTON_RETURN:
        quadCrawler_colorWipe(COLOR_GREEN);
        quadCrawler_Walk(quadCrawler_fast, cw);
        break;
      case XY_UP_L:
      case XY_DOWN_R:
      case BUTTON_TEST:
        quadCrawler_colorWipe(COLOR_GREEN);
        quadCrawler_Walk(quadCrawler_fast, ccw);
        break;

      case XY_UP_R + A_DOWN_OFFSET:
      case BUTTON_1:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, all_up);
        break;
      case BUTTON_A_CENTER:
      case BUTTON_2:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, all_dn);
        break;
      case XY_DOWN + A_DOWN_OFFSET:
      case BUTTON_3:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, t_dn);
        break;
      case XY_UP + A_DOWN_OFFSET:
      case BUTTON_4:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, h_dn);
        break;
      case XY_LEFT + A_DOWN_OFFSET:
      case BUTTON_5:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, l_dn);
        break;
      case XY_RIGHT + A_DOWN_OFFSET:
      case BUTTON_6:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, r_dn);
        break;

      case BUTTON_A_UP:
      case BUTTON_7:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, t_up_dn);
        break;
      case BUTTON_A_LEFT:
      case BUTTON_8:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, l_r_up);
        break;
      case BUTTON_A_RIGHT:
      case BUTTON_9:
        quadCrawler_colorWipe(COLOR_LIGHTBLUE);
        quadCrawler_Walk(quadCrawler_fast, all_up_dn);
        break;
      default:                      // ボタンを離したとき : 停止
        quadCrawler_colorWipe(COLOR_PURPLE);
        quadCrawler_Walk(quadCrawler_fast, stop);
        break;
    }
  }

  // SW4を押したとき初期姿勢にする (組み立て用)
/*
  uint8_t sw4 = digitalRead(Sw4);
  if(lastSw4!=sw4 && sw4==0) {
    if(!originAdj) {
      quadCrawler_colorWipe(COLOR_RED);
      quadCrawler_setPose4(POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL, POSE_NEUTRAL);
    } else {
      quadCrawler_colorWipe(COLOR_PURPLE);
      quadCrawler_Walk(quadCrawler_fast, stop);
    }
    originAdj = !originAdj;
  } else if(originAdj && !quadCrawler_checkServoON()) {
    quadCrawler_colorWipe(COLOR_PURPLE);
    quadCrawler_beep(50);
    originAdj = 0;
  }
  lastSw4 = sw4;
*/
  // 歩行などのモーション処理、経過時間に応じてサーボモーターを制御する。
  if(remote.xyLevel >= 10) {
    // アナログリモコンのJOYSTICK操作のとき速度設定
    quadCrawler_setSpeed(25000 / remote.xyLevel);
  }
  sendNotifyArduinoMode();
  quadCrawler_servoLoop();
/*
  uint16_t elapsed = (millis() - sonner_time);
  if(elapsed >= 100) {
    sonner_time = millis();
    // 超音波センサで障害物を検出したときブザーを鳴らす (100ms周期)
    double sonner_val = quadCrawler_getSonner();
    if (sonner_val < 8){
      quadCrawler_beep(sonner_val * 10);

      if(lastkey == XY_UP || lastkey == BUTTON_UP) {
        quadCrawler_colorWipe(COLOR_PURPLE);
        quadCrawler_Walk(quadCrawler_fast, stop);
      }
    }
  }
*/
}
