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

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MergeSym - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\MergeSym.exe"

!ELSE 

ALL : "Console - Win32 Release" "$(OUTDIR)\MergeSym.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"Console - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MergeSym.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\msos\include" /I "..\..\common" /D "NDEBUG" /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MergeSym.pdb" /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj" \
	"..\..\Lib\ptlibs.lib"

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

$(DS_POSTBUILD_DEP) : "Console - Win32 Release" "$(OUTDIR)\MergeSym.exe"
   copy Release\mergesym.exe ..\..\lib > nul
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\MergeSym.exe"

!ELSE 

ALL : "Console - Win32 Debug" "$(OUTDIR)\MergeSym.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"Console - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MergeSym.exe"
	-@erase "$(OUTDIR)\MergeSym.ilk"
	-@erase "$(OUTDIR)\MergeSym.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\..\msos\include" /I "..\..\common" /D "_DEBUG" /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MergeSym.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj" \
	"..\..\Lib\ptlibsd.lib"

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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\MergeSym.exe"

!ELSE 

ALL : "Console - Win32 SSL Release" "$(OUTDIR)\MergeSym.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"Console - Win32 SSL ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MergeSym.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\msos\include" /I "..\..\common" /D "NDEBUG" /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibs.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MergeSym.pdb" /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj" \
	"..\..\Lib\ptlibs.lib"

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

$(DS_POSTBUILD_DEP) : "Console - Win32 SSL Release" "$(OUTDIR)\MergeSym.exe"
   copy Release\mergesym.exe ..\..\lib > nul
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 SSL Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\MergeSym.exe"

!ELSE 

ALL : "Console - Win32 SSL Debug" "$(OUTDIR)\MergeSym.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"Console - Win32 SSL DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\MergeSym.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MergeSym.exe"
	-@erase "$(OUTDIR)\MergeSym.ilk"
	-@erase "$(OUTDIR)\MergeSym.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\..\msos\include" /I "..\..\common" /D "_DEBUG" /Fp"$(INTDIR)\MergeSym.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MergeSym.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibsd.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MergeSym.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MergeSym.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\MergeSym.obj" \
	"..\..\Lib\ptlibsd.lib"

"$(OUTDIR)\MergeSym.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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
!IF EXISTS("MergeSym.dep")
!INCLUDE "MergeSym.dep"
!ELSE 
!MESSAGE Warning: cannot find "MergeSym.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MergeSym - Win32 Release" || "$(CFG)" == "MergeSym - Win32 Debug" || "$(CFG)" == "MergeSym - Win32 SSL Release" || "$(CFG)" == "MergeSym - Win32 SSL Debug"

!IF  "$(CFG)" == "MergeSym - Win32 Release"

"Console - Win32 Release" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 Release" 
   cd "..\..\..\tools\MergeSym"

"Console - Win32 ReleaseCLEAN" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\..\tools\MergeSym"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 Debug"

"Console - Win32 Debug" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 Debug" 
   cd "..\..\..\tools\MergeSym"

"Console - Win32 DebugCLEAN" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\..\..\tools\MergeSym"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 SSL Release"

"Console - Win32 SSL Release" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 SSL Release" 
   cd "..\..\..\tools\MergeSym"

"Console - Win32 SSL ReleaseCLEAN" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 SSL Release" RECURSE=1 CLEAN 
   cd "..\..\..\tools\MergeSym"

!ELSEIF  "$(CFG)" == "MergeSym - Win32 SSL Debug"

"Console - Win32 SSL Debug" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 SSL Debug" 
   cd "..\..\..\tools\MergeSym"

"Console - Win32 SSL DebugCLEAN" : 
   cd "\Work\pwlib\src\ptlib\msos"
   $(MAKE) /$(MAKEFLAGS) /F .\Console.mak CFG="Console - Win32 SSL Debug" RECURSE=1 CLEAN 
   cd "..\..\..\tools\MergeSym"

!ENDIF 

SOURCE=.\MergeSym.cxx

"$(INTDIR)\MergeSym.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

