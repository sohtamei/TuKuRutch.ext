// copyright to SohtaMei 2019.

#ifndef remoconRobo_h
#define remoconRobo_h

void remoconRobo_init(void);
void remoconRobo_initCh4(void);

// Tone

enum {
	T_C4=262, T_D4=294, T_E4=330, T_F4=349, T_G4=392, T_A4=440, T_B4=494,
	T_C5=523, T_D5=587, T_E5=659, T_F5=698,
};

void remoconRobo_tone(int sound, int ms);

// Motor

enum {
	CH_R,
	CH_L,
	CH3,
	CH4,
};

enum {
	DIR_FORWARD = 0,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_BACK,
	DIR_ROLL_LEFT,
	DIR_ROLL_RIGHT,
};

void remoconRobo_setMotor(int ch, int speed);
void remoconRobo_setRobot(int direction, int speed);
void remoconRobo_setRobotLR(int speedL, int speedR);

int remoconRobo_getCalib(void);
int remoconRobo_setCalib(int calib);
int remoconRobo_incCalib(int offset);
/*
// MP3
void remoconRobo_initMP3(int volume);			// 0~30
void remoconRobo_playMP3(int track, int loop);
void remoconRobo_stopMP3(void);
*/
uint16_t remoconRobo_getAnalog(uint8_t ch, uint16_t count, uint8_t discharge);

#endif
