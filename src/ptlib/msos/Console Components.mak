# Microsoft Developer Studio Generated NMAKE File, Based on Console Components.dsp
!IF "$(CFG)" == ""
CFG=Console Components - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Console Components - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Console Components - Win32 Release" && "$(CFG)" != "Console Components - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Console Components.mak" CFG="Console Components - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Console Components - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Console Components - Win32 Debug" (based on "Win32 (x86) Static Library")
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

!IF  "$(CFG)" == "Console Components - Win32 Release"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Release
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\ptclib.lib"


CLEAN :
	-@erase "$(INTDIR)\Asner.obj"
	-@erase "$(INTDIR)\Console Components.pch"
	-@erase "$(INTDIR)\Cypher.obj"
	-@erase "$(INTDIR)\Ftp.obj"
	-@erase "$(INTDIR)\Ftpclnt.obj"
	-@erase "$(INTDIR)\Ftpsrvr.obj"
	-@erase "$(INTDIR)\Html.obj"
	-@erase "$(INTDIR)\Http.obj"
	-@erase "$(INTDIR)\Httpclnt.obj"
	-@erase "$(INTDIR)\Httpform.obj"
	-@erase "$(INTDIR)\Httpsrvr.obj"
	-@erase "$(INTDIR)\Httpsvc.obj"
	-@erase "$(INTDIR)\Inetmail.obj"
	-@erase "$(INTDIR)\Inetprot.obj"
	-@erase "$(INTDIR)\ipacl.obj"
	-@erase "$(INTDIR)\modem.obj"
	-@erase "$(INTDIR)\Pasn.obj"
	-@erase "$(INTDIR)\Psnmp.obj"
	-@erase "$(INTDIR)\Pssl.obj"
	-@erase "$(INTDIR)\Snmpclnt.obj"
	-@erase "$(INTDIR)\Snmpserv.obj"
	-@erase "$(INTDIR)\socks.obj"
	-@erase "$(INTDIR)\Telnet.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ptclib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Console Components.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\ptclib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Asner.obj" \
	"$(INTDIR)\Cypher.obj" \
	"$(INTDIR)\Ftp.obj" \
	"$(INTDIR)\Ftpclnt.obj" \
	"$(INTDIR)\Ftpsrvr.obj" \
	"$(INTDIR)\Html.obj" \
	"$(INTDIR)\Http.obj" \
	"$(INTDIR)\Httpclnt.obj" \
	"$(INTDIR)\Httpform.obj" \
	"$(INTDIR)\Httpsrvr.obj" \
	"$(INTDIR)\Httpsvc.obj" \
	"$(INTDIR)\Inetmail.obj" \
	"$(INTDIR)\Inetprot.obj" \
	"$(INTDIR)\ipacl.obj" \
	"$(INTDIR)\modem.obj" \
	"$(INTDIR)\Pasn.obj" \
	"$(INTDIR)\Psnmp.obj" \
	"$(INTDIR)\Pssl.obj" \
	"$(INTDIR)\Snmpclnt.obj" \
	"$(INTDIR)\Snmpserv.obj" \
	"$(INTDIR)\socks.obj" \
	"$(INTDIR)\Telnet.obj"

"$(OUTDIR)\ptclib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Debug
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\ptclibd.lib" "$(OUTDIR)\Console Components.bsc"


CLEAN :
	-@erase "$(INTDIR)\Asner.obj"
	-@erase "$(INTDIR)\Asner.sbr"
	-@erase "$(INTDIR)\Console Components.pch"
	-@erase "$(INTDIR)\Cypher.obj"
	-@erase "$(INTDIR)\Cypher.sbr"
	-@erase "$(INTDIR)\Ftp.obj"
	-@erase "$(INTDIR)\Ftp.sbr"
	-@erase "$(INTDIR)\Ftpclnt.obj"
	-@erase "$(INTDIR)\Ftpclnt.sbr"
	-@erase "$(INTDIR)\Ftpsrvr.obj"
	-@erase "$(INTDIR)\Ftpsrvr.sbr"
	-@erase "$(INTDIR)\Html.obj"
	-@erase "$(INTDIR)\Html.sbr"
	-@erase "$(INTDIR)\Http.obj"
	-@erase "$(INTDIR)\Http.sbr"
	-@erase "$(INTDIR)\Httpclnt.obj"
	-@erase "$(INTDIR)\Httpclnt.sbr"
	-@erase "$(INTDIR)\Httpform.obj"
	-@erase "$(INTDIR)\Httpform.sbr"
	-@erase "$(INTDIR)\Httpsrvr.obj"
	-@erase "$(INTDIR)\Httpsrvr.sbr"
	-@erase "$(INTDIR)\Httpsvc.obj"
	-@erase "$(INTDIR)\Httpsvc.sbr"
	-@erase "$(INTDIR)\Inetmail.obj"
	-@erase "$(INTDIR)\Inetmail.sbr"
	-@erase "$(INTDIR)\Inetprot.obj"
	-@erase "$(INTDIR)\Inetprot.sbr"
	-@erase "$(INTDIR)\ipacl.obj"
	-@erase "$(INTDIR)\ipacl.sbr"
	-@erase "$(INTDIR)\modem.obj"
	-@erase "$(INTDIR)\modem.sbr"
	-@erase "$(INTDIR)\Pasn.obj"
	-@erase "$(INTDIR)\Pasn.sbr"
	-@erase "$(INTDIR)\Psnmp.obj"
	-@erase "$(INTDIR)\Psnmp.sbr"
	-@erase "$(INTDIR)\Pssl.obj"
	-@erase "$(INTDIR)\Pssl.sbr"
	-@erase "$(INTDIR)\Snmpclnt.obj"
	-@erase "$(INTDIR)\Snmpclnt.sbr"
	-@erase "$(INTDIR)\Snmpserv.obj"
	-@erase "$(INTDIR)\Snmpserv.sbr"
	-@erase "$(INTDIR)\socks.obj"
	-@erase "$(INTDIR)\socks.sbr"
	-@erase "$(INTDIR)\Telnet.obj"
	-@erase "$(INTDIR)\Telnet.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Console Components.bsc"
	-@erase "$(OUTDIR)\ptclibd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Console Components.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Asner.sbr" \
	"$(INTDIR)\Cypher.sbr" \
	"$(INTDIR)\Ftp.sbr" \
	"$(INTDIR)\Ftpclnt.sbr" \
	"$(INTDIR)\Ftpsrvr.sbr" \
	"$(INTDIR)\Html.sbr" \
	"$(INTDIR)\Http.sbr" \
	"$(INTDIR)\Httpclnt.sbr" \
	"$(INTDIR)\Httpform.sbr" \
	"$(INTDIR)\Httpsrvr.sbr" \
	"$(INTDIR)\Httpsvc.sbr" \
	"$(INTDIR)\Inetmail.sbr" \
	"$(INTDIR)\Inetprot.sbr" \
	"$(INTDIR)\ipacl.sbr" \
	"$(INTDIR)\modem.sbr" \
	"$(INTDIR)\Pasn.sbr" \
	"$(INTDIR)\Psnmp.sbr" \
	"$(INTDIR)\Pssl.sbr" \
	"$(INTDIR)\Snmpclnt.sbr" \
	"$(INTDIR)\Snmpserv.sbr" \
	"$(INTDIR)\socks.sbr" \
	"$(INTDIR)\Telnet.sbr"

"$(OUTDIR)\Console Components.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\ptclibd.lib" 
LIB32_OBJS= \
	"$(INTDIR)\Asner.obj" \
	"$(INTDIR)\Cypher.obj" \
	"$(INTDIR)\Ftp.obj" \
	"$(INTDIR)\Ftpclnt.obj" \
	"$(INTDIR)\Ftpsrvr.obj" \
	"$(INTDIR)\Html.obj" \
	"$(INTDIR)\Http.obj" \
	"$(INTDIR)\Httpclnt.obj" \
	"$(INTDIR)\Httpform.obj" \
	"$(INTDIR)\Httpsrvr.obj" \
	"$(INTDIR)\Httpsvc.obj" \
	"$(INTDIR)\Inetmail.obj" \
	"$(INTDIR)\Inetprot.obj" \
	"$(INTDIR)\ipacl.obj" \
	"$(INTDIR)\modem.obj" \
	"$(INTDIR)\Pasn.obj" \
	"$(INTDIR)\Psnmp.obj" \
	"$(INTDIR)\Pssl.obj" \
	"$(INTDIR)\Snmpclnt.obj" \
	"$(INTDIR)\Snmpserv.obj" \
	"$(INTDIR)\socks.obj" \
	"$(INTDIR)\Telnet.obj"

"$(OUTDIR)\ptclibd.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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
!IF EXISTS("Console Components.dep")
!INCLUDE "Console Components.dep"
!ELSE 
!MESSAGE Warning: cannot find "Console Components.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Console Components - Win32 Release" || "$(CFG)" == "Console Components - Win32 Debug"
SOURCE=..\..\Ptclib\Asner.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Asner.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Asner.obj"	"$(INTDIR)\Asner.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\Ptclib\Cypher.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console Components.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Cypher.obj"	"$(INTDIR)\Console Components.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console Components.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Cypher.obj"	"$(INTDIR)\Cypher.sbr"	"$(INTDIR)\Console Components.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\Ptclib\proto\Ftp.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Ftp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Ftp.obj"	"$(INTDIR)\Ftp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Ftpclnt.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Ftpclnt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Ftpclnt.obj"	"$(INTDIR)\Ftpclnt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Ftpsrvr.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Ftpsrvr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Ftpsrvr.obj"	"$(INTDIR)\Ftpsrvr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Html.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Html.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Html.obj"	"$(INTDIR)\Html.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Http.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Http.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Http.obj"	"$(INTDIR)\Http.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Httpclnt.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Httpclnt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Httpclnt.obj"	"$(INTDIR)\Httpclnt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Httpform.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Httpform.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Httpform.obj"	"$(INTDIR)\Httpform.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Httpsrvr.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Httpsrvr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Httpsrvr.obj"	"$(INTDIR)\Httpsrvr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Httpsvc.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Httpsvc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Httpsvc.obj"	"$(INTDIR)\Httpsvc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Inetmail.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Inetmail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Inetmail.obj"	"$(INTDIR)\Inetmail.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Inetprot.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Inetprot.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Inetprot.obj"	"$(INTDIR)\Inetprot.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\ptclib\ipacl.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\ipacl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\ipacl.obj"	"$(INTDIR)\ipacl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\modem.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\modem.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\modem.obj"	"$(INTDIR)\modem.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Pasn.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Pasn.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Pasn.obj"	"$(INTDIR)\Pasn.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Psnmp.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Psnmp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Psnmp.obj"	"$(INTDIR)\Psnmp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Pssl.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Pssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Pssl.obj"	"$(INTDIR)\Pssl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Snmpclnt.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Snmpclnt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Snmpclnt.obj"	"$(INTDIR)\Snmpclnt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Snmpserv.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\Snmpserv.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\Snmpserv.obj"	"$(INTDIR)\Snmpserv.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\socks.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"


"$(INTDIR)\socks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"


"$(INTDIR)\socks.obj"	"$(INTDIR)\socks.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Ptclib\proto\Telnet.cxx

!IF  "$(CFG)" == "Console Components - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "NDEBUG" /D "PTRACING" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Telnet.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Console Components - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /D "_DEBUG" /D "PTRACING" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Console Components.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Telnet.obj"	"$(INTDIR)\Telnet.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Console Components.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

