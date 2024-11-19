C header files for various simulator titles' shared memory api, specifically for use in Linux programs

This project includes a daemon to handle all the telemetry mapping for external applications in the background, and the shared library
for the external applications to use.


## Building

To compile the shared library with the CMakeLists.txt in the root of the repo...
```
mkdir build; cd build
cmake ..
make clean; make; make install
```
You will need sudo for the make install, which will place the dll and public headers into /usr/local.
