// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "M5Atom 1.0"

#include "TukurutchEsp.h"
#include "main.h"

#include <Wire.h>
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
    RSP_BUF     = 7,
};

#define getBufLen(n) (buffer+4+offsetIdx[n]+1),*(buffer+4+offsetIdx[n]+0)

void setup()
{
    #ifdef __AVR_ATmega328P__
      MCUSR = 0;
      wdt_disable();
    #endif
    _setup(mVersion);  _Serial.println("PC mode: " mVersion);
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
  {},
  {},
  {'B',},
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
  {},
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
      _write((uint8_t*)mes, strlen(mes));
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

#define CMD_MIN  0x81
#define CMD_MAX  0x84
static const char ArgTypesTbl2[][ARG_NUM] = {
  {'B','B'},		// 0x81:wire_begin     (SDA, SCL)
  {'B','b'},		// 0x82:wire_write     (adrs, [DATA])          ret:0-OK
  {'B','b','B'},	// 0x83:wire_writeRead (adrs, [DATA], readNum) ret:[DATA]-OK, NULL-ERROR
  {'B','B'},		// 0x84:wire_read      (adrs, readNum)         ret:[DATA]-OK, NULL-ERROR
};

static void sendWireRead(int adrs, int num)
{
      Wire.requestFrom(adrs, num);
      if(Wire.available() < num) {
            callOK();
      } else {
            for(int i = 0; i < num; i++)
              buffer[i] = Wire.read();
            sendBin(buffer, num);
      }
}

static void parseData()
{
      uint8_t cmd = buffer[3];
      const char *ArgTypes = NULL;
      if(cmd < ITEM_NUM) {
             ArgTypes = ArgTypesTbl[cmd];
      } else if(cmd >= CMD_MIN && cmd <= CMD_MAX) {
             ArgTypes = ArgTypesTbl2[cmd-CMD_MIN];
      }
    
      uint8_t i;
      memset(offsetIdx, 0, sizeof(offsetIdx));
      if(ArgTypes) {
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
    
      switch(cmd){
        case 2: _setLED(getByte(0));; callOK(); break;
        case 3: sendFloat((getIMU(getByte(0)))); break;
        case 6: sendByte((_getSw(getByte(0)))); break;
        case 7: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 8: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 9: sendShort((getAdc1(getByte(0),getShort(1)))); break;
        case 19: _setCar(getByte(0),getByte(1));; callOK(); break;
        case 20: _setServo(getByte(0),getShort(1),1);; callOK(); break;
        case 21: _setCar(0,0);; callOK(); break;
        case 23: _setServo(getByte(0),getByte(1),0);; callOK(); break;
        case 31: sendString((statusWifi())); break;
        case 32: sendString((scanWifi())); break;
        case 33: sendByte((connectWifi(getString(0),getString(1)))); break;
        #ifdef __AVR_ATmega328P__
          case 0x81: Wire.begin(); callOK(); break;
        #else
          case 0x81: Wire.begin(getByte(0),getByte(1)); callOK(); break;
        #endif
          case 0x82: Wire.beginTransmission(getByte(0)); Wire.write(getBufLen(1)); sendByte(Wire.endTransmission()); break;
          case 0x83: Wire.beginTransmission(getByte(0)); Wire.write(getBufLen(1)); Wire.endTransmission(false); sendWireRead(getByte(0),getByte(2)); break;
          case 0x84: sendWireRead(getByte(0),getByte(1)); break;
        
          case 0xFE:  // firmware name
            _println("PC mode: " mVersion "\r\n");
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
          case CMD_CHECKREMOTEKEY: sendRemote(); break;
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
      _loop();
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

static void sendBin(uint8_t* buf, uint8_t num)
{
      memmove(buffer+5, buf, num);
      uint8_t* dp = buffer;
      *dp++ = 0xff;
      *dp++ = 0x55;
      *dp++ = 2+num;
      *dp++ = RSP_BUF;
      *dp++ = num;
      _write(buffer, 5+num);
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
