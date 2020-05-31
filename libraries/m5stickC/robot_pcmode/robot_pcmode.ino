// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "M5StickC 1.0"

#include <M5StickC.h>
#include <HTTPClient.h>
#include "TukurutchEsp.h"

uint8_t checkButton(uint8_t button)
{
      switch(button) {
          case 0: return M5.BtnA.isPressed();
          case 1: return M5.BtnB.isPressed();
        //case 2: return M5.BtnC.isPressed();
      }
      return 0;
}

float getIMU(uint8_t index)
{
      float data[3];
      if(index < 3) {
            M5.IMU.getGyroData(data+0,data+1,data+2);
            return data[index-0];
      } else if(index < 6) {
            M5.IMU.getAccelData(data+0,data+1,data+2);
            return data[index-3];
      } else if(index < 9) {
            M5.IMU.getAhrsData(data+0,data+1,data+2);
            return data[index-6];
      } else {
            M5.IMU.getTempData(data+0);
            return data[0];
      }
}

String getHttp(char* url, uint8_t withErrorMsg)
{
      HTTPClient client;
      client.begin(url);
      int ret = client.GET();
    
      String result;
      if(ret == 200) {
            result = client.getString();
      } else {
            result = withErrorMsg ? client.errorToString(ret): "";
      }
      client.end();
      return result;
    //	stringOne.toCharArray(charBuf, 50)
}

int8_t weather[4];
char wbuf[64];
char* getWeather(char* index)
{
      // 24:11/ 0. 0/5:10 17:00 東京都東京地方
      snprintf(wbuf,sizeof(wbuf),"http://sohta02.web.fc2.com/w/%s.txt",index);
      String str = getHttp(wbuf, false);
      int i;
      memset(wbuf,0,sizeof(wbuf));
      for(i = 0; i < 4; i++) weather[i] = -128; // INVALID
      if(str.length() > 12) {
            char tmp[5];
        const char separator[4] = {':','/','.','/'};
            char* dp2;
            const char* dp1 = str.c_str();
            for(int i = 0; i < 4; i++) {
                  dp2 = strchr(dp1, separator[i]);
                  if(dp2 == NULL) break;
                  if(dp2-dp1<sizeof(tmp)-1 && memcmp(dp1,"--",2)) {
                        memcpy(tmp, dp1, dp2-dp1);
                        tmp[dp2-dp1] = 0;
                        weather[i] = strtol(tmp, NULL, 10);
                  }
                  dp1 = dp2+1;
            }
            memcpy(wbuf, str.c_str(), dp1-str.c_str());
      }
      return wbuf;
}

void connectedCB(String localIP)
{
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println(localIP);
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
    #ifdef PCMODE
      M5.Lcd.println("PC mode: " mVersion);
    #else
      M5.Lcd.println("Arduino mode: " mVersion);
    #endif
    
    Serial.begin(115200);
    initWifi(mVersion, connectedCB);
    
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
  {'S','B',},
  {'S','S',},
  {'s',},
  {'s',},
  {'s','S','S','B',},
  {'S',},
  {'S','S','S','S','B',},
  {'S','S','S','S','S','B',},
  {'S','S','S','S','S','S','S','B',},
  {'s',},
  {},
  {'B',},
  {'B',},
  {},
  {'B','B',},
  {'B',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'s',},
  {'B',},
  {'s',},
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
        case 2: M5.Lcd.setTextColor(getShort(0));M5.Lcd.setTextSize(getByte(1));; callOK(); break;
        case 3: M5.Lcd.setCursor(getShort(0),getShort(1));; callOK(); break;
        case 4: M5.Lcd.print(getString(0));; callOK(); break;
        case 5: M5.Lcd.println(getString(0));; callOK(); break;
        case 6: M5.Lcd.drawString(getString(0),getShort(1),getShort(2),getByte(3));; callOK(); break;
        case 7: M5.Lcd.fillScreen(getShort(0));; callOK(); break;
        case 8: if(getByte(4)) M5.Lcd.fillCircle(getShort(0),getShort(1),getShort(2),getShort(3)); else M5.Lcd.drawCircle(getShort(0),getShort(1),getShort(2),getShort(3));; callOK(); break;
        case 9: if(getByte(5)) M5.Lcd.fillRect(getShort(0),getShort(1),getShort(2),getShort(3),getShort(4)); else M5.Lcd.drawRect(getShort(0),getShort(1),getShort(2),getShort(3),getShort(4));; callOK(); break;
        case 10: if(getByte(7)) M5.Lcd.fillTriangle(getShort(0),getShort(1),getShort(2),getShort(3),getShort(4),getShort(5),getShort(6)); else M5.Lcd.drawTriangle(getShort(0),getShort(1),getShort(2),getShort(3),getShort(4),getShort(5),getShort(6));; callOK(); break;
        case 11: M5.Lcd.qrcode(getString(0));; callOK(); break;
        case 13: sendByte((checkButton(getByte(0)))); break;
        case 14: sendFloat((getIMU(getByte(0)))); break;
        case 15: M5.IMU.Init();; callOK(); break;
        case 16: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 17: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 18: sendString((getWeather(getString(0)))); break;
        case 19: sendString((getWeather(getString(0)))); break;
        case 20: sendString((getWeather(getString(0)))); break;
        case 21: sendString((getWeather(getString(0)))); break;
        case 22: sendString((getWeather(getString(0)))); break;
        case 23: sendString((getWeather(getString(0)))); break;
        case 24: sendString((getWeather(getString(0)))); break;
        case 25: sendString((getWeather(getString(0)))); break;
        case 26: sendString((getWeather(getString(0)))); break;
        case 27: sendString((getWeather(getString(0)))); break;
        case 28: sendString(((weather[getByte(0)]==-128?"":String(weather[getByte(0)])))); break;
        case 29: sendString((getHttp(getString(0),true))); break;
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
    
    M5.update();  // update button and speaker
    delay(50);
    
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
