# Microsoft Developer Studio Generated NMAKE File, Based on PTLib.dsp
!IF "$(CFG)" == ""
CFG=PTLib - Win32 SSL Debug
!MESSAGE No configuration specified. Defaulting to PTLib - Win32 SSL Debug.
!ENDIF 

!IF "$(CFG)" != "PTLib - Win32 Release" && "$(CFG)" != "PTLib - Win32 Debug" && "$(CFG)" != "PTLib - Win32 SSL Debug" && "$(CFG)" != "PTLib - Win32 SSL Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PTLib.mak" CFG="PTLib - Win32 SSL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PTLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PTLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PTLib - Win32 SSL Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PTLib - Win32 SSL Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PTLib - Win32 Release"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Release
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\PTLib.dll"


CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\ptlib.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PTLib.dll"
	-@erase "$(OUTDIR)\PTLib.exp"
	-@erase "$(OUTDIR)\PTLib.lib"
	-@erase "$(OUTDIR)\PTLib.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\PTLib.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\libver.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PTLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib vfw32.lib winmm.lib mpr.lib snmpapi.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\PTLib.pdb" /debug /debugtype:both /machine:I386 /def:"..\..\..\Lib\Release\ptlib.def" /out:"$(OUTDIR)\PTLib.dll" /implib:"$(OUTDIR)\PTLib.lib" /libpath:"..\..\..\lib" 
DEF_FILE= \
	"$(INTDIR)\ptlib.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\ptlib.res"

"$(OUTDIR)\PTLib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\..\..\..\Lib
TargetName=PTLib
SOURCE="$(InputPath)"
PostBuild_Desc=Extracting debug symbols
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PTLib.dll"
   rebase -b 0x10000000 -x . .\..\..\..\Lib\PTLib.dll
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PTLib - Win32 Debug"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Debug
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\PTLibd.dll"


CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\ptlib.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PTLibd.dll"
	-@erase "$(OUTDIR)\PTLibd.exp"
	-@erase "$(OUTDIR)\PTLibd.ilk"
	-@erase "$(OUTDIR)\PTLibd.lib"
	-@erase "$(OUTDIR)\PTLibd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PTLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib vfw32.lib winmm.lib mpr.lib snmpapi.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\PTLibd.pdb" /debug /machine:I386 /def:"..\..\..\Lib\Debug\PTLibd.def" /out:"$(OUTDIR)\PTLibd.dll" /implib:"$(OUTDIR)\PTLibd.lib" /libpath:"..\lib" 
DEF_FILE= \
	"$(INTDIR)\PTLibd.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\ptlib.res"

"$(OUTDIR)\PTLibd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Debug"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Debug
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\PTLibd.dll"


CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\ptlib.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PTLibd.dll"
	-@erase "$(OUTDIR)\PTLibd.exp"
	-@erase "$(OUTDIR)\PTLibd.ilk"
	-@erase "$(OUTDIR)\PTLibd.lib"
	-@erase "$(OUTDIR)\PTLibd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PTLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib vfw32.lib winmm.lib mpr.lib snmpapi.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\PTLibd.pdb" /debug /machine:I386 /def:"..\..\..\Lib\Debug\PTLibd.def" /out:"$(OUTDIR)\PTLibd.dll" /implib:"$(OUTDIR)\PTLibd.lib" /libpath:"..\lib" 
DEF_FILE= \
	"$(INTDIR)\PTLibd.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\ptlib.res"

"$(OUTDIR)\PTLibd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Release"

OUTDIR=.\..\..\..\Lib
INTDIR=.\..\..\..\Lib\Release
# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

ALL : "$(OUTDIR)\PTLib.dll"


CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\ptlib.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PTLib.dll"
	-@erase "$(OUTDIR)\PTLib.exp"
	-@erase "$(OUTDIR)\PTLib.lib"
	-@erase "$(OUTDIR)\PTLib.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fp"$(INTDIR)\PTLib.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\libver.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PTLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib vfw32.lib winmm.lib mpr.lib snmpapi.lib wsock32.lib netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\PTLib.pdb" /debug /debugtype:both /machine:I386 /def:"..\..\..\Lib\Release\ptlib.def" /out:"$(OUTDIR)\PTLib.dll" /implib:"$(OUTDIR)\PTLib.lib" /libpath:"..\..\..\lib" 
DEF_FILE= \
	"$(INTDIR)\ptlib.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\ptlib.res"

"$(OUTDIR)\PTLib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\..\..\..\Lib
TargetName=PTLib
SOURCE="$(InputPath)"
PostBuild_Desc=Extracting debug symbols
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\..\..\..\Lib
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PTLib.dll"
   rebase -b 0x10000000 -x . .\..\..\..\Lib\PTLib.dll
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("PTLib.dep")
!INCLUDE "PTLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "PTLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PTLib - Win32 Release" || "$(CFG)" == "PTLib - Win32 Debug" || "$(CFG)" == "PTLib - Win32 SSL Debug" || "$(CFG)" == "PTLib - Win32 SSL Release"
SOURCE=.\dllmain.cxx

!IF  "$(CFG)" == "PTLib - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "PTLib - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /Zi /Od /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /Zi /O2 /Ob2 /I "..\..\..\include\ptlib\msos" /I "..\..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libver.rc

!IF  "$(CFG)" == "PTLib - Win32 Release"


"$(INTDIR)\ptlib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "NDEBUG" /d PRODUCT=PTLib $(SOURCE)


!ELSEIF  "$(CFG)" == "PTLib - Win32 Debug"


"$(INTDIR)\ptlib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "_DEBUG" /d PRODUCT=PTLib $(SOURCE)


!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Debug"


"$(INTDIR)\ptlib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "_DEBUG" /d PRODUCT=PTLib $(SOURCE)


!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Release"


"$(INTDIR)\ptlib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc09 /fo"$(INTDIR)\ptlib.res" /d "NDEBUG" /d PRODUCT=PTLib $(SOURCE)


!ENDIF 

SOURCE=..\..\..\include\ptlib\msos\ptlib.dtf

!IF  "$(CFG)" == "PTLib - Win32 Release"

IntDir=.\..\..\..\Lib\Release
OutDir=.\..\..\..\Lib
TargetName=PTLib
InputPath=..\..\..\include\ptlib\msos\ptlib.dtf
USERDEP__PTLIB="$(OutDir)\ptlibs.lib"	

"$(INTDIR)\ptlib.def" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__PTLIB)
	<<tempfile.bat 
	@echo off 
	MergeSym -I "$(Include)$(INCLUDE)" -x ptlib.ignore $(OutDir)\ptlibs.lib $(InputPath) 
	copy $(InputPath)+nul $(IntDir)\$(TargetName).def > nul 
<< 
	

!ELSEIF  "$(CFG)" == "PTLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Debug"

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Release"

IntDir=.\..\..\..\Lib\Release
OutDir=.\..\..\..\Lib
TargetName=PTLib
InputPath=..\..\..\include\ptlib\msos\ptlib.dtf
USERDEP__PTLIB="$(OutDir)\ptlibs.lib"	

"$(INTDIR)\ptlib.def" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__PTLIB)
	<<tempfile.bat 
	@echo off 
	MergeSym -I "$(Include)$(INCLUDE)" -x ptlib.ignore $(OutDir)\ptlibs.lib $(InputPath) 
	copy $(InputPath)+nul $(IntDir)\$(TargetName).def > nul 
<< 
	

!ENDIF 

SOURCE=..\..\..\include\ptlib\msos\ptlibd.dtf

!IF  "$(CFG)" == "PTLib - Win32 Release"

!ELSEIF  "$(CFG)" == "PTLib - Win32 Debug"

IntDir=.\..\..\..\Lib\Debug
OutDir=.\..\..\..\Lib
TargetName=PTLibd
InputPath=..\..\..\include\ptlib\msos\ptlibd.dtf
USERDEP__PTLIBD="$(OutDir)\ptlibsd.lib"	

"$(INTDIR)\PTLibd.def" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__PTLIBD)
	<<tempfile.bat 
	@echo off 
	MergeSym -I "$(Include)$(INCLUDE)" -x ptlib.ignore $(OutDir)\ptlibsd.lib $(InputPath) 
	copy $(InputPath)+nul $(IntDir)\$(TargetName).def  > nul 
<< 
	

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Debug"

IntDir=.\..\..\..\Lib\Debug
OutDir=.\..\..\..\Lib
TargetName=PTLibd
InputPath=..\..\..\include\ptlib\msos\ptlibd.dtf
USERDEP__PTLIBD="$(OutDir)\ptlibsd.lib"	

"$(INTDIR)\PTLibd.def" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__PTLIBD)
	<<tempfile.bat 
	@echo off 
	MergeSym -I "$(Include)$(INCLUDE)" -x ptlib.ignore $(OutDir)\ptlibsd.lib $(InputPath) 
	copy $(InputPath)+nul $(IntDir)\$(TargetName).def  > nul 
<< 
	

!ELSEIF  "$(CFG)" == "PTLib - Win32 SSL Release"

!ENDIF 


!ENDIF 

