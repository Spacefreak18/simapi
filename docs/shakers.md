# Bass Shakers

Bass Shakers are special sound transducers, monocoque uses game telemetry to generate sound pulses for specific events such as engine rpm, gears, and tyre slip. The shakers are setup much like normal speakrs.

The key to a great experience is to remember that less is more. Not too many shakers, and not too many events should be sent to a single shaker.

The monocoque configuration for shakers looks like this:

```
{   
device       = "Sound";
effect       = "Engine";
devid        = "alsa_output.usb-Generic_USB2.0_Device_20170726905959-00.analog-stereo";
channel      = 1;
tyre         = "FrontLeft";
volume       = 90;
frequency    = 27;
}
```

+ frequency - base frequency of effect
+ channel - left or right ( surround sound cards are supported, but it can be a challenge to map physical channels to the correct channel id )
+ effect - engine, gear, tyrelock, tyreslip, abs
+ devid - find with ```pacmd list-sinks | grep name:```
+ tyre - All, Fronts, Rears, FrontLeft, RearRight, etc

Setting up bass shakers and getting a good experience is a bit of a black art. Here are some tips ( mostly pertaining to SimHub ) but deal with the fine tuning and would still apply.

+ [SimHub Shaker Documentation](https://github.com/SHWotever/SimHub/wiki/ShakeIt-V3-Effects-configuration)

+ [RPM Effect Discussion](https://www.overtake.gg/threads/simhub-bass-shaker-rpm-effect-porsche-911-4-0-singer.211132)

+ [Bass Shaker Technical Thread - Tactile Immersion](https://www.overtake.gg/threads/tactile-immersion-general-discussion-hardware-software.137631/)
