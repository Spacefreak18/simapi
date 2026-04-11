# Moza KS Pro Wheel

The Moza KS Pro Wheel will track RPMs with its LEDs and display flag signals.

- 12 RPM shift-light LEDs illuminate progressively from 75% to 97% of max RPM
- LEDs blink at 98%+ RPM
- Yellow flag signal illuminates the flag LEDs amber

Use this config (or add relevant block to an existing config):

```
configs =
({
   sim = "default";
   car = "default";
   devices = ( {
                    device       = "Serial";
                    type         = "Wheel";
                    subtype      = "MozaKSProWheel";
                    devpath      = "/dev/ttyACM0";
                    baud         = 115200;
                }
            );
});

```
