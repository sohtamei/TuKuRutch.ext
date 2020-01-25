// copyright to SohtaMei 2019.


#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <EEPROM.h>
#include <remoconRoboLib.h>

#define mVersion "RemoconRobo1.0"

uint8_t initMP3 = 0;        // for playMP3
Servo srvClass[3];          // for setServo
const uint8_t srvPin[3] = {3,9,10};


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
    
    remoconRobo_init();
    digitalWrite(13, HIGH);
    Serial.begin(115200);
    delay(500);
    digitalWrite(13, LOW);
    remoconRobo_tone(500, 50);
    
    Serial.println("PC mode: " mVersion);
}

#define getByte(n)      (buffer[4+n])
#define getShort(n)     (buffer[4+n]|(buffer[5+n]<<8))
#define getLong(n)      (buffer[4+n]|(buffer[5+n]<<8UL)|(buffer[6+n]<<16UL)|(buffer[7+n]<<24UL))
static uint8_t buffer[52];

static void parseData()
{
    switch(buffer[3]){
        case 1: pinMode(13,OUTPUT);digitalWrite(13,getByte(0));; callOK(); break;
        case 2: remoconRobo_tone(getShort(0),getShort(2));; callOK(); break;
        case 3: remoconRobo_tone(getShort(0),getShort(2));; callOK(); break;
        case 4: remoconRobo_tone(getShort(0),getShort(2));; callOK(); break;
        case 5: remoconRobo_setRobot(getByte(0),getByte(1));; callOK(); break;
        case 6: remoconRobo_setRobot(0,0);; callOK(); break;
        case 7: remoconRobo_setMotor(getByte(0)-1,getShort(1));; callOK(); break;
        case 8: remoconRobo_incCalib(getShort(0));; callOK(); break;
        case 9: remoconRobo_incCalib(-getShort(0));; callOK(); break;
        case 10: sendShort((remoconRobo_getCalib())); break;
        case 11: remoconRobo_setCalib(getShort(0));; callOK(); break;
        case 19: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 20: pinMode(A0+getByte(0),OUTPUT);digitalWrite(A0+getByte(0),getByte(1));; callOK(); break;
        case 21: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 22: sendByte((pinMode(A0+getByte(0),INPUT),digitalRead(A0+getByte(0)))); break;
        case 23: sendShort((pinMode(A0+getByte(0),INPUT),remoconRobo_getAnalog(A0+getByte(0),1))); break;
        case 24: sendShort((pinMode(A0+getByte(0),INPUT),remoconRobo_getAnalog(A0+getByte(0),getShort(1)))); break;
        case 26: if(!srvClass[getByte(0)].attached()) srvClass[getByte(0)].attach(srvPin[getByte(0)]); srvClass[getByte(0)].write(getByte(1));; callOK(); break;
        case 27: if(!initMP3) remoconRobo_initMP3(30); initMP3=1; remoconRobo_playMP3(getByte(0),getByte(1));; callOK(); break;
        case 28: remoconRobo_stopMP3();; callOK(); break;
        
        //### CUSTOMIZED ###
        #ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
        #define CMD_CHECKREMOTEKEY  0x80
        case CMD_CHECKREMOTEKEY:
        sendRemote();
        break;
        #endif
    }
}

static uint8_t index = 0;
static uint8_t _packetLen = 4;

void loop()
{
    if(Serial.available()>0){
        uint8_t c = Serial.read();
        buffer[index++] = c;
        
        switch(index) {
            case 1:
            _packetLen = 4;
            if(c != 0xff)
            index = 0;
            break;
            case 2:
            if(c != 0x55) 
            index = 0;
            break;
            case 3:
            _packetLen = 3+c;
            break;
        }
        if(index >= _packetLen) {
            parseData();
            index = 0;
        }
        if(index >= sizeof(buffer)) {
            index = 0;
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

float getFloat(uint8_t n)
{
    union floatConv conv;
    for(uint8_t i=0; i<4; i++) {
        conv._byte[i] = buffer[4+n+i];
    }
    return conv._float;
}

double getDouble(uint8_t n)
{
    union doubleConv conv;
    for(uint8_t i=0; i<8; i++) {
        conv._byte[i] = buffer[4+n+i];
    }
    return conv._double;
}

char* getString(uint8_t n)
{
    return (char*)buffer+4+n;
}

static void callOK()
{
    Serial.write(0xff);
    Serial.write(0x55);
    Serial.write(0);
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
