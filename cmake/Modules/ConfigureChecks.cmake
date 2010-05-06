include(CheckIncludeFile)
include(CheckIncludeFiles)
check_include_files(alloca.h    HAVE_ALLOCA_H)
check_include_files(dlfcn.h     HAVE_DLFCN_H)
check_include_files(inttypes.h  HAVE_INTTYPES_H)
check_include_files(limits.h    HAVE_LIMITS_H)
check_include_files(memory.h    HAVE_MEMORY_H)
check_include_files(stdint.h    HAVE_STDINT_H)
check_include_files(stdlib.h    HAVE_STDLIB_H)
check_include_files(string.h    HAVE_STRING_H)
check_include_files(strings.h   HAVE_STRINGS_H)
check_include_files(sys/stat.h  HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h    HAVE_UNISTD_H)

include(CheckFunctionExists)
check_function_exists(bcopy        HAVE_BCOPY)
check_function_exists(fseek64      HAVE_FSEEK64)
check_function_exists(fseeko       HAVE_FSEEKO)
check_function_exists(ftell64      HAVE_FTELL64)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(mkstemp      HAVE_MKSTEMP)
check_function_exists(mkstemps     HAVE_MKSTEMPS)
check_function_exists(popen        HAVE_POPEN)
check_function_exists(psignal      HAVE_PSIGNAL)
check_function_exists(strsignal    HAVE_STRSIGNAL)
check_function_exists(glob	   HAVE_GLOB)
