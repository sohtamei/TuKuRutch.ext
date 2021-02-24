/* copyright (C) 2020 Sohta. */
#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <TukurutchEsp.h>
#include "main.h"

// ポート定義

#define ROVER_ADDRESS	0X38
#define P_LED			10

WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{
	digitalWrite(P_LED, !onoff);
	pinMode(P_LED, OUTPUT);
}

uint8_t _getSw(uint8_t button)
{
	switch(button) {
	case 0: return M5.BtnA.isPressed();
	case 1: return M5.BtnB.isPressed();
//	case 2: return M5.BtnC.isPressed();
	}
	return 0;
}

float getIMU(uint8_t index)
{
	float data[3]={0};
	if(index < 3) {
		M5.IMU.getGyroData(data+0,data+1,data+2);
		return data[index-0];
	} else if(index < 6) {
		M5.IMU.getAccelData(data+0,data+1,data+2);
		return data[index-3];
	} else if(index < 9) {
		//M5.IMU.getAhrsData(data+0,data+1,data+2);
		return data[index-6];
	} else {
		M5.IMU.getTempData(data+0);
		return data[0];
	}
}

void _tone(int sound, int ms)
{
#ifdef _M5StickCPlus
	M5.Beep.tone(sound, ms);
	delay(ms);
	M5.Beep.mute();
#else
	delay(ms);
#endif
}

void _beep(void)
{
#ifdef _M5StickCPlus
	M5.Beep.beep();
	delay(100);
	M5.Beep.mute();
#endif
}

// ServoCar

const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{8,33},{9,32}};
void _setServo(uint8_t idx, int16_t data/*normal:0~180, continuous:-100~+100*/, uint8_t continuous)
{
	if(idx >= numof(servoTable)) return;

	uint16_t pwmWidth;
	if(continuous) {
		#define srvZero 307		// 1.5ms/20ms*4096 = 307.2
		#define srvCoef 163		// (2.3ms-1.5ms)/20ms*4096 = 163.8
		if(data < -100) data = -100;
		else if(data > 100) data = 100;
		if(idx == 1) data = -data;
		pwmWidth = (data * srvCoef) / 100 + srvZero;
		if(data==0 && continuous!=2) pwmWidth=0;
	} else {
		#define srvMin 103		// 0.5ms/20ms*4096 = 102.4 (-90c)
		#define srvMax 491		// 2.4ms/20ms*4096 = 491.5 (+90c)
		if(data < 0) data = 0;
		else if(data > 180) data = 180;
		pwmWidth = (data * (srvMax - srvMin)) / 180 + srvMin;
	}
	ledcAttachPin(servoTable[idx].port, servoTable[idx].ledc);
	ledcWrite(servoTable[idx].ledc, pwmWidth);
}

struct { int16_t L; int16_t R;} static const dir_table[7] = {
//  L   R
  { 0,  0},  // 0:STOP
  { 1,  1},  // 1:FORWARD
  { 0,  1},  // 2:LEFT
  { 1,  0},  // 3:RIGHT
  {-1, -1},  // 4:BACK
  {-1,  1},  // 5:ROLL_LEFT
  { 1, -1},  // 6:ROLL_RIGHT
             // 7:CALIBRATION
};

void _setCar(uint8_t direction, uint8_t speed)
{
	if(direction >= numof(dir_table)) {
		_setServo(0, 0, 2);
		_setServo(1, 0, 2);
	} else {
		_setServo(0, speed * dir_table[direction].L, 1);
		_setServo(1, speed * dir_table[direction].R, 1);
	}
}

// Rover C

static void RoverC_Init(void)    
{
	Wire.begin(0,26,100);		//sda 0, scl 26
}

static void Send_iic(uint8_t Register, int16_t Speed)
{
	if(Speed >  100) Speed =  100;
	if(Speed < -100) Speed = -100;
	Wire.beginTransmission(ROVER_ADDRESS);
	Wire.write(Register);
	Wire.write(Speed);
	Wire.endTransmission();
}

uint8_t roverCInitF = false;
void setRoverC(int16_t F_L, int16_t F_R, int16_t R_L, int16_t R_R)
{
	if(!roverCInitF) {
		roverCInitF=true;
		RoverC_Init();
	}
	Send_iic(0x00, F_L);
	Send_iic(0x01, F_R);
	Send_iic(0x02, R_L);
	Send_iic(0x03, R_R);
}

void setRoverC_XYR(int16_t x, int16_t y, int16_t role)
{
	int16_t left = y+x;
	int16_t right = y-x;
	int16_t invK = 100;

	if(abs(left) > 100) invK = abs(left);
	else if(abs(right) > 100) invK = abs(right);

	if(invK != 100) {
		left  = (left*100)/invK;
		right = (right*100)/invK;
	}
	setRoverC(left+role, right-role, right+role, left-role);
}

struct { int8_t x; int8_t y; int8_t r; } const rdir_table[] = {
//  X  Y  R
  { 0, 0, 0},  // STOP
  { 1, 1, 0},  // UP_R
  { 0, 1, 0},  // UP
  {-1, 1, 0},  // UP_L
  { 1, 0, 0},  // RIGHT
  {-1, 0, 0},  // LEFT
  { 1,-1, 0},  // DOWN_R
  { 0,-1, 0},  // DOWN
  {-1,-1, 0},  // DOWN_L
  { 0, 0, 1},  // ROLL_R
  { 0, 0,-1},  // ROLL_L
};

void moveRoverC(uint8_t dir, uint8_t speed)
{
	if(dir >= sizeof(rdir_table)/sizeof(rdir_table[0])) return;
	setRoverC_XYR(speed*rdir_table[dir].x, speed*rdir_table[dir].y, speed*rdir_table[dir].r);
}

static void onConnect(String ip)
{
	M5.Lcd.fillScreen(BLACK);
	M5.Lcd.setCursor(0,0);
	M5.Lcd.println(ip);
	wsServer.listen(PORT_WEBSOCKET);
}


void _setup(const char* ver)
{
	M5.begin(true, true, true); // init lcd, power, serial
	M5.IMU.Init();
	M5.Lcd.setRotation(3);

	M5.Lcd.fillScreen(BLACK);
	M5.Lcd.setTextSize(2);

	M5.Lcd.setCursor(0, 0);
	if(_getSw(0)) {
		M5.Lcd.println("ESP SmartConfig");
		WiFi.mode(WIFI_STA);
		WiFi.beginSmartConfig();
		while (!WiFi.smartConfigDone()) {
			delay(1000);
			_setLED(1);
			_tone(T_C5, 100);
			_setLED(0);
		}
	} else {
		M5.Lcd.println(ver);
	}

	// ServoCar
	ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
	ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

	//RoverC_Init();
	Serial.begin(115200);
	initWifi(ver, false, onConnect);
}

void _loop(void)
{
//	loopWebSocket();
	M5.update();  // update button and speaker
//	delay(50);
}
