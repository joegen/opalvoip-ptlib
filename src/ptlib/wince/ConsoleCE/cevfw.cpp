//
// (c) 2002 Yuri Kiryanov, openh323@kiryanov.com
// 
// Windows CE port of OpenH323 Open Source Project, www.openh323.org
// Video For Windows Definitions

#include <mmsystemx.h>
#include <cevfw.h>

// Vfw
DWORD capGetVideoFormat(HWND hwnd, LPCAPSTATUS s, UINT wSize)
{
	// Not yet implemented
	return -1L;
}

DWORD capGetVideoFormatSize(HWND hwnd)
{
	// Not yet implemented
	return -1L;
}

BOOL capGetStatus(HWND hwnd, LPCAPSTATUS s, int wSize)
{
	// Not yet implemented
	return FALSE;
}

DWORD capGetVideoFormat(HWND hwnd, LPVOID s, int wSize)
{
	// Not yet implemented
	return -1L;
}

BOOL capSetVideoFormat(HWND hwnd, LPVOID s, int wSize)
{
	// Not yet implemented
	return FALSE;
}


BOOL capGrabFrameNoStop(HWND hwnd)
{
	// Not yet implemented
	return FALSE;
}


BOOL capGetDriverDescription (UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer)
{
	// Not yet implemented
	return FALSE;
}


HWND capCreateCaptureWindow (
        LPCSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID)
{
	// Not yet implemented
	return NULL;
}

BOOL capSetCallbackOnError(HWND hwnd, CAPERRORCALLBACK fpProc)
{
	// Not yet implemented
	return FALSE;
}

BOOL capSetCallbackOnFrame(HWND hwnd, CAPVIDEOCALLBACK fpProc)
{
	// Not yet implemented
	return FALSE;
}

BOOL capSetCallbackOnVideoStream(HWND hwnd, CAPVIDEOCALLBACK fpProc)
{
	// Not yet implemented
	return FALSE;
}


BOOL capDriverConnect(HWND hwnd, int i) 
{
	// Not yet implemented
	return FALSE;
}

BOOL capDriverDisconnect(HWND hwnd)
{
	// Not yet implemented
	return FALSE;
}


BOOL capSetUserData(HWND hwnd, LPVOID lUser)
{
	// Not yet implemented
	return FALSE;
}

LPVOID capGetUserData(HWND hwnd)
{
	// Not yet implemented
	return NULL;
}

BOOL capCaptureGetSetup(HWND hwnd, LPCAPTUREPARMS s, int wSize)
{
	// Not yet implemented
	return FALSE;
}

BOOL capCaptureSetSetup(HWND hwnd, LPCAPTUREPARMS s, int wSize)
{
	// Not yet implemented
	return FALSE;
}


BOOL capDriverGetCaps(HWND hwnd, LPCAPDRIVERCAPS s, int wSize)
{
	// Not yet implemented
	return FALSE;
}

BOOL capPreview(HWND hwnd, BOOL f) 
{
	// Not yet implemented
	return FALSE;
}

