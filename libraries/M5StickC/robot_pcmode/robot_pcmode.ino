// copyright to SohtaMei 2019.

#define PCMODE

//#define _M5StickC
#define _M5StickCPlus

#ifdef _M5StickC
  #define mVersion "M5StickC 1.0"
  #include <M5StickC.h>
#else 
  #define mVersion "M5StickCP 1.0"
  #include <M5StickCPlus.h>
#endif

#include "TukurutchEsp.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include <ArduinoWebsockets.h>
using namespace websockets;
WebsocketsServer wsServer;
#define ENABLE_WEBSOCKET

#define ROVER_ADDRESS	0X38
#define P_LED			10

#define numof(a) (sizeof(a)/sizeof((a)[0]))

esp_adc_cal_characteristics_t adc_chars;
uint16_t _getAdc1(uint8_t idx, uint16_t count)
{
      if(count == 0) count = 1;
    
      adc1_config_width(ADC_WIDTH_BIT_12);
      esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100/*VREF*/, &adc_chars);
      adc1_config_channel_atten((adc1_channel_t)idx, ADC_ATTEN_DB_11);
      uint32_t sum = 0;
      for(int i = 0; i < count; i++)
        sum += adc1_get_raw((adc1_channel_t)idx);
    
      return esp_adc_cal_raw_to_voltage(sum/count, &adc_chars);
}

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
        //case 2: return M5.BtnC.isPressed();
      }
      return 0;
}

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
            if(data==0) pwmWidth=0;
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
  { 0,  0},  // STOP
  { 1,  1},  // FORWARD
  { 0,  1},  // LEFT
  { 1,  0},  // RIGHT
  {-1, -1},  // BACK
  {-1,  1},  // ROLL_LEFT
  { 1, -1},  // ROLL_RIGHT
};

void _setCar(uint8_t direction, uint8_t speed)
{
  _setServo(0, speed * dir_table[direction].L, 1);
  _setServo(1, speed * dir_table[direction].R, 1);
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

// Rover C

void RoverC_Init(void)    
{
  Wire.begin(0,26,100);		//sda 0, scl 26
}

void Send_iic(uint8_t Register, int16_t Speed)
{
  if(Speed >  100) Speed =  100;
  if(Speed < -100) Speed = -100;
  Wire.beginTransmission(ROVER_ADDRESS);
  Wire.write(Register);
  Wire.write(Speed);
  Wire.endTransmission();
}

void setRoverC(int16_t F_L, int16_t F_R, int16_t R_L, int16_t R_R)
{
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

void onConnect(String ip)
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println(ip);
  wsServer.listen(PORT_WEBSOCKET);
}


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

M5.begin(true, true, true); // init lcd, power, serial
M5.IMU.Init();
M5.Lcd.setRotation(3);

M5.Lcd.fillScreen(BLACK);
M5.Lcd.setTextSize(2);

M5.Lcd.setCursor(0, 0);
if(M5.BtnA.isPressed()) {
  M5.Lcd.println("ESP SmartConfig");
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  while (!WiFi.smartConfigDone()) {
        delay(2000);
        _setLED(1);
        _tone(T_C5, 100);
        _setLED(0);
  }
} else {
  #ifdef PCMODE
    M5.Lcd.println("PC mode: " mVersion);
  #else
    M5.Lcd.println("Arduino mode: " mVersion);
  #endif
}

ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

RoverC_Init();
Serial.begin(115200);
#ifndef PCMODE
initWifi(mVersion, true, onConnect);
#else
initWifi(mVersion, false, onConnect);
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
  {'B',},
  {'B',},
  {'S','S',},
  {},
  {'B',},
  {'B','B',},
  {'B',},
  {'B','S',},
  {},
  {'S','B',},
  {'S','S',},
  {'s',},
  {'s',},
  {'s','S','S','B',},
  {'S',},
  {},
  {},
  {'B','B',},
  {'B','S',},
  {},
  {},
  {'B','B',},
  {},
  {},
  {'S','S','S','S',},
  {'S','S','S',},
  {'B','B',},
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
  if(wifi_uart == 1) {
    writeWifi(dp, count);
    //_Serial.print("\n"); for(int i=0; i<count; i++) _Serial.printf("%02X",dp[i]); _Serial.print("\n");   // for debug
#ifdef ENABLE_WEBSOCKET
  } else if(wifi_uart == 2) {
    _packetLen = count;
#endif
  } else
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
case 2: _setLED(getByte(0));; callOK(); break;
case 3: sendFloat((getIMU(getByte(0)))); break;
case 4: _tone(getShort(0),getShort(1));; callOK(); break;
case 5: _beep();; callOK(); break;
case 6: sendByte((_getSw(getByte(0)))); break;
case 7: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
case 8: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
case 9: sendShort((_getAdc1(getByte(0),getShort(1)))); break;
case 11: M5.Lcd.setTextColor(getShort(0));M5.Lcd.setTextSize(getByte(1));; callOK(); break;
case 12: M5.Lcd.setCursor(getShort(0),getShort(1));; callOK(); break;
case 13: M5.Lcd.print(getString(0));; callOK(); break;
case 14: M5.Lcd.println(getString(0));; callOK(); break;
case 15: M5.Lcd.drawString(getString(0),getShort(1),getShort(2),getByte(3));; callOK(); break;
case 16: M5.Lcd.fillScreen(getShort(0));; callOK(); break;
case 19: _setCar(getByte(0),getByte(1));; callOK(); break;
case 20: _setServo(getByte(0),getShort(1),1);; callOK(); break;
case 21: _setCar(0,0);; callOK(); break;
case 23: _setServo(getByte(0),getByte(1),0);; callOK(); break;
case 26: setRoverC(getShort(0),getShort(1),getShort(2),getShort(3));; callOK(); break;
case 27: setRoverC_XYR(getShort(0),getShort(1),getShort(2));; callOK(); break;
case 28: moveRoverC(getByte(0),getByte(1));; callOK(); break;
case 31: sendString((statusWifi())); break;
case 32: sendString((scanWifi())); break;
case 33: sendByte((connectWifi(getString(0),getString(1)))); break;
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
//_Serial.printf("%02x",c);  // for debug
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

  loopWebSocket();
#ifndef PCMODE
  sendNotifyArduinoMode();
#endif
  M5.update();  // update button and speaker
  delay(50);

}

#ifdef ENABLE_WEBSOCKET
uint8_t connected = 0;
static WebsocketsClient _client;

void onEventsCallback(WebsocketsEvent event, String data) {
  if(event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if(event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
    connected = 0;
  } else if(event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if(event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

void onMessageCallback(WebsocketsMessage msg) {
  _packetLen = msg.length();
  memcpy(buffer, msg.c_str(), _packetLen);
  wifi_uart = 2;
  parseData();
  _client.sendBinary((char*)buffer, _packetLen);
}

static void loopWebSocket(void)
{
  if(!wsServer.available()) return;
  if(wsServer.poll()) {
    connected = 1;
    Serial.println("connected");
    _client = wsServer.accept();
    _client.onMessage(onMessageCallback);
    _client.onEvent(onEventsCallback);
  }
  if(!connected || !_client.available()) return;
  _client.poll();
  return;
}
#endif

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
_write(buffer, dp-buffer);
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
