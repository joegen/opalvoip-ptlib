# Microsoft Developer Studio Project File - Name="Console" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Console - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Console.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Console.mak" CFG="Console - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Console - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Console - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "Console - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Lib"
# PROP Intermediate_Dir "..\..\..\Lib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\pwlib\mswin" /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /Yu"ptlib.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Lib/PTLib.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptlibs.lib"

!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Lib"
# PROP Intermediate_Dir "..\..\..\Lib\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /GX /Zi /Od /Gf /Gy /I "..\..\..\include\pwlib\mswin" /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /FR /Yu"ptlib.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Lib/PTLib.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptlibsd.lib"

!ENDIF 

# Begin Target

# Name "Console - Win32 Release"
# Name "Console - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\assert.cxx
# ADD CPP /Yc"ptlib.h"
# End Source File
# Begin Source File

SOURCE=..\common\collect.cxx
# End Source File
# Begin Source File

SOURCE=..\common\contain.cxx
# End Source File
# Begin Source File

SOURCE=.\ethsock.cxx
# End Source File
# Begin Source File

SOURCE=.\icmp.cxx
# End Source File
# Begin Source File

SOURCE=.\mail.cxx
# End Source File
# Begin Source File

SOURCE=..\common\object.cxx
# End Source File
# Begin Source File

SOURCE=..\common\osutils.cxx
# End Source File
# Begin Source File

SOURCE=..\Common\pchannel.cxx
# End Source File
# Begin Source File

SOURCE=..\Common\pconfig.cxx
# End Source File
# Begin Source File

SOURCE=..\common\pethsock.cxx
# End Source File
# Begin Source File

SOURCE=.\pipe.cxx
# End Source File
# Begin Source File

SOURCE=..\common\pipechan.cxx
# End Source File
# Begin Source File

SOURCE=..\common\ptime.cxx
# End Source File
# Begin Source File

SOURCE=.\ptlib.cxx
# End Source File
# Begin Source File

SOURCE=..\common\regex.cxx
# ADD CPP /W2 /D "__STDC__" /D "STDC_HEADERS"
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\remconn.cxx
# End Source File
# Begin Source File

SOURCE=..\Common\serial.cxx
# End Source File
# Begin Source File

SOURCE=..\common\sfile.cxx
# End Source File
# Begin Source File

SOURCE=..\common\sockets.cxx
# End Source File
# Begin Source File

SOURCE=.\svcproc.cxx
# End Source File
# Begin Source File

SOURCE=.\win32.cxx
# End Source File
# Begin Source File

SOURCE=.\wincfg.cxx
# End Source File
# Begin Source File

SOURCE=.\winserial.cxx
# End Source File
# Begin Source File

SOURCE=.\winsock.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Args.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Channel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Contain.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Dict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Dynalink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Ethsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\File.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\filepath.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Icmpsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Indchan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Ipdsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\ipsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\ipxsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Lists.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Mail.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Pdirect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Pipechan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\pprocess.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Pstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Ptime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Remconn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\semaphor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Serchan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Sfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\socket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\sockets.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Sound.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\spxsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Svcproc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\syncpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\syncthrd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Tcpsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Textfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Timeint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Timer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtLib\Udpsock.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
