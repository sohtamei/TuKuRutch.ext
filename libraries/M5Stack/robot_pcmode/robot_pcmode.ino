// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "M5STACK 1.0"

#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
#include <M5Stack.h>
#include "TukurutchEsp.h"


WebsocketsServer wsServer;

#define numof(a) (sizeof(a)/sizeof((a)[0]))

void _setLED(uint8_t onoff)
{;}

uint8_t _getSw(uint8_t button)
{
      switch(button) {
          case 0: return M5.BtnA.isPressed();
          case 1: return M5.BtnB.isPressed();
          case 2: return M5.BtnC.isPressed();
      }
      return 0;
}

float getIMU(uint8_t index)
{
  float data[3] = {0};
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
      M5.Speaker.tone(sound, ms);
      delay(ms);
      M5.Speaker.mute();
}

void _beep(void)
{
      M5.Speaker.beep();
      delay(100);
      M5.Speaker.mute();
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
    
    M5.begin(true, true, true); // init lcd, sd card, serial
    M5.Power.begin();    // use battery
    M5.IMU.Init();
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    
    M5.Lcd.setCursor(0, 0);
    if(_getSw(0)) {
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
        case 9: sendShort((getAdc1(getByte(0),getShort(1)))); break;
        case 11: M5.Lcd.setTextColor(getShort(0));M5.Lcd.setTextSize(getByte(1));; callOK(); break;
        case 12: M5.Lcd.setCursor(getShort(0),getShort(1));; callOK(); break;
        case 13: M5.Lcd.print(getString(0));; callOK(); break;
        case 14: M5.Lcd.println(getString(0));; callOK(); break;
        case 15: M5.Lcd.drawString(getString(0),getShort(1),getShort(2),getByte(3));; callOK(); break;
        case 16: M5.Lcd.fillScreen(getShort(0)); M5.Lcd.setCursor(0,0);; callOK(); break;
        case 18: sendString((statusWifi())); break;
        case 19: sendString((scanWifi())); break;
        case 20: sendByte((connectWifi(getString(0),getString(1)))); break;
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
    //delay(50);
    
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
