# Microsoft Developer Studio Project File - Name="Protocols" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Protocols - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Protocols.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Protocols.mak" CFG="Protocols - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Protocols - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Protocols - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Protocols - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Release"
# PROP Intermediate_Dir "..\..\..\Lib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\Release\proto.lib"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

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
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /FR /Yu"ptlib.h" /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\Debug\proto.lib"

!ENDIF 

# Begin Target

# Name "Protocols - Win32 Release"
# Name "Protocols - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cxx"
# Begin Source File

SOURCE=..\..\Ptclib\proto\Ftp.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Ftpclnt.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Ftpsrvr.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Html.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Http.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Httpclnt.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Httpform.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Httpsrvr.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Httpsvc.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Inetmail.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Inetprot.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Pasn.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Psnmp.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Pssl.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Snmpclnt.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Snmpserv.cxx

!IF  "$(CFG)" == "Protocols - Win32 Release"

!ELSEIF  "$(CFG)" == "Protocols - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\proto\Telnet.cxx
# ADD CPP /Yc"ptlib.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
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

SOURCE=..\..\..\Include\PtCLib\mime.h
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

SOURCE=..\..\..\Include\PtCLib\telnet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\url.h
# End Source File
# End Group
# End Target
# End Project
