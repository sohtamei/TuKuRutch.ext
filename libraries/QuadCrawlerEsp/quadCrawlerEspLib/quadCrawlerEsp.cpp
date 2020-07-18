#include <stdint.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include "quadCrawlerEsp.h"
#include "esp_camera.h"

// ポート定義

/*
#define PORT_IRRX	13
#define PORT_LED1	2
*/

#define MEN		4	// Surbo Moter Drive Enable Pin
#define Neopix	14
#define Echo	12	// Echo Pin
#define Trig	15	// Trigger Pin
#define Bz		33	// Bzzer Pin
#define SCL		27
#define SDA		26

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     32
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
#define servo_min 200  // Min pulse length out of 4096
#define servo_max 400 // Max pulse length out of 4096
#define deg_max   130  // Max deg
#define deg_min   20   // Min deg


//各サーボ定義
//Right Front Knee SV1-1
#define RFK  1
#define RFK_N 70
#define RFK_U 58
#define RFK_D 80
#define RFK_UD 130

//Right Front Crach SV1-3
#define RFC  0
#define RFC_N 85
#define RFC_R 60
#define RFC_F 90

//Right Rear Knee SV3-1
#define RRK  5
#define RRK_N 68
#define RRK_U 55
#define RRK_D 75
#define RRK_UD 130

//Right Rear Crach SV3-4
#define RRC  4
#define RRC_N 90
#define RRC_R 80
#define RRC_F 110

//Left Front Knee SV2-1
#define LFK  3
#define LFK_N 90
#define LFK_U 105
#define LFK_D 80
#define LFK_UD 20

//Left Front Crach SV2-4
#define LFC  2
#define LFC_N 90
#define LFC_R 110
#define LFC_F 80

//Left Rear Knee SV4-1
#define LRK  7
#define LRK_N 90
#define LRK_U 103
#define LRK_D 80
#define LRK_UD 20

//Left Rear Crach SV4-4
#define LRC  6
#define LRC_N 90
#define LRC_R 90
#define LRC_F 60

enum {
  ServoOff = 0,
  ServoNeutral,
  ServoNormal,
  ServoRepeat0,
  ServoRepeat1,
  ServoRepeat2,
  ServoRepeat3,
  ServoPose,
};

#define RATE  0.6

static uint8_t  servoState = 0;
static uint32_t servoTime = 0;
static uint16_t servoDelay = quadCrawler_fast;
static uint16_t accel_duration = 0;
static int32_t inv_accel = 1;

static const uint8_t (*servoMotion)[8] = NULL;
static uint8_t servoStart[8] = {RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N};
static uint8_t servoEnd[8] = {RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N};

static uint8_t cur_com = stop;

//extern volatile unsigned long timer0_millis;


static void set_servo_deg(uint8_t id, unsigned int deg) {
  const uint8_t channel_table[] = {RFK, RFC, RRK, RRC, LFK, LFC, LRK, LRC};
  uint8_t channel = channel_table[id];
  unsigned int setdata;
  setdata = (unsigned int)((deg - deg_min) *  (servo_max - servo_min) / (deg_max - deg_min)) + servo_min;
  if (setdata <= servo_max) {
    if (setdata >= servo_min) {
      pwm.setPWM(channel, 0, setdata);
      //delay(20);
    }
  }
}

static void set_servo_deg8(const uint8_t motion[8], uint8_t state) {
  uint8_t i;
  for(i = 0; i < 8; i++) {
    if(state >= ServoRepeat0 && state <= ServoRepeat3 || state == ServoNeutral) {
      set_servo_deg(i, servoEnd[i]);
      servoStart[i] = servoEnd[i];
      if(motion[i] != 0xFF) {
        servoEnd[i] = motion[i];
      }
    } else {
      if(motion[i] != 0xFF) {
        set_servo_deg(i, motion[i]);
        servoEnd[i] = motion[i];
      }
    }
  }
  servoState = state;
  servoTime = millis();
}

static void set_servo_motion(const uint8_t (*motion)[8], uint8_t state) {
  servoMotion = motion;
  set_servo_deg8(motion[0], state);
}

static void set_servo_off8(uint8_t state) {
  uint8_t i;
  for(i = 0; i < 8; i++)
    pwm.setPWM(i, 0, 0);
  servoState = state;
  servoTime = millis();
  cur_com = stop;
}

static void sv_init() {
  const uint8_t motion[] = {RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N};
  set_servo_motion(&motion, ServoNeutral);
}

static const uint8_t angle_table[5][8] = {
	{-1,    -1,    -1,    -1,    -1,    -1,    -1,    -1},			// keep
	{RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N},		// neutral
	{RFK_U, RFC_R, RRK_U, RRC_R, LFK_U, LFC_R, LRK_U, LRC_R},		// up/rear
	{RFK_D, RFC_F, RRK_D, RRC_F, LFK_D, LFC_F, LRK_D, LRC_F},		// down/forward
	{RFK_UD,RFC_N, RRK_UD,RRC_N, LFK_UD,LFC_N, LRK_UD,LRC_N},		// downMax
};

void quadCrawler_setPose4(
	uint8_t rfk, uint8_t rfc,
	uint8_t rrk, uint8_t rrc,
	uint8_t lfk, uint8_t lfc,
	uint8_t lrk, uint8_t lrc)
{
	uint8_t motion[8];
	motion[0] = angle_table[rfk][0];
	motion[1] = angle_table[rfc][1];
	motion[2] = angle_table[rrk][2];
	motion[3] = angle_table[rrc][3];
	motion[4] = angle_table[lfk][4];
	motion[5] = angle_table[lfc][5];
	motion[6] = angle_table[lrk][6];
	motion[7] = angle_table[lrc][7];
	set_servo_motion(&motion, ServoPose);
	cur_com = pose;
}

void quadCrawler_setPose1(uint8_t index, uint8_t knee, uint8_t crach)
{
	if(index >= 4) return;

	uint8_t motion[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
	motion[index*2+0] = angle_table[knee][index*2+0];
	motion[index*2+1] = angle_table[crach][index*2+1];
	set_servo_motion(&motion, ServoPose);
	cur_com = pose;
}

uint8_t quadCrawler_checkServoON(void)
{
  return (servoState != ServoOff);
}

void quadCrawler_servoLoop(void)
{
  int32_t elapsed = (millis() - servoTime) & 0x7FFFFFFF;
  // サーボOFFか、サーボHoldか、delay時間が経過してないとき
  switch(servoState) {
  case ServoOff:
    return;

  case ServoPose:
  case ServoNormal:
    if(elapsed >= 180*1000UL) {
      set_servo_off8(ServoOff);
      digitalWrite(PORT_LED1, 0);
      return;
    }
    if((elapsed >> 8) & 1)      // 256
      digitalWrite(PORT_LED1, 1);
    else
      digitalWrite(PORT_LED1, 0);
    return;

  default:
    if(elapsed < servoDelay + accel_duration/2) {	// 動作時間=servoDelay + accel_duration/2
	  int32_t k256;		// 距離の係数*256
	  if(elapsed < accel_duration) {
		// 等加速度運動 v=elapsed/inv_accel, x=elapsed^2/inv_accel/2
	    k256 = (elapsed * elapsed * (256/2))/inv_accel;
	  } else {
	    // 等速運動 v=1/servoDelay, x=(elapsed - accel_duration/2)*v
	    k256 = ((elapsed - accel_duration/2) * 256)/servoDelay;
	  }

	  uint8_t i;
	  for(i = 0; i < 8; i++) {
	    if(servoEnd[i] != servoStart[i])
		  set_servo_deg(i, ((((int)servoEnd[i] - (int)servoStart[i]) * k256)>>8) + servoStart[i]);
	  }
      return;
    }
    break;
  }

  // normal-なにもしない、repeat-動作を繰り返し
  switch(servoState) {
  case ServoRepeat0:
    set_servo_deg8(servoMotion[1], ServoRepeat1);
    break;
  case ServoRepeat1:
    set_servo_deg8(servoMotion[2], ServoRepeat2);
    break;
  case ServoRepeat2:
    set_servo_deg8(servoMotion[3], ServoRepeat3);
    break;
  case ServoRepeat3:
    set_servo_deg8(servoMotion[0], ServoRepeat0);
    break;
  case ServoNeutral:
    set_servo_off8(ServoOff);
    break;
  }
}

void quadCrawler_setSpeed(uint16_t speed) {
  servoDelay = speed;
  accel_duration = servoDelay * RATE;				// 加速時間
  inv_accel = ((uint32_t)servoDelay) * accel_duration;	// v=1/servoDelay, 加速度係数=1/servoDelay/accel_duration,
														// 加速度係数(逆数)=servoDelay*accel_duration
}

void quadCrawler_Walk(uint16_t speed, uint8_t com) {
  quadCrawler_setSpeed(speed);

  if (cur_com == com) return;
  cur_com = com;

  if (com == stop) {
    const uint8_t motion[] = {RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N};

    // normal, repeatのとき : neutral姿勢
    if(servoState != ServoNeutral && servoState != ServoOff) {
      set_servo_motion(&motion, ServoNeutral);
    }
  }
  else if (com == fw) {
    //Serial.println ("-fw");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_R, -1,    LFC_R, -1,    LRC_F},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1,  },
                           {-1,    RFC_R, -1,    RRC_F, -1,    LFC_F, -1,    LRC_R},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1,  }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == rw) {
    //Serial.println ("-rw");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_R, -1,    LFC_R, -1,    LRC_F},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1,  },
                           {-1,    RFC_R, -1,    RRC_F, -1,    LFC_F, -1,    LRC_R},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1,  }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == cw) {
    //Serial.println ("-cw");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_R, -1,    LFC_F, -1,    LRC_R},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1,  },
                           {-1,    RFC_R, -1,    RRC_F, -1,    LFC_R, -1,    LRC_F},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1,  }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == ccw) {
    //Serial.println ("-ccw");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_R, -1,    LFC_F, -1,    LRC_R},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1,  },
                           {-1,    RFC_R, -1,    RRC_F, -1,    LFC_R, -1,    LRC_F},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1,  }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == Rigt) {
    //Serial.println ("-right");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_F, -1,    LFC_F, -1,    LRC_F},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1,  },
                           {-1,    RFC_R, -1,    RRC_R, -1,    LFC_R, -1,    LRC_R},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1,  }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == Left) {
    //Serial.println ("-right");
    static const uint8_t motion[4][8] = {
                           {-1,    RFC_F, -1,    RRC_F, -1,    LFC_F, -1,    LRC_F},
                           {RFK_D, -1,    RRK_U, -1,    LFK_U, -1,    LRK_D, -1   },
                           {-1,    RFC_R, -1,    RRC_R, -1,    LFC_R, -1,    LRC_R},
                           {RFK_U, -1,    RRK_D, -1,    LFK_D, -1,    LRK_U, -1   }};
    set_servo_motion(motion, ServoRepeat0);
  }
/*
  else if (com == nt) {
    //Serial.println ("-nutral");
    const uint8_t motion[] = {RFK_N, RFC_N, RRK_N, RRC_N, LFK_N, LFC_N, LRK_N, LRC_N};
    set_servo_motion(&motion, ServoNormal);
  }
*/
  else if (com == all_up) {
    //Serial.println ("-all up");
    const uint8_t motion[] = {RFK_U, -1,    RRK_U, -1,    LFK_U, -1,    LRK_U, -1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == all_dn) {
    //Serial.println ("-all down");
    const uint8_t motion[] = {RFK_UD,-1,    RRK_UD,-1,    LFK_UD,-1,    LRK_UD,-1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == t_dn) {
    //Serial.println ("-Hip up");
    const uint8_t motion[] = {RFK_U, -1,    RRK_UD,-1,    LFK_U, -1,    LRK_UD,-1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == h_dn) {
    //Serial.println ("-Top up");
    const uint8_t motion[] = {RFK_UD,-1,    RRK_U, -1,    LFK_UD,-1,    LRK_U, -1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == l_dn) {
    //Serial.println ("-Right up");
    const uint8_t motion[] = {RFK_U, -1,    RRK_U, -1,    LFK_UD,-1,    LRK_UD,-1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == r_dn) {
    //Serial.println ("Left up");
    const uint8_t motion[] = {RFK_UD,-1,    RRK_UD,-1,    LFK_U, -1,    LRK_U, -1   };
    set_servo_motion(&motion, ServoNormal);
  }
  else if (com == t_up_dn) {
    static const uint8_t motion[4][8] = {
                           {RFK_U, -1,    RRK_U, -1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_U, -1,    RRK_UD,-1,    LFK_U, -1,    LRK_UD,-1   },
                           {RFK_U, -1,    RRK_U, -1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_U, -1,    RRK_UD,-1,    LFK_U, -1,    LRK_UD,-1   }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == l_r_up) {
    static const uint8_t motion[4][8] = {
                           {RFK_UD,-1,    RRK_UD,-1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_U, -1,    RRK_U, -1,    LFK_UD,-1,    LRK_UD,-1   },
                           {RFK_UD,-1,    RRK_UD,-1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_U, -1,    RRK_U, -1,    LFK_UD,-1,    LRK_UD,-1   }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else if (com == all_up_dn) {
    static const uint8_t motion[4][8] = {
                           {RFK_U, -1,    RRK_U, -1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_UD,-1,    RRK_UD,-1,    LFK_UD,-1,    LRK_UD,-1   },
                           {RFK_U, -1,    RRK_U, -1,    LFK_U, -1,    LRK_U, -1   },
                           {RFK_UD,-1,    RRK_UD,-1,    LFK_UD,-1,    LRK_UD,-1   }};
    set_servo_motion(motion, ServoRepeat0);
  }
  else {

  }
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, Neopix, NEO_GRB + NEO_KHZ800);

static void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void quadCrawler_colorWipe(uint8_t color) {
  switch(color) {
  case COLOR_RED:
    colorWipe(strip.Color(255,0,0), 10);
    break;
  case COLOR_GREEN:
    colorWipe(strip.Color(0,255,0), 10);
    break;
  case COLOR_BLUE:
    colorWipe(strip.Color(0,0,255), 10);
    break;
  case COLOR_YELLOW:
    colorWipe(strip.Color(128,128,0), 10);
    break;
  case COLOR_PURPLE:
    colorWipe(strip.Color(128,0,128), 10);
    break;
  case COLOR_LIGHTBLUE:
    colorWipe(strip.Color(0,128,128), 10);
    break;
  default:
    break;
  }
}

static uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void quadCrawler_rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
void quadCrawler_theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

double quadCrawler_getSonner() {
  if(digitalRead(Echo) == HIGH)
    return 100.0;

  double data;
  double distance;
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH );
  delayMicroseconds(10);
  digitalWrite(Trig, LOW );
  data = pulseIn(Echo, HIGH );
  if (data > 0) {
    distance = data * 0.017;
    return distance;
  }
  else {
    return 0;
  }
}

#define LEDC_CHANNEL_15 15
#define LEDC_TIMER_13_BIT  13
#define LEDC_BASE_FREQ     5000
void quadCrawler_tone(int sound, int ms) {
  ledcWriteTone(LEDC_CHANNEL_15, sound);
  delay(ms);
  ledcWriteTone(LEDC_CHANNEL_15, 0);
}

void quadCrawler_init(void)
{
  pinMode( PORT_LED1, OUTPUT );
  pinMode( MEN, OUTPUT );
  digitalWrite( MEN, HIGH);
  pinMode( Echo, INPUT_PULLUP );
  pinMode( Trig, OUTPUT );
  ledcSetup(LEDC_CHANNEL_15, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(Bz, LEDC_CHANNEL_15);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Wire.begin(SDA,SCL);
  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
  yield();
  digitalWrite( MEN, LOW);
  sv_init();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;//20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);
}
