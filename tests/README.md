# simapi test tool

## compile this test tool with
```
gcc -DASSETTOCORSA setsimdata.c ../simmap/mapacdata.c ../simmap/maprf2data.c -I ../simamap -o a.out
```
or
```
gcc -DRFACTOR2 setsimdata.c ../simmap/mapacdata.c ../simmap/maprf2data.c -I ../simamap -o a.out
```

## Load a dumped shared memory file
```
./setsimdata -a loadfile -f acpmf_graphics -s Graphics_dump5
```

## Set data
```
setsimdata -a set -f acpmf_physics -n SPageFilePhysics_gear -t integer -v 9
```
