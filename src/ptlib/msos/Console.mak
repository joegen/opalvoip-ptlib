# Microsoft Developer Studio Generated NMAKE File, Based on Console.dsp
!IF "$(CFG)" == ""
CFG=Console - Win32 Release
!MESSAGE No configuration specified. Defaulting to Console - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "Console - Win32 Release" && "$(CFG)" != "Console - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Console - Win32 Release"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Release
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\ptlibs.lib"


CLEAN :
	-@erase "$(INTDIR)\assert.obj"
	-@erase "$(INTDIR)\collect.obj"
	-@erase "$(INTDIR)\Console.pch"
	-@erase "$(INTDIR)\contain.obj"
	-@erase "$(INTDIR)\ethsock.obj"
	-@erase "$(INTDIR)\getdate_tab.obj"
	-@erase "$(INTDIR)\icmp.obj"
	-@erase "$(INTDIR)\mail.obj"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\osutils.obj"
	-@erase "$(INTDIR)\pchannel.obj"
	-@erase "$(INTDIR)\pconfig.obj"
	-@erase "$(INTDIR)\pethsock.obj"
	-@erase "$(INTDIR)\pipe.obj"
	-@erase "$(INTDIR)\pipechan.obj"
	-@erase "$(INTDIR)\ptime.obj"
	-@erase "$(INTDIR)\ptlib.obj"
	-@erase "$(INTDIR)\regex.obj"
	-@erase "$(INTDIR)\remconn.obj"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\sfile.obj"
	-@erase "$(INTDIR)\sockets.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\svcproc.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\wincfg.obj"
	-@erase "$(INTDIR)\winserial.obj"
	-@erase "$(INTDIR)\winsock.obj"
	-@erase "$(OUTDIR)\ptlibs.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"Lib/PTLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\ptlibs.lib" 
LIB32_OBJS= \
	"$(INTDIR)\assert.obj" \
	"$(INTDIR)\collect.obj" \
	"$(INTDIR)\contain.obj" \
	"$(INTDIR)\ethsock.obj" \
	"$(INTDIR)\getdate_tab.obj" \
	"$(INTDIR)\icmp.obj" \
	"$(INTDIR)\mail.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\osutils.obj" \
	"$(INTDIR)\pchannel.obj" \
	"$(INTDIR)\pconfig.obj" \
	"$(INTDIR)\pethsock.obj" \
	"$(INTDIR)\pipe.obj" \
	"$(INTDIR)\pipechan.obj" \
	"$(INTDIR)\ptime.obj" \
	"$(INTDIR)\ptlib.obj" \
	"$(INTDIR)\regex.obj" \
	"$(INTDIR)\remconn.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\sfile.obj" \
	"$(INTDIR)\sockets.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\svcproc.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\wincfg.obj" \
	"$(INTDIR)\winserial.obj" \
	"$(INTDIR)\winsock.obj"

"$(OUTDIR)\ptlibs.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Debug
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\ptlibsd.lib" "$(OUTDIR)\PTLib.bsc"


CLEAN :
	-@erase "$(INTDIR)\assert.obj"
	-@erase "$(INTDIR)\assert.sbr"
	-@erase "$(INTDIR)\collect.obj"
	-@erase "$(INTDIR)\collect.sbr"
	-@erase "$(INTDIR)\Console.pch"
	-@erase "$(INTDIR)\contain.obj"
	-@erase "$(INTDIR)\contain.sbr"
	-@erase "$(INTDIR)\ethsock.obj"
	-@erase "$(INTDIR)\ethsock.sbr"
	-@erase "$(INTDIR)\getdate_tab.obj"
	-@erase "$(INTDIR)\getdate_tab.sbr"
	-@erase "$(INTDIR)\icmp.obj"
	-@erase "$(INTDIR)\icmp.sbr"
	-@erase "$(INTDIR)\mail.obj"
	-@erase "$(INTDIR)\mail.sbr"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\object.sbr"
	-@erase "$(INTDIR)\osutils.obj"
	-@erase "$(INTDIR)\osutils.sbr"
	-@erase "$(INTDIR)\pchannel.obj"
	-@erase "$(INTDIR)\pchannel.sbr"
	-@erase "$(INTDIR)\pconfig.obj"
	-@erase "$(INTDIR)\pconfig.sbr"
	-@erase "$(INTDIR)\pethsock.obj"
	-@erase "$(INTDIR)\pethsock.sbr"
	-@erase "$(INTDIR)\pipe.obj"
	-@erase "$(INTDIR)\pipe.sbr"
	-@erase "$(INTDIR)\pipechan.obj"
	-@erase "$(INTDIR)\pipechan.sbr"
	-@erase "$(INTDIR)\ptime.obj"
	-@erase "$(INTDIR)\ptime.sbr"
	-@erase "$(INTDIR)\ptlib.obj"
	-@erase "$(INTDIR)\ptlib.sbr"
	-@erase "$(INTDIR)\regex.obj"
	-@erase "$(INTDIR)\regex.sbr"
	-@erase "$(INTDIR)\remconn.obj"
	-@erase "$(INTDIR)\remconn.sbr"
	-@erase "$(INTDIR)\serial.obj"
	-@erase "$(INTDIR)\serial.sbr"
	-@erase "$(INTDIR)\sfile.obj"
	-@erase "$(INTDIR)\sfile.sbr"
	-@erase "$(INTDIR)\sockets.obj"
	-@erase "$(INTDIR)\sockets.sbr"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound.sbr"
	-@erase "$(INTDIR)\svcproc.obj"
	-@erase "$(INTDIR)\svcproc.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(INTDIR)\win32.sbr"
	-@erase "$(INTDIR)\wincfg.obj"
	-@erase "$(INTDIR)\wincfg.sbr"
	-@erase "$(INTDIR)\winserial.obj"
	-@erase "$(INTDIR)\winserial.sbr"
	-@erase "$(INTDIR)\winsock.obj"
	-@erase "$(INTDIR)\winsock.sbr"
	-@erase "$(OUTDIR)\PTLib.bsc"
	-@erase "$(OUTDIR)\ptlibsd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PTLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\assert.sbr" \
	"$(INTDIR)\collect.sbr" \
	"$(INTDIR)\contain.sbr" \
	"$(INTDIR)\ethsock.sbr" \
	"$(INTDIR)\getdate_tab.sbr" \
	"$(INTDIR)\icmp.sbr" \
	"$(INTDIR)\mail.sbr" \
	"$(INTDIR)\object.sbr" \
	"$(INTDIR)\osutils.sbr" \
	"$(INTDIR)\pchannel.sbr" \
	"$(INTDIR)\pconfig.sbr" \
	"$(INTDIR)\pethsock.sbr" \
	"$(INTDIR)\pipe.sbr" \
	"$(INTDIR)\pipechan.sbr" \
	"$(INTDIR)\ptime.sbr" \
	"$(INTDIR)\ptlib.sbr" \
	"$(INTDIR)\regex.sbr" \
	"$(INTDIR)\remconn.sbr" \
	"$(INTDIR)\serial.sbr" \
	"$(INTDIR)\sfile.sbr" \
	"$(INTDIR)\sockets.sbr" \
	"$(INTDIR)\sound.sbr" \
	"$(INTDIR)\svcproc.sbr" \
	"$(INTDIR)\win32.sbr" \
	"$(INTDIR)\wincfg.sbr" \
	"$(INTDIR)\winserial.sbr" \
	"$(INTDIR)\winsock.sbr"

"$(OUTDIR)\PTLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\ptlibsd.lib" 
LIB32_OBJS= \
	"$(INTDIR)\assert.obj" \
	"$(INTDIR)\collect.obj" \
	"$(INTDIR)\contain.obj" \
	"$(INTDIR)\ethsock.obj" \
	"$(INTDIR)\getdate_tab.obj" \
	"$(INTDIR)\icmp.obj" \
	"$(INTDIR)\mail.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\osutils.obj" \
	"$(INTDIR)\pchannel.obj" \
	"$(INTDIR)\pconfig.obj" \
	"$(INTDIR)\pethsock.obj" \
	"$(INTDIR)\pipe.obj" \
	"$(INTDIR)\pipechan.obj" \
	"$(INTDIR)\ptime.obj" \
	"$(INTDIR)\ptlib.obj" \
	"$(INTDIR)\regex.obj" \
	"$(INTDIR)\remconn.obj" \
	"$(INTDIR)\serial.obj" \
	"$(INTDIR)\sfile.obj" \
	"$(INTDIR)\sockets.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\svcproc.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\wincfg.obj" \
	"$(INTDIR)\winserial.obj" \
	"$(INTDIR)\winsock.obj"

"$(OUTDIR)\ptlibsd.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Console.dep")
!INCLUDE "Console.dep"
!ELSE 
!MESSAGE Warning: cannot find "Console.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Console - Win32 Release" || "$(CFG)" == "Console - Win32 Debug"
SOURCE=.\assert.cxx

!IF  "$(CFG)" == "Console - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\assert.obj"	"$(INTDIR)\Console.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\assert.obj"	"$(INTDIR)\assert.sbr"	"$(INTDIR)\Console.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\common\collect.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\collect.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\collect.obj"	"$(INTDIR)\collect.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\contain.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\contain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\contain.obj"	"$(INTDIR)\contain.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ethsock.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\ethsock.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\ethsock.obj"	"$(INTDIR)\ethsock.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=..\common\getdate.y

!IF  "$(CFG)" == "Console - Win32 Release"

InputPath=..\common\getdate.y

"..\common\getdate_tab.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison ../common/getdate.y
<< 
	

!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

InputPath=..\common\getdate.y

"..\common\getdate_tab.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison ../common/getdate.y
<< 
	

!ENDIF 

SOURCE=..\common\getdate_tab.c

!IF  "$(CFG)" == "Console - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob0 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\getdate_tab.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\getdate_tab.obj"	"$(INTDIR)\getdate_tab.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\icmp.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\icmp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\icmp.obj"	"$(INTDIR)\icmp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\mail.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\mail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\mail.obj"	"$(INTDIR)\mail.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=..\common\object.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\object.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\object.obj"	"$(INTDIR)\object.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\osutils.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\osutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\osutils.obj"	"$(INTDIR)\osutils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\Common\pchannel.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\pchannel.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\pchannel.obj"	"$(INTDIR)\pchannel.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\Common\pconfig.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\pconfig.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\pconfig.obj"	"$(INTDIR)\pconfig.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\pethsock.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\pethsock.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\pethsock.obj"	"$(INTDIR)\pethsock.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\pipe.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\pipe.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\pipe.obj"	"$(INTDIR)\pipe.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=..\common\pipechan.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\pipechan.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\pipechan.obj"	"$(INTDIR)\pipechan.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\ptime.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\ptime.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\ptime.obj"	"$(INTDIR)\ptime.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ptlib.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\ptlib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\ptlib.obj"	"$(INTDIR)\ptlib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=..\common\regex.cxx

!IF  "$(CFG)" == "Console - Win32 Release"

CPP_SWITCHES=/nologo /MD /W2 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /D "__STDC__" /D "STDC_HEADERS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\regex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W2 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /D "__STDC__" /D "STDC_HEADERS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\regex.obj"	"$(INTDIR)\regex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\remconn.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\remconn.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\remconn.obj"	"$(INTDIR)\remconn.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=..\Common\serial.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\serial.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\serial.obj"	"$(INTDIR)\serial.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\sfile.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\sfile.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\sfile.obj"	"$(INTDIR)\sfile.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\common\sockets.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\sockets.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\sockets.obj"	"$(INTDIR)\sockets.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\sound.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\sound.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\sound.obj"	"$(INTDIR)\sound.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\svcproc.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\svcproc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\svcproc.obj"	"$(INTDIR)\svcproc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\win32.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\win32.obj"	"$(INTDIR)\win32.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\wincfg.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\wincfg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\wincfg.obj"	"$(INTDIR)\wincfg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\winserial.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\winserial.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\winserial.obj"	"$(INTDIR)\winserial.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 

SOURCE=.\winsock.cxx

!IF  "$(CFG)" == "Console - Win32 Release"


"$(INTDIR)\winsock.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ELSEIF  "$(CFG)" == "Console - Win32 Debug"


"$(INTDIR)\winsock.obj"	"$(INTDIR)\winsock.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console.pch"


!ENDIF 


!ENDIF 

