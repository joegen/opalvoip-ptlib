//
// (c) 2002 Yuri Kiryanov, openh323@kiryanov.com
// 
// Windows CE port of OpenH323 Open Source Project, www.openh323.org
// Video For Windows Definitions

#include <mmsystemx.h>
#include <cevfw.h>

BOOL VFWAPI capDefGetDriverDescription (UINT, LPSTR, int, LPSTR, int ) { return FALSE; }
HWND VFWAPI capDefCreateCaptureWindow(LPCSTR, DWORD, int, int, int, int, HWND, int) { return NULL; }

// Functions
CAPGETDRIVERDESCRIPTIONPROC capGetDriverDescription; // = capDefGetDriverDescription;
CAPCREATECAPTUREWINDOWPROC capCreateCaptureWindow; // = capDefCreateCaptureWindow;
