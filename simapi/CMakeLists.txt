set(simapi_source_files
  ac.h
  rf2.h
  pcars2.h
  scs2.h
  simdata.h
  simapi.h
  simmapper.h
  simmapper.c
  getpid.h
  getpid.c
)


add_library(simapi STATIC ${simapi_source_files})
