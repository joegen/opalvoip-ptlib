# Microsoft Developer Studio Project File - Name="Console Components" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Console Components - Win32 SSL Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Console Components.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Console Components.mak" CFG="Console Components - Win32 SSL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Console Components - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Console Components - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Console Components - Win32 SSL Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Console Components - Win32 SSL Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Console Components - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /I "$(EXPATDIR)/lib" /D "NDEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /D P_EXPAT=0$(EXPATFLAG) /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclib.lib"

!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /I "$(EXPATDIR)/lib" /D "_DEBUG" /D "PTRACING" /D P_SSL=0$(OPENSSLFLAG) /D P_EXPAT=0$(EXPATFLAG) /FR /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclibd.lib"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Console_Components___Win32_SSL_Debug"
# PROP BASE Intermediate_Dir "Console_Components___Win32_SSL_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Lib"
# PROP Intermediate_Dir "..\..\..\Lib\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR /Yu"ptlib.h" /FD /c
# ADD CPP /nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /I "$(EXPATDIR)/lib" /D "_DEBUG" /D "PTRACING" /D P_SSL=1 /D P_EXPAT=0$(EXPATFLAG) /FR /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\ptclibd.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclibd.lib"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Console_Components___Win32_SSL_Release"
# PROP BASE Intermediate_Dir "Console_Components___Win32_SSL_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Lib"
# PROP Intermediate_Dir "..\..\..\Lib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Yu"ptlib.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /I "$(EXPATDIR)/lib" /D "NDEBUG" /D "PTRACING" /D P_SSL=1 /D P_EXPAT=0$(EXPATFLAG) /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\ptclib.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclib.lib"

!ENDIF 

# Begin Target

# Name "Console Components - Win32 Release"
# Name "Console Components - Win32 Debug"
# Name "Console Components - Win32 SSL Debug"
# Name "Console Components - Win32 SSL Release"
# Begin Group "Source Files"

# PROP Default_Filter "*.cxx"
# Begin Source File

SOURCE=..\..\Ptclib\Asner.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"

!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Debug"

# ADD BASE CPP /Yu"ptlib.h"
# ADD CPP /Yu"ptlib.h"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Cypher.cxx
# ADD CPP /Yc"ptlib.h"
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\delaychan.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\dtmf.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Ftp.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Ftpclnt.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Ftpsrvr.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Html.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Http.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Httpclnt.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Httpform.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Httpsrvr.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Httpsvc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Inetmail.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Inetprot.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\ipacl.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\modem.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Pasn.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Psnmp.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\pssl.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"

!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Debug"

!ELSEIF  "$(CFG)" == "Console Components - Win32 SSL Release"

# PROP Intermediate_Dir "..\..\..\Lib\Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ptclib\pwavfile.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\pxml.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\pxmlrpc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\qchannel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\random.cxx
# End Source File
# Begin Source File

SOURCE=..\..\ptclib\shttpsvc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Snmpclnt.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Snmpserv.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\socks.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Telnet.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\asner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\cypher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\delaychan.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\dtmf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\ftp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\html.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\http.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\httpform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\httpsvc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\inetmail.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\inetprot.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\ipacl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\mime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\modem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\pasn.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\psnmp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\pssl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\pwavfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\pxml.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\pxmlrpc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\qchannel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\random.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ptclib\shttpsvc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\socks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\telnet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\url.h
# End Source File
# End Group
# End Target
# End Project
