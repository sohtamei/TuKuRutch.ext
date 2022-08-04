// copyright to SohtaMei & H.Sunakawa 2020.
#define mVersion "QuadCrawler1.5"

#include <quadCrawler.h>
#include <analogRemote.h>

static void funcLed(uint8_t onoff) { digitalWrite(P_LED, onoff); }
static analogRemote remote(MODE_XYKEYS, /*port*/P_IRRX, funcLed);

static uint8_t lastkey = 0;
static uint8_t originAdj = 0;
static uint8_t originAdjId;
static uint32_t sonner_time = 0;
extern volatile unsigned long timer0_millis;

static uint8_t lastSw4 = 1;
static int detect_sw4(void)
{
	int detect = 0;
	uint8_t sw4 = digitalRead(P_Sw4);
	if(lastSw4!=sw4 && sw4==0)
		detect = 1;
	lastSw4 = sw4;
	return detect;
}
void setup()
{
	// 初期化処理
	Serial.begin(115200);
	pinMode(P_LED, OUTPUT);
	digitalWrite(P_LED, LOW);
	quadCrawler_init();
    remote.probe();
	quadCrawler_colorWipe(COLOR_PURPLE);
	quadCrawler_beep(100);
	Serial.println("Normal: " mVersion);
}

void loop()
{
	if(originAdj) {
	//	loop_originAdj();
		if(detect_sw4()) {
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			originAdj = 0;
			return;
		} else if(originAdj && !quadCrawler_checkServoON()) {
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_beep(500);
			originAdj = 0;
			return;
		}

		remote.checkUpdated();            // リモコンコード受信
		uint8_t key = remote.keys;        // 受信したリモコンコード取得
		if(key != lastkey) {
			lastkey = key;
			int ret;
			switch(key) {
			case BUTTON_1: quadCrawler_colorRed(0); originAdjId = 1; break;
			case BUTTON_2: quadCrawler_colorRed(3); originAdjId = 3; break;
			case BUTTON_3: quadCrawler_colorRed(1); originAdjId = 2; break;
			case BUTTON_4: quadCrawler_colorRed(2); originAdjId = 4; break;
			}
			if(!originAdjId) return;

			switch(key) {
			case BUTTON_UP:
				ret = _calibServo((originAdjId-1)*2+0, CALIB_DEC);
				quadCrawler_beep((ret <= -30) ? 1000:50);
				break;

			case BUTTON_DOWN:
				ret = _calibServo((originAdjId-1)*2+0, CALIB_INC);
				quadCrawler_beep((ret >= 30) ? 1000:50);
				break;

			case BUTTON_LEFT:
				ret = _calibServo((originAdjId-1)*2+1, CALIB_DEC);
				quadCrawler_beep((ret <= -30) ? 1000:50);
				break;

			case BUTTON_RIGHT:
				ret = _calibServo((originAdjId-1)*2+1, CALIB_INC);
				quadCrawler_beep((ret >= 30) ? 1000:50);
				break;

			case BUTTON_CENTER:
				_calibServo((originAdjId-1)*2+0, CALIB_RESET);
				_calibServo((originAdjId-1)*2+1, CALIB_RESET);
				quadCrawler_beep(1000);
				break;
			}
		}
		return;
	}

	// SW4を押したとき初期姿勢にする (組み立て用)
	if(detect_sw4()) {
		int i;
		quadCrawler_colorWipe(COLOR_RED);
		for(i = 0; i < 4; i++)
			quadCrawler_setPose1(i, 0, 0);
		originAdj = 1;
		originAdjId = 0;
		return;
	}

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
			quadCrawler_Walk(quadCrawler_fast, COM_FW);
			break;
		case XY_LEFT:
		case BUTTON_LEFT:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_LEFT);
			break;
		case XY_RIGHT:
		case BUTTON_RIGHT:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_RIGHT);
			break;
		case XY_DOWN:
		case BUTTON_DOWN:
			quadCrawler_colorWipe(COLOR_RED);
			quadCrawler_Walk(quadCrawler_fast, COM_RW);
			break;
		case XY_UP_R:
		case XY_DOWN_L:
		case BUTTON_RETURN:
			quadCrawler_colorWipe(COLOR_GREEN);
			quadCrawler_Walk(quadCrawler_fast, COM_CW);
			break;
		case XY_UP_L:
		case XY_DOWN_R:
		case BUTTON_TEST:
			quadCrawler_colorWipe(COLOR_GREEN);
			quadCrawler_Walk(quadCrawler_fast, COM_CCW);
			break;

		case XY_UP_R + A_DOWN_OFFSET:
		case BUTTON_1:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_UP);
			break;
		case BUTTON_A_CENTER:
		case BUTTON_2:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_DOWN);
			break;
		case XY_DOWN + A_DOWN_OFFSET:
		case BUTTON_3:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_T_DOWN);
			break;
		case XY_UP + A_DOWN_OFFSET:
		case BUTTON_4:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_H_DOWN);
			break;
		case XY_LEFT + A_DOWN_OFFSET:
		case BUTTON_5:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_L_DOWN);
			break;
		case XY_RIGHT + A_DOWN_OFFSET:
		case BUTTON_6:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_R_DOWN);
			break;

		case BUTTON_A_UP:
		case BUTTON_7:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_T_UPDOWN);
			break;
		case BUTTON_A_LEFT:
		case BUTTON_8:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_L_R_UP);
			break;
		case BUTTON_A_RIGHT:
		case BUTTON_9:
			quadCrawler_colorWipe(COLOR_LIGHTBLUE);
			quadCrawler_Walk(quadCrawler_fast, COM_ALL_UPDOWN);
			break;
		default:                      // ボタンを離したとき : 停止
			quadCrawler_colorWipe(COLOR_PURPLE);
			quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			break;
		}
	}

	// 歩行などのモーション処理、経過時間に応じてサーボモーターを制御する。
	if(remote.xyLevel >= 10) {
		// アナログリモコンのJOYSTICK操作のとき速度設定
		quadCrawler_setSpeed(25000 / remote.xyLevel, remote.x, remote.y);
	}
	quadCrawler_servoLoop();

	uint16_t elapsed = (timer0_millis - sonner_time);
	if(elapsed >= 100) {
		sonner_time = timer0_millis;
		// 超音波センサで障害物を検出したときブザーを鳴らす (100ms周期)
		double sonner_val = quadCrawler_getSonner();
		if (sonner_val < 8){
			quadCrawler_beep(sonner_val * 10);

			if(lastkey == XY_UP || lastkey == BUTTON_UP) {
				quadCrawler_colorWipe(COLOR_PURPLE);
				quadCrawler_Walk(quadCrawler_fast, COM_STOP);
			}
		}
	}
}
