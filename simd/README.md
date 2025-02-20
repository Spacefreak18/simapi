# simd

cross sim telemetry mapping daemon.

This daemon will automatically detect the currently running simulator, (RFactor2, Acs/ACC, Automobilista 2), and map the current telemetry
data ( speed, gear, rpms, etc ) into a common memory mapped file for use in other programs such as monocoque or simmonitor. The memory
mapped filed can be found at /dev/shm/SIMAPI.DAT.

## Supported Games ( see [simapi](https://github.com/spacefreak18/simapi) for more details of what is supported from each sim )
  - Using [SimSHMBridge](https://github.com/spacefreak18/simshmbridge)
    - Asseto Corsa
    - Assetto Corsa Competizione
    - Project Cars 2 (untested)
    - Automobilista 2 (UDP, and shared memory)

  - Using [scs-sdk-plugin](https://github.com/jackz314/scs-sdk-plugin/releases)
    - Euro Truck Simuator 2
    - American Truck Simulator

  - Using [rfactor 2 plugin](https://github.com/schlegp/rF2SharedMemoryMapPlugin_Wine)
    - RFactor 2

  - Using UDP
    - Automobilista 2
    - Project Cars 2 (untested)

## Dependencies
- yder - logging
- simapi.so
- libuv base event loop
- argtable2

## Building

To build first compile simapi as a dll using the instructions at the root of the repo, then compile simd in this directory with cmake:
```
mkdir build; cd build
cmake ..
make
```

## Usage
```
simd --nodaemon --nomemmap -vv
```
or simply
```
simd
```
if you get an error like ``` simd: error while loading shared libraries: libsimapi.so.1: cannot open shared object file: No such file or directory ```
run this command first ``` export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib ```

simd has a built in work around to automatically create memory mapped files for assetto corsa and project cars 2 based sims, so a workaround
such as createsim isn't needed, but a helper process running in the wine/proton environment such as simshmbridge.exe is still needed. The
nomemmap (-h) option disables this workaround.

The nodaemon option (-n) disables daemon mode so the process does not fork. And verbose (-v) increases logging verbosity.


## ToDo
 - much, much more
