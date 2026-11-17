# simd poke

simd has a mode that allows setting (or "poking") arbitrary simdata fields.
This feature is still new and highly experimental.

The daemon must already be running for this to work.

The command example looks like this:


```
simd -p SimData_simstatus -t 2
simd -p SimData_simon -t 1

```

Those two fields must be set before monocoque or simmonitor detects running data.

Then to stop the running data run these two commands:

```
simd -p SimData_simstatus -t 0
simd -p SimData_simon -t 0

```

Other examples, like setting speed
```
simd -p SimData_velocity -t 185
```
Setting rpms
```
simd -p SimData_rpms -t 7500
```

The same basic scheme of SimData_[fieldname] should work for all fields in simdata.h
