# Microsoft Developer Studio Generated NMAKE File, Based on MergeSym.dsp
!IF "$(CFG)" == ""
CFG=MergeSym - Win32 SSL Debug
!MESSAGE No configuration specified. Defaulting to MergeSym - Win32 SSL Debug.
!ENDIF 

!IF "$(CFG)" != "MergeSym - Win32 Release" && "$(CFG)" != "MergeSym - Win32 Debug" && "$(CFG)" != "MergeSym - Win32 SSL Release" && "$(CFG)" != "MergeSym - Win32 SSL Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MergeSym.mak" CFG="MergeSym - Win32 SSL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MergeSym - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MergeSym - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "MergeSym - Win32 SSL Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MergeSym - Win32 SSL Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "MergeSym - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MergeSym.exe"


CLEAN :
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MergeSym.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\msos\include" /I "..\..\common" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MergeSym.pdb" /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj"

"$(OUTDIR)\MergeSym.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Copying symbol merge utility to pwlib/lib directory.
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\MergeSym.exe"
   copy Release\mergesym.exe ..\..\lib > nul
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MergeSym.exe"


CLEAN :
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MergeSym.exe"
	-@erase "$(OUTDIR)\MergeSym.ilk"
	-@erase "$(OUTDIR)\MergeSym.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\..\msos\include" /I "..\..\common" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MergeSym.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj"

"$(OUTDIR)\MergeSym.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MergeSym - Win32 SSL Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MergeSym.exe"


CLEAN :
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MergeSym.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\msos\include" /I "..\..\common" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MergeSym.pdb" /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj"

"$(OUTDIR)\MergeSym.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Copying symbol merge utility to pwlib/lib directory.
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\MergeSym.exe"
   copy Release\mergesym.exe ..\..\lib > nul
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 SSL Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MergeSym.exe"


CLEAN :
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MergeSym.exe"
	-@erase "$(OUTDIR)\MergeSym.ilk"
	-@erase "$(OUTDIR)\MergeSym.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\..\msos\include" /I "..\..\common" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MergeSym.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj"

"$(OUTDIR)\MergeSym.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MergeSym.dep")
!INCLUDE "MergeSym.dep"
!ELSE 
!MESSAGE Warning: cannot find "MergeSym.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MergeSym - Win32 Release" || "$(CFG)" == "MergeSym - Win32 Debug" || "$(CFG)" == "MergeSym - Win32 SSL Release" || "$(CFG)" == "MergeSym - Win32 SSL Debug"
SOURCE=.\MergeSym.cxx

"$(INTDIR)\MergeSym.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

