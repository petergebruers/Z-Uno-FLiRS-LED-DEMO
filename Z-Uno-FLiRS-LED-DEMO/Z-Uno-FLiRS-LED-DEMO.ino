/*
  Demo script: Battery Powered LED light (FLiRS)

  Author: Peter Gebruers, aka "petergebruers". Meet me here:
  https://forum.fibaro.com/index.php?/profile/1799-petergebruers/

  Or here:
  https://forum.z-wave.me/memberlist.php?mode=viewprofile&u=564463

  The full project is on Github:
  https://github.com/petergebruers/Z-Uno-FLiRS-LED-DEMO

  Licensing, warranty and liability, governed by:
  "Attribution 4.0 International (CC BY 4.0)", available at:
  https://creativecommons.org/licenses/by/4.0/

  Additional information on Github in files README and LICENSE.

  Version 1.0 public release.

  Known issues:

  When switching the LED from "on" to "off", you will notice a short
  blink (off - on - off). This cannot be avoided without extra
  hardware, it has to do with how the Z-Uno initializes the GPIO after
  wake-up.  The LED will also blink when it is on and the controller
  polls the device, for the same reason.

  Inclusion is a bit more difficult. Keep clicking the Service button...
  You have to include the Z-Uno while it is running the sketch.

  Uploading a sketch requires rescue mode: hold down the Service Button
  and keep holding it. Click the Reset button. After about one second,
  release the "Service button". The green LED should blink.
*/

// INCLUDES

#include "EEPROM.h"

// ****** USER CONFIGURABLE DATA BEGIN ****** //
// Please check if these values match your setup!

// LED pin number 13 = white user LED of Z-Uno board

#define LED_PIN LED_BUILTIN

// PG_DEBUG_OUT must be 10 for Serial0, 11 for Serial1 or
// -1 for no debugging. Be careful when connecting your
// serial interface: check voltage levels and make sure
// only one power source is used.

#define PG_DEBUG_OUT 10

// ****** USER CONFIGURABLE DATA END ****** //

// ****** DEBUGGING HELPER MACROS  BEGIN ****** //

#if PG_DEBUG_OUT == 10
#define PG_DEBUG_OUT_SER Serial0

#elif PG_DEBUG_OUT == 11
#define PG_DEBUG_OUT_SER Serial1

#elif PG_DEBUG_OUT == -1
#undef PG_DEBUG_OUT_SER

#else
#error "PG_DEBUG_OUT must be 10 for Serial0, 11 for Serial1 or \
-1 for no debugging."
#endif

#ifdef PG_DEBUG_OUT_SER

#define PG_DEBUG_INIT() PG_DEBUG_OUT_SER.begin()
#define PG_DEBUG_PRN_DOTNEWLINE(message) PG_DEBUG_OUT_SER.print(message);\
  PG_DEBUG_OUT_SER.print(".\r\n")
#define PG_DEBUG_LOG_DOTNEWLINE(message,value) PG_DEBUG_OUT_SER.print(message);\
  PG_DEBUG_OUT_SER.print(value);PG_DEBUG_OUT_SER.print(".\r\n")

#else

// Define all macros with an empty body,
// this automatically removes debugging code.

#define PG_DEBUG_INIT()
#define PG_DEBUG_PRN_DOTNEWLINE(message)
#define PG_DEBUG_LOG_DOTNEWLINE(message,value)

#endif

// ****** DEBUGGING HELPER MACROS END ****** //

// Global variables

// Z-Uno is a SOC with limited resources compared to e.g a Raspberry Pi,
// or an esp-32. To limit stack usage and parameter passing and data shuffling,
// I use global variables instead of local variables, or parameter passing.
// This goes against the idea of "abstraction" and "isolation of data".

// currentLEDValueAddr is the address of the byte in NZRAM
// which keeps the getter/setter value during deep sleep.
// When the Z-Uno sleeps, all normal variables are lost.
#define currentLEDValueAddr 0x0

// LedValue is used to store the getter/setter value.
// It is used with NZRAM functions.
uint8_t LedValue;

// WakeUpReason is set in setup() and is used to do different
// actions based on the "wake up reason".
uint8_t WakeUpReason;

// BlinkCount is used to give simple feedback when there is no
// Serial connection available.
// See function definition of setup() for blink counts.
uint8_t BlinkCount;

// LoopCount10ms stored the number of 10ms delays after last
// execution of getter/setter. It keeps the Z-Uno awake so
// it can accept more commands after
uint8_t LoopCount10ms;

// MaxLoopCount10ms is the number of loops performed after
// last getter/setter call.
// I think 100 -> 1 second is a good value.
// Tested on Fibaro HC2 and Z-Way controller.
#define MaxLoopCount10ms 100

// SetLed() reads NZRAM (last set by setter or setup)
// into LedValue and turns the LED on or off.
void SetLed() {
  NZRAM.get(currentLEDValueAddr, &LedValue, 1);

  PG_DEBUG_LOG_DOTNEWLINE("LED value = ", LedValue);

  if (LedValue == ZUNO_SWITCHED_OFF)
    digitalWrite(LED_PIN, 0);
  else
    digitalWrite(LED_PIN, 1);
}

// setup() does a lot of work in FLiRS because the Z-Uno
// can wake up for different reasons.
// Main actions are:
// - Set all I/O so they do not float (saves power).
// - Detect POR to turn of LED and notify controller
// - Detect INT button click to toggle the LED
void setup() {
  uint8_t i;
  LoopCount10ms = 0;

  // Force all pins to output, with some exceptions,
  // notably the input pin and the serial ports if
  // debugging has been enabled. This saves power.

  for (i = 0; i <= 6; i++)
    pinMode(i, OUTPUT);

  // Leave Serial1 pins alone if they are used for debugging.
#if PG_DEBUG_OUT != 11
  for (i = 7; i <=8; i++)
    pinMode(i, OUTPUT);
#endif

  for (i = 9; i <= 17; i++)
    pinMode(i, OUTPUT);

  // Skip 18 = INT1 = Service Button

  for (i = 19; i <= 22; i++)
    pinMode(i, OUTPUT);

  // Skip 23 = Service Button

  // Leave Serial0 pins alone if they are used for debugging.
#if PG_DEBUG_OUT != 10
  for (i = 24; i <=25; i++)
    pinMode(i, OUTPUT);
#endif

  PG_DEBUG_INIT();

  WakeUpReason = zunoGetWakeReason();

  switch (WakeUpReason) {
    case ZUNO_WAKEUP_REASON_POR:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_POR");
      LedValue = ZUNO_SWITCHED_OFF;
      NZRAM.put(currentLEDValueAddr, &LedValue, 1);
      zunoSendReport(1);
      BlinkCount = 1;
      break;
    case ZUNO_WAKEUP_REASON_HARDRESET:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_HARDRESET");
      BlinkCount = 2;
      break;
    case ZUNO_WAKEUP_REASON_INT1:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_INT1");
      NZRAM.get(currentLEDValueAddr, &LedValue, 1);
      PG_DEBUG_LOG_DOTNEWLINE("LED NZRAM value : ", LedValue);
      if (LedValue == ZUNO_SWITCHED_OFF)
        LedValue = ZUNO_SWITCHED_ON;
      else
        LedValue = ZUNO_SWITCHED_OFF;
      NZRAM.put(currentLEDValueAddr, &LedValue, 1);
      zunoSendReport(1);
      break;
    case ZUNO_WAKEUP_REASON_WUT:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_WUT");
      break;
    case ZUNO_WAKEUP_REASON_RADIO:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_RADIO");
      break;
    case ZUNO_WAKEUP_REASON_SOFTRESET:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_SOFTRESET");
      break;
    case ZUNO_WAKEUP_REASON_USBSUSPEND:
      PG_DEBUG_PRN_DOTNEWLINE("ZUNO_WAKEUP_REASON_USBSUSPEND");
      break;
    default:
      PG_DEBUG_LOG_DOTNEWLINE("Unknown Wakeup Reason : ", WakeUpReason);
      break;
  }

  while (BlinkCount != 0) {
    BlinkCount--;
    digitalWrite(LED_PIN, 1);
    delay(30);
    digitalWrite(LED_PIN, 0);
    delay(300);
  }

  SetLed();
}

void loop() {
  // If the Z-Uno wakes from a radio event, keep the
  // radio on for about 1 second after last communication.
  // LoopCount10ms is set to 0 in getter and setter.
  if(WakeUpReason==ZUNO_WAKEUP_REASON_RADIO){
    if (LoopCount10ms < MaxLoopCount10ms){
      delay(10);
      LoopCount10ms++;
      return;
    }
    PG_DEBUG_PRN_DOTNEWLINE("Radio LoopCount10ms reached");
  }

  // Make inclusion and exclusion easier, I think...
  if(WakeUpReason==ZUNO_WAKEUP_REASON_INT1){
    if (LoopCount10ms < 40){
      delay(10);
      LoopCount10ms++;
      return;
    }
    PG_DEBUG_PRN_DOTNEWLINE("INT1 WaitCount reached");
  }

  PG_DEBUG_PRN_DOTNEWLINE("Send device to sleep");
  zunoSendDeviceToSleep();
}

// setter() is called when the controller sends data.
// A value of ZUNO_SWITCHED_OFF means "off". All other
// values mean "on" (typically 255).
void setter(uint8_t value) {
  // I think NZRAM.put has issues with data on the stack, on
  // Z-Uno 2.1.1 beta. So I assign the parameter to a global,
  // then use that global in NZRAM.put
  LedValue = value;
  PG_DEBUG_LOG_DOTNEWLINE("setter called, argument = ", LedValue);
  NZRAM.put(currentLEDValueAddr, &LedValue, 1);
  SetLed();
  LoopCount10ms=0;
}

// setter() is called when the controller polls the device,
// or the sketch calls zunoSendReport()
uint8_t getter() {
  NZRAM.get(currentLEDValueAddr, &LedValue, 1);
  PG_DEBUG_LOG_DOTNEWLINE("getter called, data to controller = ", LedValue);
  LoopCount10ms=0;
  return LedValue;
}

// Set Sleeping Mode to ZUNO_SLEEPING_MODE_FREQUENTLY_AWAKE = FLiRS
ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_FREQUENTLY_AWAKE);

// Set Channel to either Siren of Flowstop
//ZUNO_SETUP_CHANNELS(ZUNO_SIREN(getter, setter));
ZUNO_SETUP_CHANNELS(ZUNO_FLOWSTOP(getter, setter));

// Set up battery levels in millivolts, measured on 3V pin of Z-Uno:
// For my LiIon I selected: 2900 = 0% and 3300 = 100%
// I did not do a lot of testing, the values just seem "reasonable".
ZUNO_SETUP_BATTERY_LEVELS(2900, 3300);

// Note: Z-Uno 2.1.1 beta mentions ZUNO_SETUP_BATTERY_HANDLER(H);
// in the header. When I wrote this sketch, it is not yet documented.
// It might allow you to do stuff based on battery level, like turning of the LED.
