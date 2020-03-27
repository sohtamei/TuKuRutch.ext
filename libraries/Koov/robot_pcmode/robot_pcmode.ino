// copyright to SohtaMei 2019.


#define mVersion "Koov1.0"

#include <Arduino.h>
#include <Servo.h>
#include <analogRemote.h>

#define STATUS_AMB  0
#define STATUS_BLUE 1
//efine V2          2
//efine V3          3
#define V0_PWM      4
#define V0_DIR      5
//efine V4          6
//efine V5          7
//efine V6          8
//efine V7          9
#define V1_DIR      10
//efine V8          11
#define V1_PWM      12
#define V9          13

#define MULTI_BLUE  10
#define MULTI_GREEN 12
#define MULTI_RED   13

#define V1V9_MULTI  18

#define USB_YELLOW  20
#define USB_BLUE    21

#define K2_UP       24
#define K3_RIGHT    25
#define K4_DOWN     26
#define K5_LEFT     27
#define K6          28
#define K7          29

#define REMOTE_ENABLE	// for robot_pcmode.ino.template
static void funcLed(uint8_t onoff) { digitalWrite(STATUS_AMB, onoff^1); }
static analogRemote remote(MODE_NORMAL, K2_UP, funcLed);

static void _setStatusLED(uint8_t color)
{
    color ^= 5;
    digitalWrite(STATUS_BLUE, (color>>0)&1);
    digitalWrite(STATUS_AMB, (color>>2)&1);
}

static void _setUsbLED(uint8_t color)
{
    color ^= 3;
    digitalWrite(USB_BLUE, (color>>0)&1);
    digitalWrite(USB_YELLOW, (color>>1)&1);
}

static uint8_t _V1V9_MULTI = 0xFF;
static void _setV1V9_MULTI(uint8_t onoff)
{
    if(_V1V9_MULTI == onoff) return;
    _V1V9_MULTI = onoff;
    digitalWrite(V1_PWM, LOW);
    digitalWrite(V1_DIR, LOW);
    digitalWrite(V9, LOW);
    digitalWrite(V1V9_MULTI, onoff);
}

static void _setMultiLED(uint8_t color)
{
    if(color) {
          _setV1V9_MULTI(LOW);
          color ^= 7;
          digitalWrite(MULTI_BLUE, (color>>0)&1);
          digitalWrite(MULTI_GREEN, (color>>1)&1);
          digitalWrite(MULTI_RED, (color>>2)&1);
    } else {
          _setV1V9_MULTI(HIGH);
    }
}

static void _setMotor(uint8_t ch, int16_t data)
{
    if(data >  255) data =  255;
    if(data < -255) data = -255;
uint8_t pwm[2] = {V0_PWM, V1_PWM};
uint8_t dir[2] = {V0_DIR, V1_DIR};
    if(ch==1) _setV1V9_MULTI(HIGH);
    
    if(data>=0){
          analogWrite(pwm[ch], data);
          digitalWrite(dir[ch], LOW);
    } else {
          analogWrite(pwm[ch], data+255);
          digitalWrite(dir[ch], HIGH);
    }
}

struct {
      int8_t  L;
      int8_t  R;
} static const dir_table[6] = {
      //L   R
  { 1,  1}, // DIR_FORWARD
  { 0,  1}, // DIR_LEFT,
  { 1,  0}, // DIR_RIGHT,
  {-1, -1}, // DIR_BACK,
  {-1,  1}, // DIR_ROLL_LEFT,
  { 1, -1}, // DIR_ROLL_RIGHT,
};

static void _setRobot(uint8_t direction, int16_t speed)
{
    _setMotor(1, speed * dir_table[direction].L);
    _setMotor(0, speed * dir_table[direction].R);
}

static Servo srvClass[14];          // for setServo

static void _tone(uint8_t port, uint16_t freq, uint16_t duration)
{
    if(port == V9) _setV1V9_MULTI(HIGH);
    if(srvClass[port].attached()) srvClass[port].detach();
    tone(port,freq,duration); delay(duration);
}

static void _servo(uint8_t port, uint8_t deg)
{
    if(port >= sizeof(srvClass)/sizeof(srvClass[0])) return;
    
    if(!srvClass[port].attached()) {
          pinMode(port, OUTPUT);
          srvClass[port].attach(port);
    }
    if(port == V9) _setV1V9_MULTI(HIGH);
    srvClass[port].write(deg);
}

static void _stopServo(void)
{
    uint8_t port;
    for(port = 0; port < sizeof(srvClass)/sizeof(srvClass[0]); port++) {
        if(srvClass[port].attached())
          srvClass[port].detach();
    }
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
    
    pinMode(STATUS_AMB, OUTPUT);
    pinMode(STATUS_BLUE, OUTPUT);
    pinMode(V0_PWM, OUTPUT);
    pinMode(V0_DIR, OUTPUT);
    pinMode(MULTI_BLUE, OUTPUT);
    pinMode(MULTI_GREEN, OUTPUT);
    pinMode(MULTI_RED, OUTPUT);
    pinMode(USB_YELLOW, OUTPUT);
    pinMode(USB_BLUE, OUTPUT);
    pinMode(V1V9_MULTI, OUTPUT);
    _setV1V9_MULTI(HIGH);
    Serial.begin(115200);
    
    SerialUSB.println("PC mode: " mVersion);
}

static uint8_t buffer[52];  // 0xFF,0x55,len,cmd,
static uint8_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {'B','B',},
  {'B','S',},
  {},
  {},
  {'B','B',},
  {'B','S','S',},
  {'B','S','S',},
  {'B','S','S',},
  {'B','B',},
  {},
  {'B',},
  {'B',},
  {'B',},
  {},
  {'B',},
  {'B',},
  {'B',},
  {'B','B',},
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
        case 1: _setRobot(getByte(0),getByte(1));; callOK(); break;
        case 2: _setMotor(getByte(0),getShort(1));; callOK(); break;
        case 3: _setRobot(0,0);_stopServo();; callOK(); break;
        case 5: _servo(getByte(0),getByte(1));; callOK(); break;
        case 6: _tone(getByte(0),getShort(1),getShort(2));; callOK(); break;
        case 7: _tone(getByte(0),getShort(1),getShort(2));; callOK(); break;
        case 8: _tone(getByte(0),getShort(1),getShort(2));; callOK(); break;
        case 9: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 11: _setMultiLED(getByte(0));; callOK(); break;
        case 12: _setUsbLED(getByte(0));; callOK(); break;
        case 13: _setStatusLED(getByte(0));; callOK(); break;
        case 15: sendByte((pinMode(getByte(0),INPUT_PULLUP),digitalRead(getByte(0))^1)); break;
        case 16: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        case 17: sendShort((pinMode(getByte(0),INPUT),analogRead(getByte(0)))); break;
        case 18: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 19: pinMode(getByte(0),OUTPUT);analogWrite(getByte(0),getByte(1));; callOK(); break;
        
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
    if(SerialUSB.available()>0){
        uint8_t c = SerialUSB.read();
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
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write((uint8_t)0);
}

static void sendByte(uint8_t data)
{
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+sizeof(uint8_t));
    SerialUSB.write(RSP_BYTE);
    SerialUSB.write(data);
}

static void sendShort(uint16_t data)
{
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+sizeof(uint16_t));
    SerialUSB.write(RSP_SHORT);
    SerialUSB.write(data&0xff);
    SerialUSB.write(data>>8);
}

static void sendLong(uint32_t data)
{
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+sizeof(uint32_t));
    SerialUSB.write(RSP_LONG);
    SerialUSB.write(data&0xff);
    SerialUSB.write(data>>8);
    SerialUSB.write(data>>16);
    SerialUSB.write(data>>24);
}

static void sendFloat(float data)
{
    union floatConv conv;
    conv._float = data;
    
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+sizeof(float));
    SerialUSB.write(RSP_FLOAT);
    SerialUSB.write(conv._byte[0]);
    SerialUSB.write(conv._byte[1]);
    SerialUSB.write(conv._byte[2]);
    SerialUSB.write(conv._byte[3]);
}

static void sendDouble(double data)
{
    union doubleConv conv;
    conv._double = data;
    
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+sizeof(double));
    SerialUSB.write(RSP_DOUBLE);
    for(uint8_t i=0; i<8; i++) {
        SerialUSB.write(conv._byte[i]);
    }
}

static void sendString(String s)
{
    uint8_t l = s.length();
    
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+l);
    SerialUSB.write(RSP_STRING);
    for(uint8_t i=0; i<l; i++) {
        SerialUSB.write(s.charAt(i));
    }
}

//### CUSTOMIZED ###
#ifdef REMOTE_ENABLE	// check remoconRoboLib.h or quadCrawlerRemocon.h
static void sendRemote(void)
{
    uint16_t data;
    SerialUSB.write(0xff);
    SerialUSB.write(0x55);
    SerialUSB.write(1+1+2+2);
    SerialUSB.write(CMD_CHECKREMOTEKEY);
    SerialUSB.write(remote.checkRemoteKey());
    data = remote.x;
    SerialUSB.write(data&0xff);
    SerialUSB.write(data>>8);
    data = remote.y;
    SerialUSB.write(data&0xff);
    SerialUSB.write(data>>8);
}
#endif
