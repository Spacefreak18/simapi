# simd usage
if you have not yet built simd, see the [readme](https://github.com/Spacefreak18/simapi/blob/master/simd/README.md).

The first step is to compile (or download from releases) the shared memory compatibility [binaries](https://github.com/spacefreak18/simshmbridge?tab=readme-ov-file#compilation).

if you wish to use the automatic bridging mode (recommended), first copy the config file from [here](https://github.com/Spacefreak18/simapi/blob/master/simd/conf/simd.config) to ~/.config/simd/simd.config

next, add the appropriate [bridge](https://github.com/spacefreak18/simshmbridge) exe to your steam launch command like this:
```
SIMD_BRIDGE_EXE=~/path/to/acbridge.exe %command%
```
or
```
SIMD_BRIDGE_EXE=~/path/to/pcars2bridge.exe %command%
```
note, it is highly recommended to temporarily backup your launch command and to use this minimal launch command first to get it working.

then, to run simd run the command
```
simd --nodaemon --nomemmap -vv
```
or to use as a daemon, simply
```
simd
```
if you get an error like ``` simd: error while loading shared libraries: libsimapi.so.1: cannot open shared object file: No such file or directory ```
run this command first
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

simd has a built in work around to automatically create memory mapped files for assetto corsa and project cars 2 based sims, so a workaround
such as createsim isn't needed, but a helper process running in the wine/proton environment such as simshmbridge.exe is still needed. The
nomemmap (-h) option disables this workaround.

The nodaemon option (-n) disables daemon mode so the process does not fork. And verbose (-v) increases logging verbosity.

## troubleshooting notes
Be sure to run simd before starting any games. And that's it. It's near enough to a stable state that once you're sure you have everything
working you can have simd run on system startup.

Before that I recommend starting simd before anything else with ```simd --nodaemon -vv``` to hopefully see any logs.

For troubleshooting try ```hexdump /dev/shm/acpmf_physics``` for Assetto Corsa sims or ```hexdump /dev/shm/$pcars2$``` for Automobilista2 and Project Cars2 sims.

If there is no data in one of these files while the sim is running, there is likely a problem with your launch command in steam or the path to your [simshmbridge](https://github.com/spacefreak18/simshmbridge) exe.

There should also be plenty of non-zeros when you run ```hexdump /dev/shm/SIMAPI.DAT```.

If you have non-zero data in these files, then simd is running and doing it's job and running monocoque or simmonitor should work.

See the troubleshooting section.
