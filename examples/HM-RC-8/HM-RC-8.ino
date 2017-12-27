//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// define all device properties
#define DEVICE_ID HMID(0x00,0xda,0x00)
#define DEVICE_SERIAL "HMRC00da00"
#define DEVICE_MODEL  0x00,0xda
#define DEVICE_FIRMWARE 0x01
#define DEVICE_TYPE DeviceType::Remote
#define DEVICE_INFO 0x08,0x00,0x00

//Add support for encrypted message transmission
#define USE_AES
#define HM_DEF_KEY 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10
#deifne HM_DEF_KEY_INDEX 0


#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>


// we use a Pro Mini
// Arduino pin for the LED
// D4/D5 == PIN 4/5
#define LED_GREEN 4
#define LED_RED 5
// Arduino pin for the config button
// B0 == PIN 8
#define CONFIG_BUTTON_PIN 8
// Arduino pins for the buttons
#define BTN01_PIN 14  // PC0
#define BTN02_PIN 3   // PD3
#define BTN03_PIN 15  // PC1
#define BTN04_PIN 7   // PD7
#define BTN05_PIN 9   // PB1
#define BTN06_PIN 6   // PD6
#define BTN07_PIN 16  // PC2
#define BTN08_PIN 17  // PC3


// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<LED_RED,LED_GREEN> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;
class Hal : public HalType {
  // extra clock to count button press events
  AlarmClock btncounter;
public:
  void init (const HMID& id) {
    HalType::init(id);
    // get new battery value after 50 key press
    battery.init(50,btncounter);
    battery.low(22);
    battery.critical(19);
  }

  void sendPeer () {
    --btncounter;
  }

  bool runready () {
    return HalType::runready() || btncounter.runready();
  }
};

typedef RemoteChannel<Hal,PEERS_PER_CHANNEL> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,8> RemoteType;

Hal hal;
RemoteType sdev(0x20);
ConfigButton<RemoteType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  remoteISR(sdev,1,BTN01_PIN);
  remoteISR(sdev,2,BTN02_PIN);
  remoteISR(sdev,3,BTN03_PIN);
  remoteISR(sdev,4,BTN04_PIN);
  remoteISR(sdev,5,BTN05_PIN);
  remoteISR(sdev,6,BTN06_PIN);
  remoteISR(sdev,7,BTN07_PIN);
  remoteISR(sdev,8,BTN08_PIN);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
}

void loop() {
  bool pinchanged = false;
  for( int i=1; i<=sdev.channels(); ++i ) {
    if( sdev.channel(i).checkpin() == true) {
      pinchanged = true;
    }
  }
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( pinchanged == false && worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
