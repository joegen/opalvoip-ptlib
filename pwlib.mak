
!IFNDEF PWLIBDIR
PWLIBDIR=c:\work\pwlib
!ENDIF

INCLUDE=$(INCLUDE);$(PWLIBDIR)\include\ptlib\msos;$(PWLIBDIR)\include\pwlib\mswin;$(PWLIBDIR)\include
LIB=$(LIB);$(PWLIBDIR)\Lib

all:
	cd src\ptlib\msos
	nmake /nologo /f "Console.mak" CFG="Console - Win32 Release" RECURSE=0
	cd ..\..\..\tools\mergesym
	nmake /nologo /f "MergeSym.mak" CFG="MergeSym - Win32 Release" RECURSE=0
	cd ..\..\src\ptlib\msos
	nmake /nologo /f "PTLib.mak" CFG="PTLib - Win32 Release" RECURSE=0
!IFDEF OPENSSLDIR
OPENSSLFLAG=1
OPENSSLLIBS="ssleay32.lib libeay32.lib"
	nmake /nologo /f "Console Components.mak" CFG="Console Components - Win32 SSL Release" RECURSE=0
!ELSE
	nmake /nologo /f "Console Components.mak" CFG="Console Components - Win32 Release" RECURSE=0
!ENDIF
	cd ..\..\pwlib\mswin
	nmake /nologo /f "GUI.mak" CFG="GUI - Win32 Release" RECURSE=0
	nmake /nologo /f "PWLib.mak" CFG="PWLib - Win32 Release" RECURSE=0
	nmake /nologo /f "GUI Components.mak" CFG="GUI Components - Win32 Release" RECURSE=0
	cd ..\..\..\tools\asnparser
	nmake /nologo /f "ASNParser.mak" CFG="ASNParser - Win32 Release" RECURSE=0
	cd ..\pwrc
	nmake /nologo /f "pwrc.mak" CFG="pwrc - Win32 Release" RECURSE=0
	cd ..\pwtest
	nmake /nologo /f "PWTest.mak" CFG="pwtest - Win32 Release" RECURSE=0


debug: all
	nmake /nologo /f "Console.mak" CFG="Console - Win32 Debug" RECURSE=0
	nmake /nologo /f "PTLib.mak" CFG="PTLib - Win32 Debug" RECURSE=0
	nmake /nologo /f "Console Components.mak" CFG="Console Components - Win32 Debug" RECURSE=0
	nmake /nologo /f "GUI.mak" CFG="GUI - Win32 Debug" RECURSE=0
	nmake /nologo /f "PWLib.mak" CFG="PWLib - Win32 Debug" RECURSE=0
	nmake /nologo /f "GUI Components.mak" CFG="GUI Components - Win32 Debug" RECURSE=0
	nmake /nologo /f "PWTest.mak" CFG="pwtest - Win32 Debug" RECURSE=0
