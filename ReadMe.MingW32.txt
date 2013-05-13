The following script can be used to compile PTLib (and Opal)
using MingW.

Uncomment the appropriate line for 64 or 32 bit builds

-------------------------------------------------------
#!/bin/sh

export HOST_PREFIX=x86_64-w64-mingw32
#export HOST_PREFIX=i586-mingw32msvc

export PKG_CONFIG_LIBDIR=/usr/local/${HOST_PREFIX}/ 
./configure --host=${HOST_PREFIX} 
