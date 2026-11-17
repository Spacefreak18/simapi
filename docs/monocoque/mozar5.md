# Moza R5

The MozaR5 will track rpms with it's leds. This also works with the MozaR3 and MozaR8.

There is also a newer firmware, if you are on the newer firmware use the ```subtype = "MozaNew"```.

Use this config (or add relevant block to an existing config):


```
configs =
({
   sim = "default";
   car = "default";
   devices = ( {
                    device       = "Serial";
                    type         = "Wheel";
                    subtype      = "MozaR5";
                    devpath      = "/dev/ttyACM0";
                    baud         = 115200;
                }
            );
});

```
