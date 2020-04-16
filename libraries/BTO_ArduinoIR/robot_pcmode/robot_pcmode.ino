// copyright to SohtaMei 2019.


#define mVersion "BTO_ArduinoIR1.0"

#include <Wire.h>

#define SLAVE_ADDRESS	0x52
//command
enum {
    R1_memo_no_write	= 0x15,		// ReadCH設定	(CH)
    R2_data_num_read	= 0x25,		// ReadNum設定	(0,LenH, LenL)
    R3_data_read		= 0x35,		// ReadData		(0,data[len])
    
    W1_memo_no_write	= 0x19,		// WriteCH設定	(CH)
    W2_data_num_write	= 0x29,		// WriteNum設定	(LenH, LenL)
    W3_data_write		= 0x39,		// WriteData	(data[len])
    W4_flash_write		= 0x49,		// WriteFlash
    
    T1_trans_start		= 0x59, 	// Send
};

uint8_t workBuf[256];
void sendRemote(uint8_t* buf, uint16_t totalLen)
{
      uint16_t i;
    
      memset(workBuf, 0, sizeof(workBuf));
      memcpy(workBuf, buf, totalLen);
      totalLen = (totalLen+2) & ~3;
    
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(W2_data_num_write);
      Wire.write(highByte(totalLen/4));
      Wire.write(lowByte(totalLen/4));
      Wire.endTransmission(true);
    
      for(i = 0; i < totalLen; i += 16) {
            uint8_t len = totalLen - i;
            if(len > 16) len = 16;
            Wire.beginTransmission(SLAVE_ADDRESS);
            Wire.write(W3_data_write);
            Wire.write(buf+i, len);
            Wire.endTransmission(true);
            delay(10);
      }
    
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(T1_trans_start);
      Wire.endTransmission(true);
}

void setButton(uint8_t index)
{
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(W1_memo_no_write);
      Wire.write(index);
      Wire.endTransmission(true);
    
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(W4_flash_write);
      Wire.endTransmission(true);
}

char* getRemote(uint8_t index)
{
      uint16_t i;
      uint16_t totalLen = 0;
    
      memset(workBuf, 0, sizeof(workBuf));
    
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(R1_memo_no_write);
      Wire.write(index);
      Wire.endTransmission(true);
    
      Wire.beginTransmission(SLAVE_ADDRESS);
      Wire.write(R2_data_num_read);
      Wire.endTransmission(false);
      Wire.requestFrom(SLAVE_ADDRESS, 2+1);
      if(Wire.available() < 2+1) return (char*)workBuf;
      Wire.read();	// dummy00
      totalLen = Wire.read();
      totalLen = (totalLen<<8) + Wire.read();
      totalLen = totalLen*4 - 2/*last 'OFF'*/ + 1/*dummy00*/;
      if(totalLen > 125) totalLen = 125;	// UART送信最大/2
    
      for(i = 0; i < totalLen; i += 16) {
            uint8_t len = totalLen - i;
            if(len > 16) len = 16;
        
            Wire.beginTransmission(SLAVE_ADDRESS);
            Wire.write(R3_data_read);
            Wire.endTransmission(false);
            Wire.requestFrom(SLAVE_ADDRESS, (int)len);
            if(Wire.available() < len) return (char*)workBuf;
        
            uint8_t j;
            for(j = 0; j < len; j++)
              snprintf((char*)workBuf+(i+j)*2, 3, "%02x", Wire.read());
            delay(10);
      }
    
      return (char*)workBuf+2/*dummy00*/;
}

void sendButton(uint8_t index)
{
      digitalWrite(index+2, HIGH);
      pinMode(index+2, OUTPUT);
      digitalWrite(index+2, LOW);
      delay(10);
      digitalWrite(index+2, HIGH);
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
    
    Wire.begin();
    
    
    _Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[256];  // 0xFF,0x55,len,cmd,
static uint8_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {'B',},
  {},
  {'B',},
  {'B',},
  {'b',},
  {'b',},
  {'b',},
  {'b',},
  {'b',},
  {},
  {'b',},
  {'b',},
  {'b',},
  {'b',},
  {'b',},
  {},
  {'b',},
  {'b',},
  {'b',},
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
                case 'b': offset += buffer[4+offset]+1; break;
                default: break;
            }
            if(4+offset > _packetLen) return;
        }
    }
    
    switch(buffer[3]){
        case 1: sendButton(getByte(0)-1);; callOK(); break;
        case 3: setButton(getByte(0)-1);; callOK(); break;
        case 4: sendString((getRemote(getByte(0)-1))); break;
        case 5: sendRemote(getBufLen(0));; callOK(); break;
        case 6: sendRemote(getBufLen(0));; callOK(); break;
        case 7: sendRemote(getBufLen(0));; callOK(); break;
        case 8: sendRemote(getBufLen(0));; callOK(); break;
        case 9: sendRemote(getBufLen(0));; callOK(); break;
        case 11: sendRemote(getBufLen(0));; callOK(); break;
        case 12: sendRemote(getBufLen(0));; callOK(); break;
        case 13: sendRemote(getBufLen(0));; callOK(); break;
        case 14: sendRemote(getBufLen(0));; callOK(); break;
        case 15: sendRemote(getBufLen(0));; callOK(); break;
        case 17: sendRemote(getBufLen(0));; callOK(); break;
        case 18: sendRemote(getBufLen(0));; callOK(); break;
        case 19: sendRemote(getBufLen(0));; callOK(); break;
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
