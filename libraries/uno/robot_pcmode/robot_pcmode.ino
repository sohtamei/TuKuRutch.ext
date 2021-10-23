// copyright to SohtaMei 2019.

#define PCMODE

#define mVersion "uno1.0"		// マイコン拡張名＋version
#include "main.h"

#include <Wire.h>

#if defined(__AVR_ATmega328P__)
  #include <avr/wdt.h>

#elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
  #include <BLEPeripheral.h>
  static BLEPeripheral blePeripheral = BLEPeripheral();
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

#define LEDC_BUZZER  15
#define LEDC_PWM_START 2
#define LEDC_PWM_END   14

void setup()
{
    #if defined(__AVR_ATmega328P__)
      MCUSR = 0;
      wdt_disable();
    #elif defined(ESP32)
      ledcSetup(LEDC_BUZZER, 5000/*Hz*/, 13/*bit*/);
    #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
      Serial.begin(19200);
      _bleSetup();
    #endif
    _setup(mVersion);
      _Serial.println("PC mode: " mVersion);
}

static uint8_t buffer[256];  // 0xFF,0x55,len,cmd,
static uint8_t* _packet_dp = buffer;
static uint16_t _packetLen = 4;

#define ARG_NUM  16
#define ITEM_NUM (sizeof(ArgTypesTbl)/sizeof(ArgTypesTbl[0]))
static uint8_t offsetIdx[ARG_NUM] = {0};
static const char ArgTypesTbl[][ARG_NUM] = {
  {},
  {'B','B',},
  {'S','S',},
  {'B','S',},
  {'B',},
};

enum {
        MODE_UART = 0,
        MODE_WS,
        MODE_BLE,
};
uint8_t comMode = MODE_UART;

void _write(uint8_t* dp, int count)
{
    #if defined(ESP32)
      if(comMode == MODE_WS) {
            _packet_dp = dp;
            _packetLen = count;
      } else
    #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
      if(comMode == MODE_BLE) {
            _packet_dp = dp;
            _packetLen = count;
      } else
    #endif
        _Serial.write(dp, count);
}

void _println(const char* mes)
{
      _write((uint8_t*)mes, strlen(mes));
}

// sync with scratch-vm/src/extensions/scratch3_tukurutch/comlib.js
static const char ArgTypesTbl2[][ARG_NUM] = {
  {'B','B'},		// 0x81:wire_begin     (SDA, SCL)
  {'B','b'},		// 0x82:wire_write     (adrs, [DATA])          ret:0-OK
  {'B','b','B'},	// 0x83:wire_writeRead (adrs, [DATA], readNum) ret:[DATA]-OK, NULL-ERROR
  {'B','B'},		// 0x84:wire_read      (adrs, readNum)         ret:[DATA]-OK, NULL-ERROR
  {},				// 0x85:wire_scan      ()                      ret:[LIST]
    
  {'b'},			// 0x86:digiWrite      (LIST[port,data])
  {'B'},			// 0x87:digiRead       (port)                  ret:level
  {'B','S'},		// 0x88:anaRead        (port, count)           ret:level(int16)
  {'B','S','S'},	// 0x89:tone           (port,freq,ms)
  {'b'},			// 0x8a:setPwms        (LIST[port,data])
  {},				// 0x8b:neoPixcel      ()
  {'B','B'},		// 0x8c:setCameraMode  (mode,gain)
};
#define CMD_MIN   0x81
#define CMD_MAX  (CMD_MIN + sizeof(ArgTypesTbl2)/sizeof(ArgTypesTbl2[0]) - 1)

// sync with scratch-vm/src/extensions/scratch3_tukurutch/comlib.js,
//           mBlock/src/extensions/ExtensionManager.as
static const char ArgTypesTbl3[][ARG_NUM] = {
  {},				// 0xFB:statusWifi     ()                      ret:status SSID ip
  {},				// 0xFC:scanWifi       ()                      ret:SSID1 SSID2 SSID3 ..
  {'s','s'},		// 0xFD:connectWifi    (ssid,pass)             ret:status
    
  {},				// 0xFE:getFwName      ()                      ret:FwName
  {},				// 0xFF:reset          ()                      ret:
};
#define CMD3_MIN   0xFB
#define CMD3_MAX  (CMD3_MIN + sizeof(ArgTypesTbl3)/sizeof(ArgTypesTbl3[0]) - 1)

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

static void sendWireScan(void)
{
      int num = 0;
      int i;
      for(i = 0; i < 127; i++) {
            Wire.beginTransmission(i);
            int ret = Wire.endTransmission();
            if(!ret) buffer[num++] = i;
      }
      sendBin(buffer, num);
}

static void digiWrite(uint8_t* buf, int num)
{
      int i;
      for(i = 0; i < num; i+= 2) {
            pinMode(buf[i+0], OUTPUT);
            digitalWrite(buf[i+0], buf[i+1]);
      }
}

static int _analogRead(uint8_t port, uint16_t count)
{
    #if defined(ESP32)
      return getAdc1(port,count);
    #else
      pinMode(port, INPUT);
      if(count == 0) count = 1;
      int i;
      uint32_t sum = 0;
      for(i = 0; i < count; i++)
        sum += analogRead(port);
    
     #if defined(__AVR_ATmega328P__)
      return ((sum / count) * 625UL) / 128;	// 1024->5000
     #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
      return ((sum / count) * 825UL) / 256;	// 1024->3300
     #else
      return ((sum / count) * 825UL) / 256;	// 1024->3300
     #endif
    #endif
}

void _tone(uint8_t port, int16_t freq, int16_t ms)
{
    #if defined(ESP32)
      ledcAttachPin(port, LEDC_BUZZER);
      ledcWriteTone(LEDC_BUZZER, freq);
      delay(ms);
      ledcWriteTone(LEDC_BUZZER, 0);
    #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
      ;
    #else
      tone(port, freq, ms);
      delay(ms);
    #endif
}

#if defined(ESP32)
static uint8_t ledc2port[LEDC_PWM_END+1] = {0};
static void _setPwm(uint8_t port, uint16_t data)
{
      int i;
      for(i = LEDC_PWM_START; i <= LEDC_PWM_END; i++) {
            if(ledc2port[i] == port+1) {
                  ledcWrite(i, data);
                  return;
            } else if(ledc2port[i] == 0) {
                  ledc2port[i] = port+1;
                  ledcSetup(i, 50/*Hz*/, 12/*bit*/);
                  ledcAttachPin(port, i);
                  ledcWrite(i, data);
                  return;
            }
      }
}
#else
// 328P:3,5,6,9,10,11  8bit
// nRF51,nRF52:5と11除く、8bit、3chまで
// RP2040
  #define _setPwm(port, data) analogWrite(port, (data)>>4)
#endif

static void _setPwms(uint8_t* buf, int num)
{
      int i;
      for(i = 0; i < num; i += 3)
        _setPwm(buf[i+0], buf[i+1]|(buf[i+2]<<8));
}

static void parseData()
{
      uint8_t cmd = buffer[3];
      const char *ArgTypes = NULL;
      if(cmd < ITEM_NUM) {
             ArgTypes = ArgTypesTbl[cmd];
        
      } else if(cmd >= CMD_MIN && cmd <= CMD_MAX) {
             ArgTypes = ArgTypesTbl2[cmd-CMD_MIN];
        
      } else if(cmd >= CMD3_MIN && cmd <= CMD3_MAX) {
             ArgTypes = ArgTypesTbl3[cmd-CMD3_MIN];
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
        case 1: _setLED(getByte(0),getByte(1));; callOK(); break;
        case 2: _tone(P_BUZZER,getShort(0),getShort(1));; callOK(); break;
        case 3: sendShort((_getAdc1(getByte(0),getShort(1)))); break;
        case 4: sendByte((_getSw(getByte(0)))); break;
        #if defined(ESP32)
          case 0x81: Wire.begin(getByte(0),getByte(1)); callOK(); break;
        #else
          case 0x81: Wire.begin(); callOK(); break;
        #endif
          case 0x82: Wire.beginTransmission(getByte(0)); Wire.write(getBufLen(1)); sendByte(Wire.endTransmission()); break;
          case 0x83: Wire.beginTransmission(getByte(0)); Wire.write(getBufLen(1)); Wire.endTransmission(false); sendWireRead(getByte(0),getByte(2)); break;
          case 0x84: sendWireRead(getByte(0),getByte(1)); break;
          case 0x85: sendWireScan(); break;
        
          case 0x86: digiWrite(getBufLen(0)); callOK(); break;
          case 0x87: pinMode(getByte(0),INPUT); sendByte(digitalRead(getByte(0))); break;
          case 0x88: sendShort(_analogRead(getByte(0),getShort(1)));break;
          case 0x89: _tone(getByte(0),getShort(1),getShort(2)); callOK(); break;
          case 0x8a: _setPwms(getBufLen(0)); callOK(); break;
          case 0x8b: callOK(); break;
        #if defined(CAMERA_ENABLED)
          case 0x8c: _setCameraMode(getByte(0),getByte(1)); callOK(); break;
        #endif
        #if defined(ESP32)
          // WiFi設定
          case 0xFB: sendString(statusWifi()); break;
          case 0xFC: sendString(scanWifi()); break;
          case 0xFD: sendByte(connectWifi(getString(0),getString(1))); break;
        #endif
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
          default: callOK(); break;
      }
}

int16_t _readUart(void)
{
      if(_Serial.available()>0) {
            comMode = MODE_UART;
            return _Serial.read();
      }
      return -1;
}

static uint8_t blePeri_enable = false;
static uint8_t _index = 0;
void loop()
{
      int16_t c;
      while((c=_readUart()) >= 0) {
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
    
    #if defined(ESP32)
      readWifi();
    #elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
      if(blePeri_enable) blePeripheral.poll();
    #endif
    _loop();
}

#if defined(ESP32)
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
      comMode = MODE_WS;
      parseData();
      _client.sendBinary((char*)_packet_dp, _packetLen);
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
#elif defined(NRF51_SERIES) || defined(NRF52_SERIES)
static BLEDescriptor _nameDesc = BLEDescriptor("2901", mVersion);
static BLEService        _service = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLECharacteristic _rxChar  = BLECharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite|BLEWriteWithoutResponse, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
  // write/writeWoResp, both OK
static BLECharacteristic _txChar  = BLECharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify|BLERead, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);

void bleStop() {
        if(blePeri_enable) blePeripheral.end();
        blePeri_enable = false;
}

static void _bleSetup() {
      blePeripheral.setLocalName(mVersion);
    
      blePeripheral.addAttribute(_service);
      blePeripheral.addAttribute(_nameDesc);
      blePeripheral.setAdvertisedServiceUuid(_service.uuid());
    
      blePeripheral.addAttribute(_txChar);
      blePeripheral.addAttribute(_rxChar);
      _rxChar.setEventHandler(BLEWritten, _bleWritten);
    
      blePeripheral.setEventHandler(BLEConnected, _bleConnected);
      blePeripheral.setEventHandler(BLEDisconnected, _bleDisconnected);
    
      blePeripheral.begin();
      blePeri_enable = true;
}

static void _bleConnected(BLECentral& central) {
      Serial.println(F("Connected"));
}

static void _bleDisconnected(BLECentral& central) {
      Serial.println(F("Disconnected"));
}

static void _bleWritten(BLECentral& central, BLECharacteristic& _char) {
      //Serial.print(F("R:")); _dump(_rxChar.value(), _rxChar.valueLength());
      _packetLen =_rxChar.valueLength();
      memcpy(buffer, _rxChar.value(), _packetLen);
      comMode = MODE_BLE;
      parseData();
      //Serial.print(F("W:")); _dump((uint8_t*)buffer, _packetLen);
      _txChar.setValue((uint8_t*)_packet_dp, _packetLen);
}
/*
static void _dump(const uint8_t* buf, int size)
{
      int i;
      char tmp[4];
      for(i = 0; i < size; i++) {
            snprintf(tmp, sizeof(tmp), "%02x", buf[i]);
            Serial.print(tmp);
      }
      Serial.println();
}
*/
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

/*static*/ void callOK()
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

void sendBin(uint8_t* buf, uint8_t num)
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
