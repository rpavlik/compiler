#include_next <sys/stat.h>

#define S_IXGRP     _S_IXUSR
#define S_IXOTH     _S_IXUSR

#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#define lstat stat
