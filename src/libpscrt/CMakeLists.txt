project(libpscrt)


set(pscrt_sources
  malloc_opt.c
      )


set(pscrt_sources_x64
  memset_em64t.s
  memset_opteron.s
  memcpy_em64t.c
  memcpy_opteron.c
  malloc_opt.c
  )


# TODO: Enable ASM language.  For now we just tell CMake to run the C
# compiler (gcc) and add some flags.
set_source_files_properties(
  memset_em64t.s
  memset_opteron.s
  PROPERTIES
  LANGUAGE C
  COMPILE_FLAGS "-x assembler-with-cpp"
  )



path64_set_multitarget_sources(pscrt-static COMMON ${pscrt_sources})
path64_set_multitarget_sources(pscrt-static "x86_64" ${pscrt_sources_x64})
path64_set_multitarget_property_(pscrt-static COMPILE_FLAGS -fpic)
path64_set_multitarget_property_(pscrt-static OUTPUT_NAME pscrt)
path64_add_multitarget_library(pscrt-static STATIC)
path64_add_dependencies_from_multitarget(compiler-stage-libs-C pscrt-static)


# Profiling library
if(PATH64_ENABLE_PROFILING)
    path64_set_multitarget_sources(pscrt_p COMMON ${pscrt_sources})
    path64_set_multitarget_sources(pscrt_p "x86_64" ${pscrt_sources_x64})
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")   # temporary workaround for bug in gcc on osol
        path64_set_multitarget_property_(pscrt_p COMPILE_FLAGS -pg)
    endif()
    path64_add_multitarget_library(pscrt_p STATIC pscrt_p_static_src_list)
endif()

#add_library(pscrt SHARED ${pscrt_sources})
#set_property(TARGET pscrt PROPERTY SOVERSION 1)
#set_property(TARGET pscrt PROPERTY LINK_FLAGS -Wl,--version-script="${libpscrt_SOURCE_DIR}/libpscrt.vs")


