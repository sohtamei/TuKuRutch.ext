// copyright to SohtaMei 2019.


#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
// #define M5STACK_MPU6050
// #define M5STACK_200Q
#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>

#define mVersion "M5STACK 1.0"

uint8_t checkButton(uint8_t button)
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
/*
enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
};
*/


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

void setup()
{
    #ifdef __AVR_ATmega328P__
    MCUSR = 0;
    wdt_disable();
    #endif
    
    M5.begin(true, true, true); // init lcd, sd card, serial
    M5.Power.begin();    // use battery
    M5.IMU.Init();
    
    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("PC mode: " mVersion);
    
    Serial.begin(115200);
    
    _Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[52];  // 0xFF,0x55,len,cmd,
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
  {'S','S','s',},
  {'S','S','s',},
  {},
  {'S','S',},
  {'S','S',},
  {'S','S',},
  {},
  {'s','s',},
  {},
  {},
  {'B',},
  {'B',},
  {'B','B',},
  {'B',},
  {},
};

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
        case 12: M5.Lcd.drawJpgFile(SD,getString(2),getShort(0),getShort(1));; callOK(); break;
        case 13: M5.Lcd.drawBmpFile(SD,getString(2),getShort(0),getShort(1));; callOK(); break;
        case 15: M5.Speaker.tone(getShort(0),getShort(1));delay(getShort(1));; callOK(); break;
        case 16: M5.Speaker.tone(getShort(0),getShort(1));delay(getShort(1));; callOK(); break;
        case 17: M5.Speaker.tone(getShort(0),getShort(1));delay(getShort(1));; callOK(); break;
        case 18: M5.Speaker.beep();; callOK(); break;
        case 19: WiFi.begin(getString(0),getString(1));; callOK(); break;
        case 20: sendByte((WiFi.status())); break;
        case 21: sendByte((WiFi.localIP()[3])); break;
        case 22: sendByte((checkButton(getByte(0)))); break;
        case 23: sendFloat((getIMU(getByte(0)))); break;
        case 24: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 25: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 0xFE:  // firmware name
        _Serial.println("PC mode: " mVersion);
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
    if(_Serial.available()>0){
        uint8_t c = _Serial.read();
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
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write((uint8_t)0);
}

static void sendByte(uint8_t data)
{
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+sizeof(uint8_t));
    _Serial.write(RSP_BYTE);
    _Serial.write(data);
}

static void sendShort(uint16_t data)
{
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+sizeof(uint16_t));
    _Serial.write(RSP_SHORT);
    _Serial.write(data&0xff);
    _Serial.write(data>>8);
}

static void sendLong(uint32_t data)
{
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+sizeof(uint32_t));
    _Serial.write(RSP_LONG);
    _Serial.write(data&0xff);
    _Serial.write(data>>8);
    _Serial.write(data>>16);
    _Serial.write(data>>24);
}

static void sendFloat(float data)
{
    union floatConv conv;
    conv._float = data;
    
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+sizeof(float));
    _Serial.write(RSP_FLOAT);
    _Serial.write(conv._byte[0]);
    _Serial.write(conv._byte[1]);
    _Serial.write(conv._byte[2]);
    _Serial.write(conv._byte[3]);
}

static void sendDouble(double data)
{
    union doubleConv conv;
    conv._double = data;
    
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+sizeof(double));
    _Serial.write(RSP_DOUBLE);
    for(uint8_t i=0; i<8; i++) {
        _Serial.write(conv._byte[i]);
    }
}

static void sendString(String s)
{
    uint8_t l = s.length();
    
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+l);
    _Serial.write(RSP_STRING);
    for(uint8_t i=0; i<l; i++) {
        _Serial.write(s.charAt(i));
    }
}

//### CUSTOMIZED ###
#ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
static void sendRemote(void)
{
    uint16_t data;
    _Serial.write(0xff);
    _Serial.write(0x55);
    _Serial.write(1+1+2+2);
    _Serial.write(CMD_CHECKREMOTEKEY);
    _Serial.write(remote.checkRemoteKey());
    data = remote.x;
    _Serial.write(data&0xff);
    _Serial.write(data>>8);
    data = remote.y;
    _Serial.write(data&0xff);
    _Serial.write(data>>8);
}
#endif
