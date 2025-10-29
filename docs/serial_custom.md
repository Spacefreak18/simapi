# Custom Serial

Monocoque is capable of sending a user defined string to a serial device. Said string will be delivered with every device update.

This string is built with a lua script that is ran with every device update. A sample [arduino sketch](https://github.com/Spacefreak18/monocoque/blob/master/src/arduino/custom/custom.ino) to receive the string and a 
[lua script](https://github.com/Spacefreak18/monocoque/blob/master/conf/custom.lua) concatenating the string are available in the repository.


an example config show just the custom serial device.


```
configs = (
            {         
                sim = "default";         
                car = "default";         
                devices = (         
                            {
                                device       = "Serial";
                                type         = "Custom";
                                config       = "~/git/monocoque/conf/custom.lua";
                                baud         = 115200;
                                devpath      = "/dev/ttyACM0";
                            });
            } 
); 
```

