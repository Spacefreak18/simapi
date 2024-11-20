C header files for various simulator titles' shared memory api, specifically for use in Linux programs

This project includes a [daemon](https://github.com/spacefreak18/simapi/tree/master/simd) to handle all the telemetry mapping for external 
applications in the background, and the shared library for the external applications to use. The external application can either map and
open the memory mapped filed directly (SIMAPI.DAT) and use the header file simapi.h to enumerate the fields within the data structure, or 
use the functions within simmapper.c to open and map the memory mapped file.


## Building

To compile the shared library with the CMakeLists.txt in the root of the repo...
```
mkdir build; cd build
cmake ..
make clean; make; make install
```
You will need sudo for the make install, which will place the dll and public headers into /usr/local.

## Supported Sims
the closest to documentation found for each sim
### Assetto Corsa / Assetto Corsa Competizione
[acsharedmemorydocumentation](https://www.overtake.gg/attachments/acsharedmemorydocumentation-pdf.667802)
### RFactor2
[modding resources](https://www.studio-397.com/modding-resources/)
[internals plugin](https://github.com/TheIronWolfModding/rF2SharedMemoryMapPlugin/blob/master/Include/InternalsPlugin.hpp)
[linux plugin download](https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine)
### Projects Cars 2 (Automobilista 2)
[sharedmemory.h](https://github.com/viper4gh/CREST2-AMS2/blob/master/SharedMemory.h)

## Contributing
Feel free to make pull requests by adding mappings to simmapper.c for any simulator!
