#ifndef edges_version_HEADER
#define edges_version_HEADER
dnl

#define edges_VERSION 0
#define edges_MINOR 0
#define edges_PATCHLEVEL 0

regexp(VERSION, `^v\([0-9]+\)\([^.]\)?\.\([0-9]+\)\([^.]\)?\.\([0-9]+\)[^-.]*\(-\([0-9]+\)-g\([0-9a-fA-F]+\)\)?$', ``
#undef edges_VERSION
#undef edges_MINOR
#undef edges_PATCHLEVEL
#define edges_VERSION \1
#define edges_MINOR \3
#define edges_PATCHLEVEL \5
'ifelse(`\7',,,``#define edges_AHEAD \7
'')`'ifelse(`\8',,,``#define edges_COMMIT \8
'')`'')

#endif
