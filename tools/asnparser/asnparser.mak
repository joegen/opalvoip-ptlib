# Microsoft Developer Studio Generated NMAKE File, Based on asnparser.dsp
!IF "$(CFG)" == ""
CFG=ASNParser - Win32 SSL Debug
!MESSAGE No configuration specified. Defaulting to ASNParser - Win32 SSL Debug.
!ENDIF 

!IF "$(CFG)" != "ASNParser - Win32 Release" && "$(CFG)" != "ASNParser - Win32 Debug" && "$(CFG)" != "ASNParser - Win32 SSL Debug" && "$(CFG)" != "ASNParser - Win32 SSL Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "asnparser.mak" CFG="ASNParser - Win32 SSL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ASNParser - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ASNParser - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "ASNParser - Win32 SSL Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "ASNParser - Win32 SSL Release" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "ASNParser - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\asnparser.exe"


CLEAN :
	-@erase "$(INTDIR)\asn_grammar.obj"
	-@erase "$(INTDIR)\asn_lex.obj"
	-@erase "$(INTDIR)\asnparser.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PreCompile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\asnparser.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\asnparser.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asnparser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\asnparser.pdb" /machine:I386 /out:"$(OUTDIR)\asnparser.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\asn_grammar.obj" \
	"$(INTDIR)\asn_lex.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\PreCompile.obj"

"$(OUTDIR)\asnparser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Copying ASN parser to pwlib/lib directory.
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\asnparser.exe"
   copy Release\asnparser.exe ..\..\lib > nul
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\asnparser.exe"


CLEAN :
	-@erase "$(INTDIR)\asn_grammar.obj"
	-@erase "$(INTDIR)\asn_lex.obj"
	-@erase "$(INTDIR)\asnparser.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PreCompile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\asnparser.exe"
	-@erase "$(OUTDIR)\asnparser.ilk"
	-@erase "$(OUTDIR)\asnparser.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\asnparser.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asnparser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\asnparser.pdb" /debug /machine:I386 /out:"$(OUTDIR)\asnparser.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\asn_grammar.obj" \
	"$(INTDIR)\asn_lex.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\PreCompile.obj"

"$(OUTDIR)\asnparser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\asnparser.exe"


CLEAN :
	-@erase "$(INTDIR)\asn_grammar.obj"
	-@erase "$(INTDIR)\asn_lex.obj"
	-@erase "$(INTDIR)\asnparser.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PreCompile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\asnparser.exe"
	-@erase "$(OUTDIR)\asnparser.ilk"
	-@erase "$(OUTDIR)\asnparser.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fp"$(INTDIR)\asnparser.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asnparser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlibd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\asnparser.pdb" /debug /machine:I386 /out:"$(OUTDIR)\asnparser.exe" /pdbtype:sept /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\asn_grammar.obj" \
	"$(INTDIR)\asn_lex.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\PreCompile.obj"

"$(OUTDIR)\asnparser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\asnparser.exe"


CLEAN :
	-@erase "$(INTDIR)\asn_grammar.obj"
	-@erase "$(INTDIR)\asn_lex.obj"
	-@erase "$(INTDIR)\asnparser.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PreCompile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\asnparser.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fp"$(INTDIR)\asnparser.pch" /Yu"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asnparser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ptlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\asnparser.pdb" /machine:I386 /out:"$(OUTDIR)\asnparser.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\asn_grammar.obj" \
	"$(INTDIR)\asn_lex.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\PreCompile.obj"

"$(OUTDIR)\asnparser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Copying ASN parser to pwlib/lib directory.
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\asnparser.exe"
   copy Release\asnparser.exe ..\..\lib > nul
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
!IF EXISTS("asnparser.dep")
!INCLUDE "asnparser.dep"
!ELSE 
!MESSAGE Warning: cannot find "asnparser.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ASNParser - Win32 Release" || "$(CFG)" == "ASNParser - Win32 Debug" || "$(CFG)" == "ASNParser - Win32 SSL Debug" || "$(CFG)" == "ASNParser - Win32 SSL Release"
SOURCE=.\asn_grammar.cxx

!IF  "$(CFG)" == "ASNParser - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /D "MSDOS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_grammar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /D "MSDOS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_grammar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /D "MSDOS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_grammar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /D "MSDOS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_grammar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\asn_grammar.y

!IF  "$(CFG)" == "ASNParser - Win32 Release"

InputPath=.\asn_grammar.y
InputName=asn_grammar

".\asn_grammar.cxx"	".\asn_grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -t -v -d $(InputName).y 
	copy $(InputName)_tab.c $(InputName).cxx 
	copy $(InputName)_tab.h $(InputName).h
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

InputPath=.\asn_grammar.y
InputName=asn_grammar

".\asn_grammar.cxx"	".\asn_grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -t -v -d $(InputName).y 
	copy $(InputName)_tab.c $(InputName).cxx 
	copy $(InputName)_tab.h $(InputName).h
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

InputPath=.\asn_grammar.y
InputName=asn_grammar

".\asn_grammar.cxx"	".\asn_grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -t -v -d $(InputName).y 
	copy $(InputName)_tab.c $(InputName).cxx 
	copy $(InputName)_tab.h $(InputName).h
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

InputPath=.\asn_grammar.y
InputName=asn_grammar

".\asn_grammar.cxx"	".\asn_grammar.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	bison -t -v -d $(InputName).y 
	copy $(InputName)_tab.c $(InputName).cxx 
	copy $(InputName)_tab.h $(InputName).h
<< 
	

!ENDIF 

SOURCE=.\asn_lex.cxx

!IF  "$(CFG)" == "ASNParser - Win32 Release"

CPP_SWITCHES=/nologo /MD /W2 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_lex.obj" : $(SOURCE) "$(INTDIR)" ".\asn_grammar.h"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W2 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_lex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

CPP_SWITCHES=/nologo /MDd /W2 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_lex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

CPP_SWITCHES=/nologo /MD /W2 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\asn_lex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\asn_lex.l

!IF  "$(CFG)" == "ASNParser - Win32 Release"

InputPath=.\asn_lex.l
InputName=asn_lex
USERDEP__ASN_L="asn_grammar.h"	"asn_grammar.cxx"	

".\asn_lex.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__ASN_L)
	<<tempfile.bat 
	@echo off 
	flex -t $(InputName).l > $(InputName).cxx
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

InputPath=.\asn_lex.l
InputName=asn_lex
USERDEP__ASN_L="asn_grammar.h"	"asn_grammar.cxx"	

".\asn_lex.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__ASN_L)
	<<tempfile.bat 
	@echo off 
	flex -t $(InputName).l > $(InputName).cxx
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

InputPath=.\asn_lex.l
InputName=asn_lex
USERDEP__ASN_L="asn_grammar.h"	"asn_grammar.cxx"	

".\asn_lex.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__ASN_L)
	<<tempfile.bat 
	@echo off 
	flex -t $(InputName).l > $(InputName).cxx
<< 
	

!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

InputPath=.\asn_lex.l
InputName=asn_lex
USERDEP__ASN_L="asn_grammar.h"	"asn_grammar.cxx"	

".\asn_lex.cxx" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)" $(USERDEP__ASN_L)
	<<tempfile.bat 
	@echo off 
	flex -t $(InputName).l > $(InputName).cxx
<< 
	

!ENDIF 

SOURCE=.\main.cxx

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\asnparser.pch" ".\asn_grammar.h"


SOURCE=.\PreCompile.cpp

!IF  "$(CFG)" == "ASNParser - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\asnparser.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\PreCompile.obj"	"$(INTDIR)\asnparser.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=0$(OPENSSLFLAG) /Fp"$(INTDIR)\asnparser.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\PreCompile.obj"	"$(INTDIR)\asnparser.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Debug"

CPP_SWITCHES=/nologo /MDd /W4 /GX /ZI /Od /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "_DEBUG" /D P_SSL=1 /Fp"$(INTDIR)\asnparser.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\PreCompile.obj"	"$(INTDIR)\asnparser.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ASNParser - Win32 SSL Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /I "..\..\include\ptlib\msos" /I "..\..\include" /I "$(OPENSSLDIR)/inc32" /D "NDEBUG" /D P_SSL=1 /Fp"$(INTDIR)\asnparser.pch" /Yc"ptlib.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\PreCompile.obj"	"$(INTDIR)\asnparser.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

