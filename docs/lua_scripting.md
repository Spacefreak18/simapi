# Lua Scripting

Monocoque is capable of passing a lua script for certain devices in order to use custom data to define how LEDs are driven.

The following devices support lua scripting at this time, to add more open a pull request or an issue.
[Simhub Arduino LED Sketch](https://github.com/SHWotever/SimHub/wiki/Getting-started-with-the-SIMHUB-STANDARD-ARDUINO-PRO-MICRO-LEDs-sketch)
[Cammus C5]()
[Cammus C12]()
[Moza R5](https://spacefreak18.github.io/simapi/mozar5)


An object in lua called simdata is available which effectively mirrors [simdata.h](https://github.com/Spacefreak18/simapi/blob/master/simapi/simdata.h) in simapi. All data points are not yet available
but can be added simply. Open a pull request or an issue.

Currently available data:

- simdata.gearc - character string of the current gear for the current player
- simdata.playerflag - current flag shown to player
- simdata.rpms - rpms
- simdata.gear - numeric gear where 0 is neutral
- simdata.velocity - forward velocity in km/h
- simdata.mtick - constantly incrementing time variable
- simdata.maxrpm - max rpm of current car ( not provided by all sims )
- simdata.pd[x].radius - distance a nearby car is from current driver
- simdata.pd[x].theta - angle a nearby car is away from current driver

the following color enum presets are available
RED
YELLOW
BLUE
GREEN
ORANGE

Also the following functions can be called from lua.

- set_led_to_color - param1: led index, param2: hardcoded color enum preset (see above)
- set_led_range_to_color - param1: range start index, param2: range end index, param3: hardcoded color enum preset (see above)
- set_led_to_rgb_color - param1: led index, param2: color in [8 bit rgb](https://www.rapidtables.com/web/color/RGB_Color.html)
- set_led_range_to_rgb_color - param1: led start index, param2: led end index, param3: color in [8 bit rgb](https://www.rapidtables.com/web/color/RGB_Color.html)
- led_clear_all - turns off all leds
- led_clear_range - turns off range of leds, param1: range start index, param2: range end index

A relatively complex example and other examples can be found in the [conf](https://github.com/Spacefreak18/monocoque/tree/master/conf) directory.

To active the lua config just simply add the "config" entry to each applicable device node like this:
```
        {
            device       = "Serial";
            type         = "Simleds";
            config       = "~/git/monocoque/conf/rpms_and_radar.lua"
            baud         = 115200;
            devpath      = "/dev/simdev0";
        }
```
