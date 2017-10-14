# Motivation

I always wanted to have a battery operated Z-Wave device, which I could control... For Instance, an LED light. Apart from a "Siren" and a certain Remotec IR controller, I could not find any other FLiRS implementation. Well, that was 2 years ago and ... now, in 2017, FLiRS thermostats exist and we have the Z-Uno! I thought it would be cool to have my own battery powered LED light! Just for fun and giggles. With Z-Uno firmware 2.1.1 this is possible! So I wrote this sketch, added lots of debugging over serial and lots of comments. As an LED I use the built-in white LED, so you can try this sketch without adding any hardware. Power consumption is really low. Idle current on my test board is about 100 uA at 3.6 V and around 70 uA at 3.0V. Of course, it still uses 40 mA for at least a second if it wakes up, so you need a good battery with low internal resistance.

This is my second published Z-Uno sketch, so I would really appreciate it if you could have a look at  it and give me some feedback. BTW my first published sketch is here: https://github.com/petergebruers/Z-Uno-BH1750>

# Features

-   FLiRS means it can respond to commands from the controller in 1 second. That is not "superfast" but it is OK for this application.
-   Local on/off possible by clicking the "Service button". Reports state change to the controller.
-   Comments in source code.
-   Debugging output to Serial0 or Serial1. Can be interesting to watch the order of execution.
-   Blinks 1,2 or 3 times to give a visual clue about the reason for wake-up. See code for details.

# Requirements

-   Tested on Z-Uno firmware 2.1.1, does not work on older versions.
-   Use Rescue mode to flash.
-   Please make sure no pins (except for your serial adapter and an external LED) are connected to a load, ground or supply voltage. The sketch forces all pins to "output" mode but does not initialize the pins to VCC or GND. But the Z-Uno can and will source or sink current...
-   Be careful with batteries... Check if they are protected... Also observe the ratings of the Z-Uno: 2.8 - 3.6 V. If you charge a cell while it is connected to the Z-Uno, you might exceed the maximum voltage!
-   I think it is best to connect only one power source to the Z-Uno. If you connect the USB to flash the sketch, The Z-Uno outputs 3.12 V on the battery pins, unless you remove R12. Please have a look at the schematic diagram: <http://z-uno.z-wave.me/technical/power-supply/>

# Installation and download

-   Make sure you have installed Arduino and the Z-Uno board definition.

    -   <http://z-uno.z-wave.me/install>

-   Make sure you have the Windows driver

    -   <http://z-uno.z-wave.me/install/driver>

-   Download this sketch from GitHub:

    -   <https://github.com/petergebruers/Z-Uno-FLiRS-LED-DEMO>

-   If you want debugging, connect a serial adapter to your Z-Uno. Debugging over USB is not possible due to sleep mode. Do not connect the power of your Serial interface to the Z-Uno, only ground. I recommend 330 ohm resistors in series with RX and TX.
-   Check the value of PG\_DEBUG\_OUT. It sets the debugging output to Serial 0, 1 or none.
-   Check Arduino, Tools, Frequency. The default is Russia. If you are unable to include your Z-Uno, double check this setting and re-upload the sketch if it was wrong.
-   Exclude your Z-Uno from your controller, because you probably are not having the same channel definitions as me, if you have installed another sketch before this one. If the green LED is not blinking, keep clicking the "Service button" until the red LED starts blinking and your controller confirms the exclusion.
-   If the green LED is not blinking, enter "Rescue mode". Hold down the "Service button" and keep holding it. Click the "Reset button". After about one second, release the "Service button". The green LED should blink.
-   Upload the sketch to your Z-Uno. Wait until the green LED turns off.
-   Include the Z-Uno. Keep clicking the "Service button" until the red LED starts blinking and your controller confirms the inclusion. After the interview (Z-Way) or addition of the module finishes (Fibaro Home Center), you should see 2 switches. They perform the same function, because that is how Z-Wave makes this device compatible with really old controllers (the ones that do not understand "multiple instances (endpoints)").

# Troubleshooting

-   If you cannot upload the sketch, check if the green LED is blinking. If it is not, enter "Rescue mode".
-   If you cannot include or exclude the device (confirmed by red LED blinking rapidly), keep trying... It requires a certain "rhythm".
-   If you need help, check "About the author" below.
-   If you want to measure the standby current, add an RC filter with a big time constant to average the peaks. I use 50 k and a 330 uF tantalum.

# Possible future improvements or variants

-   If you notice an unexpected "blink" of the LED, this is "by design". The Z-Uno initializes the pins, and it takes some time to get the LED pin back to output. The only way I can think of, to avoid this, is by adding a flip-flop or serial register... I have some ideas, but I have not yet tested them.
-   It should be possible to integrate this with (some) sensors, but it might be a little bit more difficult then you think, because of the sleep mode. Please check the "FLIRSHumidity" example published by Z-Wave.me. Notice the use of a low power "wake up timer".
-   I do not claim this solution is optimal. I did lots of testing, and I think it is OK. But this kind of device is new to me, so I cannot say I have lots of experience...

# Revision list

-   1.0.0: Initial public release.

# Licensing, warranty and liability

This sketch and all parts are governed by: Creative Commons "Attribution 4.0 International (CC BY 4.0)", available at: <https://creativecommons.org/licenses/by/4.0/>

# About the author

-   I'm Peter Gebruers, aka "petergebruers", and I live in Flanders near Brussels. I am an engineer and I've worked mostly in IT as a systems/network engineer.
-   I am quite new to Z-Uno development but I do have some experience with microcontrollers and SoC like PIC 8 bit, STM32, ESP8266. It is a hobby. I own two Z-Uno Boards. I do not earn anything from posting this.
-   I started my Home Automation System in 2012, based on a Fibaro HC2. Later I added a RaZberry. I have about 100 physical nodes.
-   You can find me on one of these forums:

    -   <https://forum.fibaro.com/index.php?/profile/1799-petergebruers/>
    -   <https://forum.z-wave.me/memberlist.php?mode=viewprofile&u=564463>


