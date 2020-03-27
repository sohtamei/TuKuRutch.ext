#define mVersion "QuadCrawler1.1"

#include <Arduino.h>
#include <util/delay.h>
#include <quadCrawler.h>
#include <analogRemote.h>

static void funcLed(uint8_t onoff) { digitalWrite(13, onoff); }
static analogRemote remote(MODE_XYKEYS, /*port*/2, funcLed);

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  quadCrawler_init();
  quadCrawler_colorWipe(COLOR_PURPLE);
  quadCrawler_beep(100);
  Serial.begin(115200);
  Serial.println("Normal: " mVersion);
}

static uint8_t lastkey = 0;
static uint8_t originAdj = 0;
static uint8_t lastSw4 = 1;
static uint32_t sonner_time = 0;
extern volatile unsigned long timer0_millis;

void loop()
{
  #define A_DOWN_OFFSET  0x10
  remote.checkUpdated();
  uint8_t key = remote.keys;
  switch(key) {
  case BUTTON_A_XY:
    key = remote.xyKeys;
    break;
  case BUTTON_A_DOWN:
    key = remote.xyKeys + A_DOWN_OFFSET;
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
      case BUTTON_MENU:
        quadCrawler_beep(3000);
        break;
      case BUTTON_0:
        quadCrawler_beep(50);
        for (int n = 0; n < 5; n++) {
          quadCrawler_rainbow(5);
        }
        break;

      case XY_UP:
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
      default:
        quadCrawler_colorWipe(COLOR_PURPLE);
        quadCrawler_Walk(quadCrawler_fast, stop);
        break;
    }
  }

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

  if(remote.xyLevel >= 10) {
    quadCrawler_setSpeed(25000 / remote.xyLevel);
  }
  quadCrawler_servoLoop();

  uint16_t elapsed = (timer0_millis - sonner_time);
  if(elapsed >= 100) {
    sonner_time = timer0_millis;
    double sonner_val = quadCrawler_getSonner();
    if (sonner_val < 8){
      quadCrawler_beep(sonner_val * 10);

      if(lastkey == XY_UP || lastkey == BUTTON_UP) {
        quadCrawler_colorWipe(COLOR_PURPLE);
        quadCrawler_Walk(quadCrawler_fast, stop);
      }
    }
  }
}
