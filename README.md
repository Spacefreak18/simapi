C header files for various simulator titles' shared memory api, specifically for use in Linux programs

This project includes a [daemon](https://github.com/spacefreak18/simapi/tree/master/simd) to handle all the telemetry mapping for external 
applications in the background, and the shared library for the external applications to use. The external application can either map and
open the memory mapped filed directly (SIMAPI.DAT) and use the header file simapi.h to enumerate the fields within the data structure, or 
use the functions within simmapper.c to open and map the memory mapped file.


## Building

To compile the shared library with the CMakeLists.txt in the root of the repo...
```bash
cmake -B build
cmake --build build
```
You will need sudo to install, which will place the library and public headers into `/usr/local`:
```bash
sudo cmake --install build
```

## [Supported Sims](https://spacefreak18.github.io/simapi/supportedsims)

### Platinum Support (All features)
- Assetto Corsa
- Assetto Corsa Competizione
- Automobilista 2
- RFactor 2
- LeMans Ultimate

### Gold Support (Most features)
- Project Cars 2

### Silver Support (Minimal features)
- Assetto Corsa Evo
- Assetto Corsa Rally
- American Truck Simulator
- Euro Truck Simulator 2

### Bronze Support (Very minimal features)
- Live For Speed
- BeamNG
- Dirt Rally 2

### Additional Support
- Richard Burns Rally RSF Edition
- Wreckfest 2
- F1 2018/2022

See the full [compatibility matrix](https://spacefreak18.github.io/simapi/supportedsims) for details on protocols and plugins required for each sim.

### Developer Documentation
- Assetto Corsa: [acsharedmemorydocumentation](https://www.overtake.gg/attachments/acsharedmemorydocumentation-pdf.667802)
- RFactor2: [modding resources](https://www.studio-397.com/modding-resources/) | [internals plugin](https://github.com/TheIronWolfModding/rF2SharedMemoryMapPlugin/blob/master/Include/InternalsPlugin.hpp) | [linux plugin](https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine)
- Project Cars 2 / AMS2: [sharedmemory.h](https://github.com/viper4gh/CREST2-AMS2/blob/master/SharedMemory.h)

## Contributing
Feel free to make pull requests by adding mappings to simmapper.c for any simulator!
