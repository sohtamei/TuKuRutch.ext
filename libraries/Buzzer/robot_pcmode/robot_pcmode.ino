// copyright to SohtaMei 2019.


#define mVersion "Buzzer1.0"

#include <Arduino.h>

void _tone(int sound, int ms) { tone(12, sound, ms); delay(ms); }

uint16_t _getAnalog(uint8_t ch, uint16_t count)
{
      if(count == 0) count = 1;
      uint32_t sum = 0;
      uint16_t i;
      for(i = 0; i < count; i++)
        sum += analogRead(ch);
      sum = ((sum / count) * 625UL) / 128;  // 1024->5000
      return sum;
}


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
    
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    pinMode(12, OUTPUT);
    _tone(500, 50);
    Serial.begin(115200);
    
    Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[52];  // 0xFF,0x55,len,cmd,
static uint8_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {'B',},
  {'S','S',},
  {'S','S',},
  {'S','S',},
  {},
  {'B','B',},
  {'B','B',},
  {'B',},
  {'B',},
  {'B',},
  {'B','S',},
};

static void parseData()
{
    uint8_t i;
    if(buffer[3] >= ITEM_NUM) return;
    
    memset(offsetIdx, 0, sizeof(offsetIdx));
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
    
    switch(buffer[3]){
        case 1: pinMode(13,OUTPUT);digitalWrite(13,getByte(0));; callOK(); break;
        case 2: _tone(getShort(0),getShort(1));; callOK(); break;
        case 3: _tone(getShort(0),getShort(1));; callOK(); break;
        case 4: _tone(getShort(0),getShort(1));; callOK(); break;
        case 6: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 7: pinMode(A0+getByte(0),OUTPUT);digitalWrite(A0+getByte(0),getByte(1));; callOK(); break;
        case 8: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 9: sendByte((pinMode(A0+getByte(0),INPUT),digitalRead(A0+getByte(0)))); break;
        case 10: sendShort((pinMode(A0+getByte(0),INPUT),_getAnalog(A0+getByte(0),1))); break;
        case 11: sendShort((pinMode(A0+getByte(0),INPUT),_getAnalog(A0+getByte(0),getShort(1)))); break;
        
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
    if(Serial.available()>0){
        uint8_t c = Serial.read();
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
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write((uint8_t)0);
}

static void sendByte(uint8_t data)
{
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+sizeof(uint8_t));
    Serial.write(RSP_BYTE);
    Serial.write(data);
}

static void sendShort(uint16_t data)
{
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+sizeof(uint16_t));
    Serial.write(RSP_SHORT);
    Serial.write(data&0xff);
    Serial.write(data>>8);
}

static void sendLong(uint32_t data)
{
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+sizeof(uint32_t));
    Serial.write(RSP_LONG);
    Serial.write(data&0xff);
    Serial.write(data>>8);
    Serial.write(data>>16);
    Serial.write(data>>24);
}

static void sendFloat(float data)
{
    union floatConv conv;
    conv._float = data;
    
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+sizeof(float));
    Serial.write(RSP_FLOAT);
    Serial.write(conv._byte[0]);
    Serial.write(conv._byte[1]);
    Serial.write(conv._byte[2]);
    Serial.write(conv._byte[3]);
}

static void sendDouble(double data)
{
    union doubleConv conv;
    conv._double = data;
    
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+sizeof(double));
    Serial.write(RSP_DOUBLE);
    for(uint8_t i=0; i<8; i++) {
        Serial.write(conv._byte[i]);
    }
}

static void sendString(String s)
{
    uint8_t l = s.length();
    
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+l);
    Serial.write(RSP_STRING);
    for(uint8_t i=0; i<l; i++) {
        Serial.write(s.charAt(i));
    }
}

//### CUSTOMIZED ###
#ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
static void sendRemote(void)
{
    uint16_t data;
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(1+1+2+2);
    Serial.write(CMD_CHECKREMOTEKEY);
    Serial.write(remoconRobo_checkRemoteKey());
    data = remoconRobo_getRemoteX();
    Serial.write(data&0xff);
    Serial.write(data>>8);
    data = remoconRobo_getRemoteY();
    Serial.write(data&0xff);
    Serial.write(data>>8);
}
#endif
