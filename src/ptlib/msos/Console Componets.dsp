# Microsoft Developer Studio Project File - Name="Console Componets" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Console Componets - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Console Componets.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Console Componets.mak" CFG="Console Componets - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Console Componets - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "Console Componets - Win32 Debug" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "Console Componets - Win32 Release"

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
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclib.lib"

!ELSEIF  "$(CFG)" == "Console Componets - Win32 Debug"

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
# ADD CPP /nologo /MDd /W4 /GX /Zi /Od /Gf /Gy /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /FR /Yu"ptlib.h" /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ptclibd.lib"

!ENDIF 

# Begin Target

# Name "Console Componets - Win32 Release"
# Name "Console Componets - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cxx"
# Begin Source File

SOURCE=..\..\Ptclib\Asner.cxx

!IF  "$(CFG)" == "Console Componets - Win32 Release"

!ELSEIF  "$(CFG)" == "Console Componets - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\Cypher.cxx

!IF  "$(CFG)" == "Console Componets - Win32 Release"

!ELSEIF  "$(CFG)" == "Console Componets - Win32 Debug"

# ADD CPP /Yu"ptlib.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\modem.cxx
# End Source File
# Begin Source File

SOURCE=..\..\Ptclib\socks.cxx
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

SOURCE=..\..\..\Include\PtCLib\modem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Include\PtCLib\socks.h
# End Source File
# End Group
# End Target
# End Project
