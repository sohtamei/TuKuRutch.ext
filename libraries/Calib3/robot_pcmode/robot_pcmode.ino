// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "Calib1.0"

#include <M5Atom.h>
#include <HTTPClient.h>
#include <libb64/cencode.h>
#include <libb64/cdecode.h>
#include <Preferences.h>
#include "TukurutchEsp.h"


WebsocketsServer wsServer;
static Preferences preferencesRobot;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

uint32_t last_enc[2] = {0,0};
int32_t duration[2] = {0,0};
uint8_t cnt[2] = {0,0};
#define CNT_MAX		10

#define P_ENC0A		22
#define P_ENC0B		19
#define P_ENC1A		23
#define P_ENC1B		33
#define P_LED		21

#if 1
// ATOM
#define P_SRV0		32
#define P_SRV1		26
#else
// M5CAMERA
#define P_SRV0		13
#define P_SRV1		4
#endif

enum {
      CAL0P = 0,
      CAL0M,
      CAL1P,
      CAL1M,
};

enum {
      PWM0,
      PWM25,
      PWM50,
      PWM75,
      PWM100,
};

struct calibrate {
      uint16_t id;
      uint16_t ver;
      uint16_t T;
      uint16_t _reserve;
      uint16_t pwm[4][4];
       int16_t maxOffset[2];
       int16_t speedMax[4];
} static cal;

static const struct calibrate calInit = {
      0,
      1,
      18,
      0,
  {{308,314,319,336},{297,292,287,270},{295,290,285,267},{309,315,320,339}},
  {0,0},
  {2000,-2000,2000,-2000},
};

enum {
      SERVO_IDLE,
      SERVO_STOP_REQ,
      SERVO_OFF_REQ,
};
static uint8_t servo_stt = 0;
static uint32_t servo_time = 0;

void _setLED(uint8_t onoff)
{
      digitalWrite(P_LED, !onoff);
}

uint8_t ledStatus = 0;
static void irq_int0(void)
{
      cnt[0]++;
      if(cnt[0] >= CNT_MAX) {
            cnt[0] = 0;
        
            uint32_t cur = micros();
            uint32_t d = (cur - last_enc[0]) & ~0x80000000;
            duration[0] = digitalRead(P_ENC0B) ? d: -d;
            last_enc[0] = cur;
        
            ledStatus ^= 1;
            _setLED(ledStatus);
      }
      attachInterrupt(digitalPinToInterrupt(P_ENC0A), irq_int0, FALLING);  // RISING
}

static void irq_int1(void)
{
      cnt[1]++;
      if(cnt[1] >= CNT_MAX) {
            cnt[1] = 0;
        
            uint32_t cur = micros();
            uint32_t d = (cur - last_enc[1]) & ~0x80000000;
            duration[1] = digitalRead(P_ENC1B) ? d: -d;
            last_enc[1] = cur;
      }
      attachInterrupt(digitalPinToInterrupt(P_ENC1A), irq_int1, FALLING);  // RISING
}

static int16_t _getSpeed(uint8_t index)
{
      int32_t ret = duration[index];
      if(!ret) return 0;
      duration[index] = 0;
      return (1000000000L/(50L/CNT_MAX))/ret;  // us -> m rps
    //  return ret;
}

//////////////////////////////
#define PWM_MIN     143
#define PWM_NEUTRAL 307
#define PWM_MAX     471
const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{8,P_SRV0},{9,P_SRV1}};
void _setPwm(uint8_t idx, int16_t data)
{
      ledcAttachPin(servoTable[idx].port, servoTable[idx].ledc);
      ledcWrite(servoTable[idx].ledc, data);
}

static const uint16_t maxOffsetTbl[] = {164,83,70,63,58,54,51,48,45,43,41,40,38,37,35,34,33,32,31,30,29,28,27,26,26,25,24,24,23,22,22,21,21,20,19,19,18,18,17,17,16,16,16,15,15,14,14,14,13,13,12};

int16_t limitOffset(int16_t offset)
{
           if(offset >= (int) numof(maxOffsetTbl)) offset = numof(maxOffsetTbl)-1;
      else if(offset <= (int)-numof(maxOffsetTbl)) offset = -(numof(maxOffsetTbl)-1);
      return offset;
}

void _setMotor(int16_t left, int16_t right/* -4 ~ +4 */,
  int16_t calib,   /* -50(Left) ~ +50(right) */
  int16_t duration)
{
           if(left >  PWM100) left =  PWM100;
      else if(left < -PWM100) left = -PWM100;
    
           if(right >  PWM100) right =  PWM100;
      else if(right < -PWM100) right = -PWM100;
    
      uint16_t pwmL = 0;
      uint16_t pwmR = 0;
      int data = 0;
      int calib4 = (calib + ((calib>=0)?2:-2) )/4;
      switch(left) {
          case PWM100:
            data = limitOffset(cal.maxOffset[0] - calib*2);
            if(data >= 0)
              pwmL = cal.pwm[CAL0P][PWM0] + maxOffsetTbl[data];
            else
              pwmL = PWM_MAX;
            break;
        
          case -PWM100:
            data = limitOffset(cal.maxOffset[1] - calib*2);
            if(data >= 0)
              pwmL = cal.pwm[CAL0M][PWM0] - maxOffsetTbl[data];
            else
              pwmL = PWM_MIN;
            break;
        
          case PWM0:
            if(calib == 0)
              pwmL = (cal.pwm[CAL0P][PWM0] + cal.pwm[CAL0M][PWM0])/2;
            else if(calib > 0)
              pwmL = cal.pwm[CAL0P][PWM0] + calib4;
            else
              pwmL = cal.pwm[CAL0M][PWM0] + calib4;
            break;
        
          default:
            if(left >= 0)
              pwmL = cal.pwm[CAL0P][ left] + calib4;
            else
              pwmL = cal.pwm[CAL0M][-left] + calib4;
            break;
      }
    
      switch(right) {
          case PWM100:
            data = limitOffset(cal.maxOffset[0] - calib*2);
            if(data >= 0)
              pwmR = PWM_MIN;
            else
              pwmR = cal.pwm[CAL1P][PWM0] - maxOffsetTbl[-data];
            break;
        
          case -PWM100:
            data = limitOffset(cal.maxOffset[1] - calib*2);
            if(data >= 0)
              pwmR = PWM_MAX;
            else
              pwmR = cal.pwm[CAL1M][PWM0] + maxOffsetTbl[-data];
            break;
        
          case PWM0:
            if(calib == 0)
              pwmR = (cal.pwm[CAL1P][PWM0] + cal.pwm[CAL1M][PWM0])/2;
            else if(calib > 0)
              pwmR = cal.pwm[CAL1M][PWM0] + calib4;
            else
              pwmR = cal.pwm[CAL1P][PWM0] + calib4;
            break;
        
          default:
            if(right >= 0)
              pwmR = cal.pwm[CAL1P][ right] + calib4;
            else
              pwmR = cal.pwm[CAL1M][-right] + calib4;
            break;
      }
    //Serial.printf("%d %d\n", pwmL, pwmR);
      _setPwm(0, pwmL);
      _setPwm(1, pwmR);
    
      if(duration) {
            servo_stt = SERVO_STOP_REQ;
            servo_time = millis() + duration;
        
      } else if(left == PWM0 && right == PWM0) {
            servo_stt = SERVO_OFF_REQ;
            servo_time = millis() + 2000;
        
      } else {
            servo_stt = SERVO_IDLE;
            servo_time = 0;
      }
}

void _stopServo(void)
{
      servo_stt = SERVO_IDLE;
      servo_time = 0;
      _setPwm(0, 0);
      _setPwm(1, 0);
}

enum {
      CMD_STOP = 0,
      CMD_FORWARD,
      CMD_LEFT,
      CMD_RIGHT,
      CMD_BACK,
      CMD_ROLL_LEFT,
      CMD_ROLL_RIGHT,
      CMD_CALIBRATION,
};

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

void _setCar(uint8_t direction, uint16_t speed, int16_t calib, int16_t duration)
{
  if(direction >= CMD_CALIBRATION) {
        _setPwm(0, PWM_NEUTRAL);
        _setPwm(1, PWM_NEUTRAL);
  } else {
        _setMotor(speed*dir_table[direction].L, speed*dir_table[direction].R, calib, duration);
  }
}

/////////////////////////////////////////////

static char* _downloadCal(int16_t id, char* base64)
{
  if(base64 && base64[0]) {
        int len = base64_decode_chars(base64, strlen(base64), strBuf);
        if(len == sizeof(cal)) {
              memcpy((char*)&cal, strBuf, len);
              preferencesRobot.putBytes("calib", &cal, sizeof(cal));
              snprintf(strBuf, sizeof(strBuf), "OK");
        } else {
              snprintf(strBuf, sizeof(strBuf), "Error");
        }
        return strBuf;
  }

  char url[50];
  if(id==0) id=cal.id;
  snprintf(url, sizeof(url), "http://sohta02.web.fc2.com/calib/M5CC%04d.txt", id);
  memset(strBuf, 0, sizeof(strBuf));
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        if(base64_decode_expected_len(payload.length()) >= sizeof(strBuf)) goto Error;
    
        int len = base64_decode_chars((const char*)payload.c_str(), payload.length(), strBuf);
        if(len != sizeof(cal)) goto Error;
        memcpy((char*)&cal, strBuf, len);
        preferencesRobot.putBytes("calib", &cal, sizeof(cal));
    
        uint16_t i;
        for(i=0; i<len; i++) Serial.printf("%02x", ((uint8_t*)&cal)[i]);
        Serial.println();
        snprintf(strBuf, sizeof(strBuf), "OK:%s", payload.c_str());
  } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    Error:
        snprintf(strBuf, sizeof(strBuf), "Error");
  }
  http.end();
  return strBuf;
}

char* _getCal(void)
{
  base64_encode_chars((char*)&cal, sizeof(cal), strBuf);
  return strBuf;
}

void _setCID(int16_t id, int16_t ver, int16_t T)
{
  cal.id = id;
  cal.ver = ver;
  cal.T = T;
  preferencesRobot.putBytes("calib", &cal, sizeof(cal));
}

void _setCPwm(uint8_t index, int16_t pwm0, int16_t pwm1, int16_t pwm2, int16_t pwm3)
{
  cal.pwm[index][0] = pwm0;
  cal.pwm[index][1] = pwm1;
  cal.pwm[index][2] = pwm2;
  cal.pwm[index][3] = pwm3;
  preferencesRobot.putBytes("calib", &cal, sizeof(cal));
}

void _setCMaxOffset(int16_t maxOffset0, int16_t maxOffset1)
{
  cal.maxOffset[0] = maxOffset0;
  cal.maxOffset[1] = maxOffset1;
  preferencesRobot.putBytes("calib", &cal, sizeof(cal));
}

void _setCSpeedMax(int16_t speedMax0P, int16_t speedMax0M, int16_t speedMax1P, int16_t speedMax1M)
{
  cal.speedMax[0] = speedMax0P;
  cal.speedMax[1] = speedMax0M;
  cal.speedMax[2] = speedMax1P;
  cal.speedMax[3] = speedMax1M;
  preferencesRobot.putBytes("calib", &cal, sizeof(cal));
}

void onConnect(String ip)
{
  _setLED(1);

  wsServer.listen(PORT_WEBSOCKET);
  Serial.println(ip);
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

_setLED(0);

Serial.begin(115200);

pinMode(P_ENC0A, INPUT);
pinMode(P_ENC0B, INPUT);
attachInterrupt(digitalPinToInterrupt(P_ENC0A), irq_int0, FALLING);
pinMode(P_ENC1A, INPUT);
pinMode(P_ENC1B, INPUT);
attachInterrupt(digitalPinToInterrupt(P_ENC1A), irq_int1, FALLING);

// ServoCar
ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);

initWifi(mVersion, false, onConnect);

preferencesRobot.begin("M5CameraCar", false);
if(preferencesRobot.getBytes("calib", &cal, sizeof(cal)) < sizeof(cal)) {
      Serial.println("init calib");
      //memset(&cal, 0, sizeof(cal));
      memcpy(&cal, &calInit, sizeof(cal));
      preferencesRobot.putBytes("calib", &cal, sizeof(cal));
} else {
      Serial.println(cal.id);
      Serial.println(cal.ver);
      for(uint8_t i=0; i<4; i++)
        Serial.printf("%d %d %d %d %d %d\n", cal.pwm[i][0], cal.pwm[i][1], cal.pwm[i][2], cal.pwm[i][3], cal.speedMax[i]);
      Serial.printf("%d %d\n", cal.maxOffset[0], cal.maxOffset[1]);
}

_Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[256];  // 0xFF,0x55,len,cmd,
static uint8_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {'B','S','S','S',},
  {'S','S','S','S',},
  {'B','S',},
  {},
  {},
  {'B',},
  {'S','S','S',},
  {'B','S','S','S','S',},
  {'S','S',},
  {'S','S','S','S',},
  {'S','s',},
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
    case 1: _setCar(getByte(0),getShort(1),getShort(2),getShort(3));; callOK(); break;
    case 2: _setMotor(getShort(0),getShort(1),getShort(2),getShort(3));; callOK(); break;
    case 3: _setPwm(getByte(0),getShort(1));; callOK(); break;
    case 4: _stopServo();; callOK(); break;
    case 6: sendShort((_getSpeed(getByte(0)))); break;
    case 7: _setCID(getShort(0),getShort(1),getShort(2));; callOK(); break;
    case 8: _setCPwm(getByte(0),getShort(1),getShort(2),getShort(3),getShort(4));; callOK(); break;
    case 9: _setCMaxOffset(getShort(0),getShort(1));; callOK(); break;
    case 10: _setCSpeedMax(getShort(0),getShort(1),getShort(2),getShort(3));; callOK(); break;
    case 11: sendString((_downloadCal(getShort(0),getString(1)))); break;
    case 12: sendString((_getCal())); break;
    case 14: sendString((statusWifi())); break;
    case 15: sendString((scanWifi())); break;
    case 16: sendByte((connectWifi(getString(0),getString(1)))); break;
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
  if(servo_stt) {
        int32_t d = servo_time - millis();
        if(d <= 0) {
              switch(servo_stt) {
                  case SERVO_STOP_REQ:
                    servo_stt = SERVO_OFF_REQ;
                    servo_time = millis() + 2000;
                    _setPwm(0, (cal.pwm[CAL0P][PWM0] + cal.pwm[CAL0M][PWM0])/2);
                    _setPwm(1, (cal.pwm[CAL1P][PWM0] + cal.pwm[CAL1M][PWM0])/2);
                    break;
            
                  case SERVO_OFF_REQ:
                  default:
                    servo_stt = SERVO_IDLE;
                    servo_time = 0;
                    _setPwm(0, 0);
                    _setPwm(1, 0);
                    break;
              }
        }
  }

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
