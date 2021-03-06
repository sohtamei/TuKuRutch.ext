// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "Duke32AIO 1.0"

#include <Adafruit_NeoPixel.h>
#include "TukurutchEsp.h"

WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

#define P_MOTOR0_IN1  13
#define P_MOTOR0_IN2  12
#define P_MOTOR0_EN   18
#define P_MOTOR1_IN1  14
#define P_MOTOR1_IN2  15
#define P_MOTOR1_EN   19
#define LEDC_MOTOR0  2
#define LEDC_MOTOR1  3

#define MotorL 0
#define MotorR 1

#define LEDC_SERVO0  0
#define LEDC_SERVO1  1

#define P_NEOPIXEL     27
#define PixelCount     64

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PixelCount, P_NEOPIXEL, NEO_GRB + NEO_KHZ800);
void SetNeoPixelRGB(uint8_t r, uint8_t g, uint8_t b)
{
      for(uint8_t i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, r, g, b);
            strip.show();
            delay(10);
      }
}

void SetNeoPixel(uint8_t index){
      int r,g,b;
      switch(index) {
            case 0: r=0;   g=0;   b=0;   break;  // black
            case 1: r=255; g=0;   b=0;   break;  // red
            case 2: r=0;   g=255; b=0;   break;  // green
            case 3: r=128; g=128; b=0;   break;  // yellow
            case 4: r=0;   g=0;   b=255; break;  // blue
            case 5: r=128; g=0;   b=128; break;  // magenta
            case 6: r=0;   g=128; b=128; break;  // cyan
            case 7: r=128; g=128; b=128; break;  // white
            default: return;
      }
      SetNeoPixelRGB(r, g, b);
}

void _setLED(uint8_t onoff)
{
      if(onoff)
        SetNeoPixel(4);
      else
        SetNeoPixel(0);
}

const struct {uint8_t ledc; uint8_t port;} servoTable[] = {{0,23},{1,4}};
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

void _setMotor(uint8_t motorNo, int16_t speed){
      if(motorNo >= 2) return;
    
      int16_t abs = speed;
      if(abs < 0)   abs = -abs;
      if(abs > 100) abs = 100;
      ledcWrite(LEDC_MOTOR0+motorNo, (abs*255)/100);
    
      switch(motorNo) {
          case MotorL:
            digitalWrite(P_MOTOR0_IN1, (speed>0) ? HIGH:LOW);
            digitalWrite(P_MOTOR0_IN2, (speed<0) ? HIGH:LOW);
            break;
          case MotorR:
            digitalWrite(P_MOTOR1_IN1, (speed>0) ? HIGH:LOW);
            digitalWrite(P_MOTOR1_IN2, (speed<0) ? HIGH:LOW);
            break;
      }
}

struct { int16_t L; int16_t R;
} static const dir_table[7] = {
    //  L   R
  { 0,  0},  // 0:STOP
  { 1,  1},  // 1:FORWARD
  { 0,  1},  // 2:LEFT
  { 1,  0},  // 3:RIGHT
  {-1, -1},  // 4:BACK
  {-1,  1},  // 5:ROLL_LEFT
  { 1, -1},  // 6:ROLL_RIGHT
};

void _setCar(uint8_t direction, uint8_t speed)
{
      _setMotor(MotorL, speed * dir_table[direction].L);
      _setMotor(MotorR, speed * dir_table[direction].R);
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
    
    Serial.begin(115200);
    
    strip.begin();
    SetNeoPixel(0);
    
    // DC motor
    digitalWrite(P_MOTOR0_IN1, LOW);
    digitalWrite(P_MOTOR0_IN2, LOW);
    digitalWrite(P_MOTOR1_IN1, LOW);
    digitalWrite(P_MOTOR1_IN2, LOW);
    pinMode(P_MOTOR0_IN1, OUTPUT);
    pinMode(P_MOTOR0_IN2, OUTPUT);
    pinMode(P_MOTOR1_IN1, OUTPUT);
    pinMode(P_MOTOR1_IN2, OUTPUT);
    ledcSetup(servoTable[0].ledc, 50/*Hz*/, 12/*bit*/);
    ledcSetup(servoTable[1].ledc, 50/*Hz*/, 12/*bit*/);
    
    // servo motor
    ledcSetup(LEDC_MOTOR0, 255/*Hz*/, 8/*bit*/);
    ledcSetup(LEDC_MOTOR1, 255/*Hz*/, 8/*bit*/);
    ledcAttachPin(P_MOTOR0_EN, LEDC_MOTOR0);
    ledcAttachPin(P_MOTOR1_EN, LEDC_MOTOR1);
    
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
  {},
  {},
  {},
  {},
  {'B','B',},
  {'B',},
  {'B','S',},
  {},
  {},
  {},
  {},
  {},
  {},
  {},
  {},
  {},
  {'B','B',},
  {'B','S',},
  {},
  {},
  {'B','B',},
  {},
  {'B',},
  {'B','B','B',},
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
        case 7: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 8: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 9: sendShort((getAdc1(getByte(0),getShort(1)))); break;
        case 19: _setCar(getByte(0),getByte(1));; callOK(); break;
        case 20: _setMotor(getByte(0),getShort(1));; callOK(); break;
        case 21: _setCar(0,0);_setServo(0,0,1);_setServo(1,0,1);; callOK(); break;
        case 23: _setServo(getByte(0),getByte(1),0);; callOK(); break;
        case 25: SetNeoPixel(getByte(0));; callOK(); break;
        case 26: SetNeoPixelRGB(getByte(0),getByte(1),getByte(2));; callOK(); break;
        case 28: sendString((statusWifi())); break;
        case 29: sendString((scanWifi())); break;
        case 30: sendByte((connectWifi(getString(0),getString(1)))); break;
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
