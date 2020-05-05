// copyright to SohtaMei 2019.


#define mVersion "duke32 1.0"
//------------------------------------------
// Moter Driver
//------------------------------------------
#define TB_A1   12  // moterA IN1
#define TB_A2   13  // moterA IN2
#define TB_Ap   18  // moterA PWM
#define TB_B1   14  // moterB IN1
#define TB_B2   15  // moterB IN2
#define TB_Bp   19  // moterB PWM

#define NEO_PIN     27
#define Servo_PIN1  23
#define Servo_PIN2   4

#define ANA11   34
#define ANA12   35

#define DIGI01  32
#define DIGI02  33
#define DIGI11  26
#define DIGI12  25

#define I2CSDA  21
#define I2CSCL  22

// UART2
#define UARTRXD 16
#define UARTTXD 17

#include <WiFi.h>
#include <AsyncUDP.h>
#include <Preferences.h>

#include <Wire.h>

#define PORT  54321

char g_ssid[32] = {0};
char g_password[32] = {0};
WiFiServer server(PORT);
WiFiClient client;
AsyncUDP udp;
Preferences preferences;

// https://github.com/Makuna/NeoPixelBus
#include <NeoPixelBus.h>
#define colorSaturation 32
#define interval        50
const uint16_t PixelCount = 45; // this example assumes 4 pixels, making it smaller will cause a failure

// three element pixels, in different order and speeds
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, NEO_PIN);
//NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> strip(PixelCount, PixelPin);

#define LEDC_CHANNEL_0  0  // channel of 16 channels (started from zero)
#define LEDC_CHANNEL_1  1  // channel of 16 channels (started from zero)
#define LEDC_TIMER_BIT 12  // use 12 bit precission for LEDC timer
#define LEDC_BASE_FREQ 50  // use 50 Hz as a LEDC base frequency

// Servo Range
// int srvMin = 103; // (103/4096)*20ms ≒ 0.5 ms (-90°)
// int srvMax = 491; // (491/4096)*20ms ≒ 2.4 ms (+90°)
#define srvMin 300
#define srvMax 480
#define srvInterval 3

/*
 * NeoPixels
 */
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

void SetNeoPixel(uint8_t index){
      RgbColor* npcolor = NULL;
      switch(index) {
          case 0: npcolor = &black; break;
          case 1: npcolor = &red;   break;
          case 2: npcolor = &green; break;
          case 3: npcolor = &blue;  break;
          case 4: npcolor = &white; break;
          default: return;
      }
    
      for(uint8_t i=0; i<PixelCount; i++){
            strip.SetPixelColor(i, *npcolor);
            strip.Show();
            delay(interval);
      }
}

/*
 * Servo
 */
void Servo_INIT(){
      // Setup timer and attach timer to a servo pin
      ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
      ledcAttachPin(Servo_PIN1, LEDC_CHANNEL_0);
    
      ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
      ledcAttachPin(Servo_PIN2, LEDC_CHANNEL_1);
    
      ledcWrite(0, srvMin);
      delay(500);
      ledcWrite(1, srvMin);
}

void setServo(uint8_t index, uint16_t angle)
{
      if(index >= 2) return;
      ledcWrite(index, angle);
}

/*
 * Motor
 */
void Motor_INIT(){
      pinMode( TB_A1, OUTPUT );
      pinMode( TB_A2, OUTPUT );
      pinMode( TB_Ap, OUTPUT );
      pinMode( TB_B1, OUTPUT );
      pinMode( TB_B2, OUTPUT );
      pinMode( TB_Bp, OUTPUT );
       
      digitalWrite( TB_A1, LOW );
      digitalWrite( TB_A2, LOW );
      digitalWrite( TB_Ap, LOW );
      digitalWrite( TB_B1, LOW );
      digitalWrite( TB_B2, LOW );
      digitalWrite( TB_Bp, LOW );
}

void motorA_forward(){
      digitalWrite( TB_Ap, HIGH );
      digitalWrite( TB_A1, HIGH );
      digitalWrite( TB_A2, LOW );
}
void motorA_back(){
      digitalWrite( TB_Ap, HIGH );
      digitalWrite( TB_A1, LOW );
      digitalWrite( TB_A2, HIGH );
}
void motorA_stop(){
      digitalWrite( TB_Ap, LOW );
      digitalWrite( TB_A1, LOW );
      digitalWrite( TB_A2, LOW );
}

void motorB_forward(){
      digitalWrite( TB_Bp, HIGH );
      digitalWrite( TB_B1, HIGH );
      digitalWrite( TB_B2, LOW );
}
void motorB_back(){
      digitalWrite( TB_Bp, HIGH );
      digitalWrite( TB_B1, LOW );
      digitalWrite( TB_B2, HIGH );
}
void motorB_stop(){
      digitalWrite( TB_Bp, LOW );
      digitalWrite( TB_B1, LOW );
      digitalWrite( TB_B2, LOW );
}

void setMotor(uint8_t index, uint8_t dir)
{
      if(index == 0) {
            switch(dir) {
                case 0: motorA_stop();    break;
                case 1: motorA_forward(); break;
                case 2: motorA_back();    break;
            }
      } else if(index == 1) {
            switch(dir) {
                case 0: motorB_stop();    break;
                case 1: motorB_forward(); break;
                case 2: motorB_back();    break;
            }
      }
}

struct {
      int  L;
      int  R;
} static const dir_table[7] = {
     //L  R
  {0, 0},  // STOP
  {2, 1},  // FORWARD
  {0, 1},  // LEFT
  {2, 0},  // RIGHT
  {1, 2},  // BACK
  {1, 1},  // ROLL_LEFT
  {2, 2},  // ROLL_RIGHT
};

void setRobot(int direction)
{
      setMotor(0, dir_table[direction].L);
      setMotor(1, dir_table[direction].R);
}

/*
 * ADC
 */
void ADC_INIT(){
      pinMode(ANA11, INPUT);
      pinMode(ANA12, INPUT);
}

uint16_t getAdc(uint8_t index)
{
uint8_t adcCh[2] = {ANA11,ANA12};
    if(index>=2) return 0;
    return analogRead(adcCh[index]);
}

/*
 * DIO in
 */
void DIO_INIT(){
      pinMode(DIGI01, INPUT);
      pinMode(DIGI02, INPUT);
      pinMode(DIGI11, INPUT);
      pinMode(DIGI12, INPUT);
}

uint8_t getDigital(uint8_t index)
{
uint8_t din_ch[4] = {DIGI01,DIGI02,DIGI11,DIGI12};
    if(index>=4) return 0;
    return digitalRead(din_ch[index]);
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
    
    preferences.begin("tukurutch", false);
    
    // reset all the neopixels to an off state
    strip.Begin();
    strip.Show();
    
    // init Servo
    Servo_INIT();
    
    // init Motor
    Motor_INIT();
    
    // init ADC
    ADC_INIT();
    
    // init DIO
    DIO_INIT();
    
    Serial2.begin(115200);
    
    Serial.begin(115200);
    
    preferences.getString("ssid", g_ssid, sizeof(g_ssid));
    preferences.getString("password", g_password, sizeof(g_password));
    Serial.print("Connecting to ");    Serial.println(g_ssid);
    
    //WiFi.mode(WIFI_STA);
    //WiFi.disconnect();
    WiFi.begin(g_ssid, g_password);
    
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
  {'s','s',},
  {},
  {},
  {},
  {},
  {},
  {'B',},
  {'B',},
  {},
  {'B','B',},
  {'B',},
  {'B',},
};

#if 0		// wifi debug
#define DPRINT(a) _Serial.println(a)
#define DWRITE(a) _Serial.write(a)
#else
#define DPRINT(a)
#define DWRITE(a)
#endif

uint8_t wifi_uart = 0;

void _write(uint8_t* dp, int count)
{
    #if defined(ESP32)
      if(wifi_uart)
        client.write(dp, count);
      else
    #endif
        _Serial.write(dp, count);
}

void _println(char* mes)
{
    #if defined(ESP32)
      if(wifi_uart)
        client.println(mes);
      else
    #endif
        _Serial.println(mes);
}

#if defined(ESP32)
enum {
      CONNECTION_NONE = 0,
      CONNECTION_WIFI,
      CONNECTION_TCP,
};
uint8_t connection_status = CONNECTION_NONE;
uint32_t last_udp;
#endif

int16_t _read(void)
{
      if(_Serial.available()>0) {
            wifi_uart = 0;
            return _Serial.read();
      }
    #if defined(ESP32)
      if(WiFi.status() != WL_CONNECTED) {
            connection_status = CONNECTION_NONE;
            return -1;
      }
    
      uint32_t cur = millis();
      if(cur - last_udp > 1000) {
            last_udp = cur;
            udp.broadcastTo(mVersion, PORT);
      }
    
      switch(connection_status) {
          case CONNECTION_NONE:
            server.begin();
        #ifdef _M5STACK_H_
            M5.Lcd.clear(BLACK);
            M5.Lcd.setCursor(0,0);
            M5.Lcd.println(WiFi.localIP());
        #endif
            DPRINT(WiFi.localIP());
            connection_status = CONNECTION_WIFI;
        
          case CONNECTION_WIFI:
            client = server.available();
            if(!client) {
                  return -1;
            }
            DPRINT("connected");
            connection_status = CONNECTION_TCP;
        
          case CONNECTION_TCP:
            if(!client.connected()) {
                  DPRINT("disconnected");
                  client.stop();
                  connection_status = CONNECTION_WIFI;
                  return -1;
            }
            if(client.available()<=0) {
                  return -1;
            }
            wifi_uart = 1;
            return client.read();
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
        case 2: preferences.putString("ssid",getString(0));preferences.putString("password",getString(1));WiFi.begin(getString(0),getString(1));; callOK(); break;
        case 3: sendByte((WiFi.status())); break;
        case 4: sendByte((WiFi.localIP()[3])); break;
        case 5: sendString((preferences.getString("ssid", g_ssid, sizeof(g_ssid)),g_ssid)); break;
        case 6: sendString((preferences.getString("password", g_password, sizeof(g_password)),g_password)); break;
        case 8: SetNeoPixel(getByte(0));; callOK(); break;
        case 9: setRobot(getByte(0));; callOK(); break;
        case 10: setRobot(0);; callOK(); break;
        case 11: setMotor(getByte(0),getByte(1));; callOK(); break;
        case 12: sendByte((getDigital(getByte(0)))); break;
        case 13: sendShort((getAdc(getByte(0)))); break;
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
        DWRITE(c);
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
