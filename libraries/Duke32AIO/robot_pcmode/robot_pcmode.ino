// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "Duke32AIO 1.0"

#include <Wire.h>
#include "TukurutchEsp.h"

WifiRemote remote;

// duke32.h
/********************************
* for LED Control
*********************************/
#define NEO_PIN     27

/********************************
* for Servo Control
*********************************/
#define Servo_PIN1  23
#define Servo_PIN2   4

#define LEDC_CHANNEL_0  0  // channel of 16 channels (started from zero)
#define LEDC_CHANNEL_1  1  // channel of 16 channels (started from zero)
#define LEDC_TIMER_BIT 12  // use 12 bit precission for LEDC timer
#define LEDC_BASE_FREQ 50  // use 50 Hz as a LEDC base frequency

// Servo Range
// int srvMin = 103; // (103/4096)*20ms = 0.5 ms (-90c)
// int srvMax = 491; // (491/4096)*20ms = 2.4 ms (+90c)
#define srvMin 103
#define srvMax 491

/********************************
* for Motor Control
*********************************/
#define MTR_A1  13  // moterA IN1
#define MTR_A2  12  // moterA IN2
#define MTR_AE  18  // moterA EN
#define MTR_B1  14  // moterB IN1
#define MTR_B2  15  // moterB IN2
#define MTR_BE  19  // moterB EN

#define LEDC_CHANNEL_2  2   // channel of 16 channels (started from zero)
#define LEDC_CHANNEL_3  3   // channel of 16 channels (started from zero)
#define LEDC_TIMER_BIT_MTR 8    // use 8 bit precission for LEDC timer
#define LEDC_BASE_FREQ_MTR 255  // use 50 Hz as a LEDC base frequency

#define MotorL 0
#define MotorR 1

/********************************
* for I/O Control
*********************************/
#define ANA11   34
#define ANA12   35
#define A34     34  //I, GPIO34, ADC1_CH6
#define A35     35  //I, GPIO35, ADC1_CH7

#define DIGI01  32
#define DIGI02  33
#define D32     32  //IO, GPIO32, ADC1_CH4, TOUCH9
#define D33     33  //IO, GPIO33, ADC1_CH5, TOUCH8

#define DIGI11  26
#define DIGI12  25
#define D26     26  //IO, GPIO25, DAC_1, ADC2_CH8
#define D25     25  //IO, GPIO26, DAC_2, ADC2_CH9

// I2C
#define I2CSDA  21
#define I2CSCL  22
#define D21     21  //IO, GPIO21
#define D22     22  //IO, GPIO22

// UART2
#define UARTRXD 16
#define UARTTXD 17
#define D16     16  //IO, GPIO16, U2RXD
#define D17     17  //IO, GPIO17, U2TXD
// --- duke32.h


// robot_normal.ino
#define useLED
/********************************
* for LED Control
*********************************/
#ifdef useLED
  // https://github.com/Makuna/NeoPixelBus
  #include <NeoPixelBus.h>
  
  const uint16_t PixelCount = 64;

  // three element pixels, in different order and speeds
  //NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> strip(PixelCount, NEO_PIN);
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> ledStrip(PixelCount, NEO_PIN);

  String ledValueR = String(0);
  String ledValueG = String(0);
  String ledValueB = String(0);

  uint8_t colorR = ledValueR.toInt() / 2;
  uint8_t colorG = ledValueG.toInt() / 2;
  uint8_t colorB = ledValueB.toInt() / 2;
#endif

/********************************
* for Servo Control
*********************************/
String servoValueL = String(90);
String servoValueR = String(90);

uint8_t servoNo;    // Set 0 or 1
uint16_t angle;     // min=0, max=180
uint16_t servoL = servoValueL.toInt();
uint16_t servoR = servoValueR.toInt();


/********************************
* for Motor Control
*********************************/
const uint8_t motorInterval = 30;  //[ms]
// --- robot_normal.ino


// duke32.cpp
/********************************
* for Servo Control
*********************************/
void servoInit(){
      // Setup timer and attach timer to a servo pin
      ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
      ledcAttachPin(Servo_PIN1, LEDC_CHANNEL_0);
    
      ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
      ledcAttachPin(Servo_PIN2, LEDC_CHANNEL_1);
}

void setServo(uint8_t servoNo, uint16_t angle){
      uint16_t pwmWidth;
    
      if(servoNo < 0){
            servoNo = 0;
      }else if(servoNo > 1){
            servoNo = 1;
      }
    
      if(angle < 0){
            angle = 0;
      }else if(angle > 180){
            angle = 180;
      }
    
      pwmWidth = ((srvMax - srvMin) / 180) * angle + srvMin;
    
      ledcWrite(servoNo, pwmWidth);
}

/********************************
* for Motor Control
*********************************/
void Motor_INIT(){
      pinMode(MTR_A1, OUTPUT);
      pinMode(MTR_A2, OUTPUT);
    //  pinMode(MTR_AE, OUTPUT);
      pinMode(MTR_B1, OUTPUT);
      pinMode(MTR_B2, OUTPUT);
    //  pinMode(MTR_BE, OUTPUT);
    
      digitalWrite(MTR_A1, LOW);
      digitalWrite(MTR_A2, LOW);
    //  digitalWrite(MTR_AE, LOW);
      digitalWrite(MTR_B1, LOW);
      digitalWrite(MTR_B2, LOW);
    //  digitalWrite(MTR_BE, LOW);
    
      // Setup timer and attach timer to a servo pin
      ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ_MTR, LEDC_TIMER_BIT_MTR);
      ledcAttachPin(MTR_AE, LEDC_CHANNEL_2);
    
      ledcSetup(LEDC_CHANNEL_3, LEDC_BASE_FREQ_MTR, LEDC_TIMER_BIT_MTR);
      ledcAttachPin(MTR_BE, LEDC_CHANNEL_3);
}
// --- duke32.cpp

void setMotorSpeed(uint8_t motorNo, int16_t speed){
      if(motorNo >= 2) return;
    
      int16_t abs = speed;
      if(abs < 0)   abs = -abs;
      if(abs > 255) abs = 255;
      ledcWrite(motorNo+LEDC_CHANNEL_2, abs);
    
      switch(motorNo) {
          case MotorL:
            digitalWrite(MTR_A1, (speed>0) ? HIGH:LOW);
            digitalWrite(MTR_A2, (speed<0) ? HIGH:LOW);
            break;
          case MotorR:
            digitalWrite(MTR_B1, (speed>0) ? HIGH:LOW);
            digitalWrite(MTR_B2, (speed<0) ? HIGH:LOW);
            break;
      }
}

struct { int16_t L; int16_t R;
} static const dir_table[7] = {
    //  L   R
  { 0,  0},  // STOP
  { 1,  1},  // FORWARD
  { 0,  1},  // LEFT
  { 1,  0},  // RIGHT
  {-1, -1},  // BACK
  {-1,  1},  // ROLL_LEFT
  { 1, -1},  // ROLL_RIGHT
};

void setRobot(uint8_t direction, uint8_t speed)
{
      setMotorSpeed(MotorL, speed * dir_table[direction].L);
      setMotorSpeed(MotorR, speed * dir_table[direction].R);
}

/*
 * ADC
 */
void ADC_INIT(){
      pinMode(ANA11, INPUT);
      pinMode(ANA12, INPUT);
}

uint16_t getAdc(uint8_t index)
{
  uint8_t adcCh[2] = {ANA11,ANA12};
      if(index>=2) return 0;
      return analogRead(adcCh[index]);
}

/*
 * DIO in
 */
void DIO_INIT(){
      pinMode(DIGI01, INPUT);
      pinMode(DIGI02, INPUT);
      pinMode(DIGI11, INPUT);
      pinMode(DIGI12, INPUT);
}

uint8_t getDigital(uint8_t index)
{
  uint8_t din_ch[4] = {DIGI01,DIGI02,DIGI11,DIGI12};
      if(index>=4) return 0;
      return digitalRead(din_ch[index]);
}

#ifdef useLED
/********************************
* for LED Control
*********************************/
#define colorSaturation 32
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

void SetNeoPixel(uint8_t index){
      RgbColor* npcolor = NULL;
      switch(index) {
            case 0: npcolor = &black; break;
            case 1: npcolor = &red;   break;
            case 2: npcolor = &green; break;
            case 3: npcolor = &blue;  break;
            case 4: npcolor = &white; break;
            default: return;
      }
    
      for(uint8_t i=0; i<PixelCount; i++){
            ledStrip.SetPixelColor(i, *npcolor);
      }
      ledStrip.Show();
}

void SetNeoPixelRGB(uint8_t r, uint8_t g, uint8_t b){
      if(r>32) r=32;
      if(g>32) g=32;
      if(b>32) b=32;
      RgbColor color(r,g,b);
    
      for(uint8_t i=0; i<PixelCount; i++){
            ledStrip.SetPixelColor(i, color);
      }
      ledStrip.Show();
}
#endif


#ifdef __AVR_ATmega328P__
#include <avr/wdt.h>
#endif

#if defined(_SAMD21_)
#define _Serial SerialUSB
#else
#define _Serial Serial
#endif

enum {
    RSP_BYTE    = 1,
    RSP_SHORT   = 2,
    RSP_LONG    = 3,
    RSP_FLOAT   = 4,
    RSP_DOUBLE  = 5,
    RSP_STRING  = 6,
};

#define getBufLen(n) (buffer+4+offsetIdx[n]+1),*(buffer+4+offsetIdx[n]+0)

void setup()
{
    #ifdef __AVR_ATmega328P__
    MCUSR = 0;
    wdt_disable();
    #endif
    
    // robot_normal.ino
    Serial.begin(115200);
    
    #ifdef useLED
      ledStrip.Begin();
    #endif
    
    // reset all Servo to initial Position
    servoInit();
    setServo(0, servoL);
    setServo(1, servoR);
    
    // reset all Motor to initial Setting
    Motor_INIT();
    delay(50);
    setMotorSpeed(MotorL, 0);
    setMotorSpeed(MotorR, 0);
    
    // init ADC
    ADC_INIT();
    
    // init DIO
    DIO_INIT();
    
    #ifndef PCMODE
    initWifi(mVersion, true);
    #else
    initWifi(mVersion, false);
    #endif
    
    _Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[256];  // 0xFF,0x55,len,cmd,
static uint8_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {},
  {'B','B',},
  {},
  {'B','S',},
  {},
  {},
  {'B',},
  {'B','B','B',},
  {'B','B',},
  {'B',},
  {'B',},
  {},
  {},
  {},
  {},
  {},
  {},
  {},
  {'s','s',},
};

uint8_t wifi_uart = 0;

void _write(uint8_t* dp, int count)
{
    #if defined(ESP32)
      if(wifi_uart)
        writeWifi(dp, count);
      else
    #endif
        _Serial.write(dp, count);
}

void _println(char* mes)
{
    #if defined(ESP32)
      if(wifi_uart)
        printlnWifi(mes);
      else
    #endif
        _Serial.println(mes);
}

int16_t _read(void)
{
      if(_Serial.available()>0) {
            wifi_uart = 0;
            return _Serial.read();
      }
    #if defined(ESP32)
      int ret = readWifi();
      if(ret != -1) {
            wifi_uart = 1;
            return ret;
      }
    #endif
      return -1;
}

static void parseData()
{
    uint8_t i;
    memset(offsetIdx, 0, sizeof(offsetIdx));
    if(buffer[3] < ITEM_NUM) {
        const char *ArgTypes = ArgTypesTbl[buffer[3]];
        uint16_t offset = 0;
        for(i = 0; i < ARG_NUM && ArgTypes[i]; i++) {
            offsetIdx[i] = offset;
            switch(ArgTypes[i]) {
                case 'B': offset += 1; break;
                case 'S': offset += 2; break;
                case 'L': offset += 4; break;
                case 'F': offset += 4; break;
                case 'D': offset += 8; break;
                case 's': offset += strlen((char*)buffer+4+offset)+1; break;
                case 'b': offset += buffer[4+offset]+1; break;
                default: break;
            }
            if(4+offset > _packetLen) return;
        }
    }
    
    switch(buffer[3]){
        case 2: setRobot(getByte(0),getByte(1));; callOK(); break;
        case 3: setRobot(0,0);; callOK(); break;
        case 4: setMotorSpeed(getByte(0),getShort(1));; callOK(); break;
        case 7: SetNeoPixel(getByte(0));; callOK(); break;
        case 8: SetNeoPixelRGB(getByte(0),getByte(1),getByte(2));; callOK(); break;
        case 9: setServo(getByte(0),getByte(1));; callOK(); break;
        case 10: sendByte((getDigital(getByte(0)))); break;
        case 11: sendShort((getAdc(getByte(0)))); break;
        case 17: sendString((statusWifi())); break;
        case 18: sendString((scanWifi())); break;
        case 19: sendByte((connectWifi(getString(0),getString(1)))); break;
        case 0xFE:  // firmware name
        _println("PC mode: " mVersion);
        break;
        case 0xFF:  // software reset
        #if defined(__AVR_ATmega328P__)
        wdt_enable(WDTO_15MS);
        while(1);
        #elif defined(_SAMD21_)
        NVIC_SystemReset();
        #elif defined(ESP32)
        ESP.restart();
        #endif
        break;
        //### CUSTOMIZED ###
        #ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
        #define CMD_CHECKREMOTEKEY  0x80
        case CMD_CHECKREMOTEKEY:
        sendRemote();
        break;
        #endif
    }
}

static uint8_t _index = 0;
void loop()
{
    int16_t c;
    while((c=_read()) >= 0) {
        //_Serial.write(a)  // for debug
        buffer[_index++] = c;
        
        switch(_index) {
            case 1:
            _packetLen = 4;
            if(c != 0xff)
            _index = 0;
            break;
            case 2:
            if(c != 0x55) 
            _index = 0;
            break;
            case 3:
            _packetLen = 3+c;
            break;
        }
        if(_index >= _packetLen) {
            parseData();
            _index = 0;
        }
        if(_index >= sizeof(buffer)) {
            _index = 0;
        }
    }
    
    #ifndef PCMODE
      sendNotifyArduinoMode();
    #endif
    remote.updateRemote();
    
}

union floatConv { 
    float _float;
    uint8_t _byte[4];
} conv;

union doubleConv {
    double _double;
    uint8_t _byte[8];
};

uint8_t getByte(uint8_t n)
{
    return buffer[4+offsetIdx[n]];
}

int16_t getShort(uint8_t n)
{
    uint8_t x = 4+offsetIdx[n];
    return buffer[x+0]|((uint32_t)buffer[x+1]<<8);
}
int32_t getLong(uint8_t n)
{
    uint8_t x = 4+offsetIdx[n];
    return buffer[x+0]|((uint32_t)buffer[x+1]<<8)|((uint32_t)buffer[x+2]<<16)|((uint32_t)buffer[x+3]<<24);
}

float getFloat(uint8_t n)
{
    uint8_t x = 4+offsetIdx[n];
    union floatConv conv;
    for(uint8_t i=0; i<4; i++) {
        conv._byte[i] = buffer[x+i];
    }
    return conv._float;
}

double getDouble(uint8_t n)
{
    uint8_t x = 4+offsetIdx[n];
    union doubleConv conv;
    for(uint8_t i=0; i<8; i++) {
        conv._byte[i] = buffer[x+i];
    }
    return conv._double;
}

char* getString(uint8_t n)
{
    uint8_t x = 4+offsetIdx[n];
    return (char*)buffer+x;
}

static void callOK()
{
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = (uint8_t)0;
    _write(buffer, dp-buffer);
}

static void sendByte(uint8_t data)
{
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+sizeof(uint8_t);
    *dp++ = RSP_BYTE;
    *dp++ = data;
    _write(buffer, dp-buffer);
}

static void sendShort(uint16_t data)
{
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+sizeof(uint16_t);
    *dp++ = RSP_SHORT;
    *dp++ = data&0xff;
    *dp++ = data>>8;
}

static void sendLong(uint32_t data)
{
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+sizeof(uint32_t);
    *dp++ = RSP_LONG;
    *dp++ = data&0xff;
    *dp++ = data>>8;
    *dp++ = data>>16;
    *dp++ = data>>24;
    _write(buffer, dp-buffer);
}

static void sendFloat(float data)
{
    union floatConv conv;
    conv._float = data;
    
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+sizeof(float);
    *dp++ = RSP_FLOAT;
    *dp++ = conv._byte[0];
    *dp++ = conv._byte[1];
    *dp++ = conv._byte[2];
    *dp++ = conv._byte[3];
    _write(buffer, dp-buffer);
}

static void sendDouble(double data)
{
    union doubleConv conv;
    conv._double = data;
    
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+sizeof(double);
    *dp++ = RSP_DOUBLE;
    for(uint8_t i=0; i<8; i++) {
        *dp++ = conv._byte[i];
    }
    _write(buffer, dp-buffer);
}

static void sendString(String s)
{
    uint8_t l = s.length();
    
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+l;
    *dp++ = RSP_STRING;
    for(uint8_t i=0; i<l; i++) {
        *dp++ = s.charAt(i);
    }
    _write(buffer, dp-buffer);
}

//### CUSTOMIZED ###
#ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
static void sendRemote(void)
{
    uint16_t data;
    
    uint8_t* dp = buffer;
    *dp++ = 0xff;
    *dp++ = 0x55;
    *dp++ = 1+1+2+2;
    *dp++ = CMD_CHECKREMOTEKEY;
    *dp++ = remote.checkRemoteKey();
    data = remote.x; *dp++ = data&0xff; *dp++ = data>>8;
    data = remote.y; *dp++ = data&0xff; *dp++ = data>>8;
    _write(buffer, dp-buffer);
}
#endif
