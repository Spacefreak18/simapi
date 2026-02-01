# simd

cross sim telemetry mapping daemon.

This daemon will automatically detect the currently running simulator, (RFactor2, Acs/ACC, Automobilista 2), and map the current telemetry
data ( speed, gear, rpms, etc ) into a common memory mapped file for use in other programs such as monocoque or simmonitor. The memory
mapped filed can be found at /dev/shm/SIMAPI.DAT.

## Supported Games ( see [simapi](https://github.com/spacefreak18/simapi) for more details of what is supported from each sim )

## Dependencies
- yder - logging
- simapi.so
- libuv base event loop
- argtable2

## Building

simd and simapi are both available in the [AUR](https://aur.archlinux.org/packages/simd-git)
be sure to install [simapi](https://aur.archlinux.org/packages/simapi-git) first

To build first compile and install simapi using the instructions at the root of the repo, then compile simd with cmake:
```bash
cmake -B build
cmake --build build
```

## User-Level Installation

To install simd as a user-level systemd service:

```bash
# Build and install to ~/.local (includes binary, service file, and config)
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build
cmake --install build

# Reload systemd user daemon and enable the service
systemctl --user daemon-reload
systemctl --user enable --now simd
```

This installs:
- Binary to `~/.local/bin/simd`
- Systemd service to `~/.config/systemd/user/simd.service`
- Config file to `~/.config/simd/simd.config`

To manage the service:
```bash
systemctl --user status simd    # Check status
systemctl --user stop simd      # Stop the service
systemctl --user restart simd   # Restart the service
journalctl --user -u simd -f    # View logs
```

To disable installation of specific components:
```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local \
    -DINSTALL_SYSTEMD_SERVICE=OFF \
    -DINSTALL_DEFAULT_CONFIG=OFF
```

## Usage
if you wish to use the automatic bridging mode, first copy the config file from [here](https://github.com/Spacefreak18/simapi/blob/master/simd/conf/simd.config) to ~/.config/simd/simd.config and add the appropriate
[bridge](https://github.com/spacefreak18/simshmbridge) exe to your steam launch command like this:
```
SIMD_BRIDGE_EXE=~/path/to/acbridge.exe %command%
```
or
```
SIMD_BRIDGE_EXE=~/path/to/pcars2bridge.exe %command%
```

```
simd --nodaemon --nomemmap -vv
```
or simply
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

for more information on [simd usage](https://spacefreak18.github.io/simapi/simd)


## ToDo
 - much, much more
