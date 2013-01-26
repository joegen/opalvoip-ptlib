Using PortAudio
---------------

PTLib can be configured to use PortAudio V19 for audio input and
output. Some distros (Ubuntu, for example) only include PortAudio
V18 so it is necessary to build it manually.

PTLib can also use PortMixer (part of the Audacity project) but
not many distros provide this feature as it requires patching
PortAudio.

The script below will extract latest PortAudio and PortMixer
from the correct source repositories, build and install it
into /opt/portaudio. Once this is done, configure ptlib as
follows:

   ./configure --with-portaudio-dir=/opt/portaudio

This script has been verified on OSX Mountain Lion and
Ubuntu 12.01 for x64

    Craig (craigs@postincrement.com)

    17 January, 2013

--------------------------------------------------

#!/bin/sh

PREFIX=/opt/portaudio

svn co https://subversion.assembla.com/svn/portaudio/portaudio/trunk portaudio
svn co http://audacity.googlecode.com/svn/audacity-src/trunk/lib-src/portmixer portmixer

cd portaudio
patch -p4 < ../portmixer/portaudio.patch
cd ..

cd portaudio
make clean
./configure --prefix=${PREFIX}
make
sudo make install
cd ..

cd portmixer
make clean
( export CFLAGS=-fPIC ; ./configure --with-pa-include=../portaudio/include/ --prefix=${PREFIX} )
make
sudo cp libportmixer.a ${PREFIX}/lib
sudo cp include/portmixer.h ${PREFIX}/include
cd ..
