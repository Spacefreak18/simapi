set(include_source_files
  acdata.h
  rfdata.h
  rf2data.h
  pcars2data.h
  ams2udpdata.h
  scs2data.h
)


add_library(include STATIC ${include_source_files})
