/*
 * videoio.h
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: videoio.h,v $
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */


#ifndef _PVIDEOIO

#include <vfw.h>


#include "../../videoio.h"
  protected:
    static LRESULT CALLBACK ErrorHandler(HWND hWnd, int id, LPCSTR err);
    LRESULT HandleError(int id, LPCSTR err);
    static LRESULT CALLBACK VideoHandler(HWND hWnd, LPVIDEOHDR vh);
    LRESULT HandleVideo(LPVIDEOHDR vh);

    HWND          hCaptureWindow;
    CAPDRIVERCAPS driverCaps;
    PBYTEArray    bitmapInfoBuffer;
    BITMAPINFO *  bitmapInfo;
    PSyncPoint    frameAvailable;
    LPBYTE        lastFramePtr;
    unsigned      lastFrameSize;
};


#endif
