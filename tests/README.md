# simapi test tools

## Telemetry Viewer

A live telemetry viewer for DiRT Rally 2.0 and other simulators. Make sure the SIMD daemon is running first.

### Compile the telemetry viewer:
```
gcc -o view_telemetry view_telemetry.c -I ../include -I ../simapi -L /usr/local/lib -lsimapi -lm
```

### Run the telemetry viewer:

**Bash/Zsh:**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
./view_telemetry
```

**Fish shell:**
```fish
set -x LD_LIBRARY_PATH /usr/local/lib
./view_telemetry
```

This will display live telemetry data including RPM, speed, gear, throttle/brake/clutch positions, and lap information.

## Original setsimdata tool

### Compile this test tool with
```
gcc -DASSETTOCORSA setsimdata.c ../simmap/mapacdata.c ../simmap/maprf2data.c -I ../simamap -o a.out
```
or
```
gcc -DRFACTOR2 setsimdata.c ../simmap/mapacdata.c ../simmap/maprf2data.c -I ../simamap -o a.out
```

### Load a dumped shared memory file
```
./setsimdata -a loadfile -f acpmf_graphics -s Graphics_dump5
```

### Set data
```
setsimdata -a set -f acpmf_physics -n SPageFilePhysics_gear -t integer -v 9
```
