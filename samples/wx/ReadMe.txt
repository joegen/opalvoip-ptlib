Building PTLib/wxWidgets applications
=====================================

Last updated, 1 May 2011


Application can use both PTLib and wxWidgets. So, as well as all the usual
PTLib installation, you also have to get wxWidgets installed and compiled
before you can get this sample built.

You can get wxWidgets from http://www.wxwidgets.org


Before we start, the assumption is (since you are reading this) that you
already have installed and built PTLib.


For Windows:
------------
  1.  Download the installer for wxWidgets, eg wxMSW-2.8.9-Setup.exe, or later.

  2.  Run the installer.

  3.  Set the environment variable WXDIR to the directory that you installed
      wxWidgets into in step 2. Also set WXVER to 28 or 29 depending on the
      version of wxWidgets you installed. Be sure to restart DevStudio.

  4.  Open the DevStudio workspace %WXDIR%\build\msw\wx.dsw. You are likely to
      be asked whether to convert project files. Let it do the conversion
      and use the converted projects. Note: later versions may already have
      converted projects so look for a .sln file too.

  5.  Build the "Unicode Release" and "Unicode Debug" versions of everything.
      The safest way is to simply select "Unicode Debug" in the tool bar drop
      down list, go "Build Solution", then do the same for "Unicode Release".

  6.  Open the DevStudio workspace %WXDIR%\utils\wxrc\wxrc.dsw, if missing then
      %WXDIR%\utils\wxrc\wxrc.dsp will do. Let it convert the projects as before.

  7.  Build the "Unicode Release" version. Note: when using VC Express 2005, you may get
      a large number of undefined symbols when linking. To fix this, add the
      following libraries to the linker command line:

                 user32.lib ole32.lib advapi32.lib shell32.lib

  8.  Copy the %WXDIR%\utils\wxrc\vc_mswu\wxrc.exe file to %WXDIR%\bin\wxrc.exe.
      You may need to create the %WXDIR%\bin directory.

  9.  wxWidgets is now ready to use.

You should now be able to open wx_XXXX.sln, where XXXX depends on the version
of Visual Studio you are using, and build the example.



For Linux:
----------
  1.  Download the tar file, eg wxGTK-2.8.9.tar.gz, or later.

  2.  Unpack it somewhere, you don't need to be root (yet)

  3.  cd wxGTK-2.8.*

  4.  Build wxWidgets. You can read the instructions in README-GTX.txt,
      or, if you are too lazy and feel lucky, follow the salient bits
      below (taken fromREADME-GTX.txt) which worked for me:

  5.  mkdir build_gtk

  6.  cd build_gtk

  7.  ../configure --with-gtk=2

  8.  make

  9.  "su" and enter root password>

  11. make install

  12. ldconfig

  13. wxWidgets is now ready to use.

You should now be able to go to $PTLIBDIRDIR/samples/wx and do "make".


For Mac OS-X
------------

  1.  Use "mac ports" to install wxWidgets.

  2.  Build PTLib/OPAL as per OPAL VoIP web site, with one additional caveat.
      If you are using 64 bit Snow Leopard, you must configure with the
      --enable-force32 option as wxWidgets only supports 32 bit OS-X at the
      time of writing.

You should now be able to go to $PTLIBDIR/samples/wx and do "make".


Application Icons
-----------------

Even though the XPM format is used for any internal icons used by wxWidgets,
the application itself will need platform specific formats. Here are some
utilities needed to convert between the various icon formats.

It is recommended to use ImageMagick, there are Linux (yum/apt-get),
Windows (http://www.imagemagick.org/) and Mac (via "mac ports") versions
and the commands are as simple as:

  convert app.ico app.xpm
  convert app.ico app.icns

etc.

There is just one trick, when converting to XPM you will have to convert the
declaration line from:

  static char *app__[] = 

to

  static char const * const AppIcon_xpm[] = 


===========================================================================
