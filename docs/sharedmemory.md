# Shared Memory

Shared Memory Mapped Files is a high performance method of inter process communication. Used often in game development.
To learn more than you want to know about shared memory visit this [page](https://learn.microsoft.com/en-us/dotnet/standard/io/memory-mapped-files) and to learn about different methods of telemetry
output visit this [page](https://docs.departedreality.com/dr-sim-manager/development/telemetry-outputs-overview).

Unfortunately, for Linux gaming there is a separate challenge as Proton/Wine does not make these memory mapped files available to
native Linux processes [yet](https://bugs.winehq.org/show_bug.cgi?id=54015).

However, some sims allow native dll plugins which can compiled with Winelib, removing the need for a compatibility process. There they should (just work) after you add the DLL plugin.

So some sort of compatibility process is necessary, I recommend installing [simd](https://spacefreak18.github.io/simapi/simd)

But for a more complete rundown and other methods visit this [page](https://github.com/spacefreak18/simshmbridge).
