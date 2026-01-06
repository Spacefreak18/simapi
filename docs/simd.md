# simd
To jump right into how to use simd visit this [page](https://spacefreak18.github.io/simapi/simd).

Applications such as monocoque and simmonitor, being sim-agnostic in nature, require some sort of
common mapping of the telemetry the different titles are making available.

This is where simd comes in, a separate process to consolidate the available data from any running sim
into a standardized data format.

Running on Linux natively also presents it's own set of challenges. Many titles make use of shared memory
mapped files, which wine does not make easily available for consumption outside of the same running 
wine prefix.

By default simd contains extra compatibility for these workarounds. Such as automatically running a 
defined bridge process from the simshmbridge process.

Each sim title has its own set of steps to setup telemetry. For that list see this page.

simd also provides functionality to arbitrarily set common simdata, to test functionality of devices
and displays in monocoque and simmonitor respectively. See [poke](simd_poke).

simd also allows fine grained control over the start of memory mapping through the use of [the simd.conf](https://github.com/Spacefreak18/simapi/blob/master/simd/conf/simd.config) file.
