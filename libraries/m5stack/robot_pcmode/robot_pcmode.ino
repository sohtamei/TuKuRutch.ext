// copyright to SohtaMei 2019.


#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
// #define M5STACK_MPU6050
// #define M5STACK_200Q
#include <M5Stack.h>

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
    
    M5.begin(true, true, true); // init lcd, sd card, serial
    M5.Power.begin();    // use battery
    M5.IMU.Init();
    
    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println(mVersion);
    
    Serial.begin(115200);
    
    Serial.println("PC mode: " mVersion);
}

#define getByte(n)      (buffer[4+n])
#define getShort(n)     (buffer[4+n]|(buffer[5+n]<<8))
#define getLong(n)      (buffer[4+n]|(buffer[5+n]<<8UL)|(buffer[6+n]<<16UL)|(buffer[7+n]<<24UL))
static uint8_t buffer[52];

static void parseData()
{
    switch(buffer[3]){
        case 2: M5.Lcd.setTextColor(getShort(0));M5.Lcd.setTextSize(getByte(2));; callOK(); break;
        case 3: M5.Lcd.setCursor(getShort(0),getShort(2));; callOK(); break;
        case 4: M5.Lcd.print(getString(0));; callOK(); break;
        case 5: M5.Lcd.println(getString(0));; callOK(); break;
        case 6: M5.Lcd.drawString(getString(5),getShort(0),getShort(2),getByte(4));; callOK(); break;
        case 7: M5.Lcd.fillScreen(getShort(0));; callOK(); break;
        case 8: if(getByte(8)) M5.Lcd.fillCircle(getShort(0),getShort(2),getShort(4),getShort(6)); else M5.Lcd.drawCircle(getShort(0),getShort(2),getShort(4),getShort(6));; callOK(); break;
        case 9: if(getByte(10)) M5.Lcd.fillRect(getShort(0),getShort(2),getShort(4),getShort(6),getShort(8)); else M5.Lcd.drawRect(getShort(0),getShort(2),getShort(4),getShort(6),getShort(8));; callOK(); break;
        case 10: if(getByte(14)) M5.Lcd.fillTriangle(getShort(0),getShort(2),getShort(4),getShort(6),getShort(8),getShort(10),getShort(12)); else M5.Lcd.drawTriangle(getShort(0),getShort(2),getShort(4),getShort(6),getShort(8),getShort(10),getShort(12));; callOK(); break;
        case 11: M5.Lcd.qrcode(getString(0));; callOK(); break;
        case 12: M5.Lcd.drawJpgFile(SD,getString(4),getShort(0),getShort(2));; callOK(); break;
        case 13: M5.Lcd.drawBmpFile(SD,getString(4),getShort(0),getShort(2));; callOK(); break;
        case 15: M5.Speaker.tone(getShort(0),getShort(2));delay(getShort(2));; callOK(); break;
        case 16: M5.Speaker.tone(getShort(0),getShort(2));delay(getShort(2));; callOK(); break;
        case 17: M5.Speaker.tone(getShort(0),getShort(2));delay(getShort(2));; callOK(); break;
        case 18: M5.Speaker.beep();; callOK(); break;
        case 19: sendByte((checkButton(getByte(0)))); break;
        case 20: sendFloat((getIMU(getByte(0)))); break;
        case 21: pinMode(getByte(0),OUTPUT);digitalWrite(getByte(0),getByte(1));; callOK(); break;
        case 22: sendByte((pinMode(getByte(0),INPUT),digitalRead(getByte(0)))); break;
        
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
static uint8_t _packetLen = 4;

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
