/*
 * vfw.cxx
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
 * $Log: vfw.cxx,v $
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoInputDevice::PVideoInputDevice(VideoFormat videoFmt,
                                     unsigned channel,
                                     ColourFormat colourFmt)
  : PVideoDevice(videoFmt, channel, colourFmt)
{
  hCaptureWindow = NULL;
  bitmapInfo = NULL;
}


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  Close();

  deviceName = devName;

  if ((hCaptureWindow = capCreateCaptureWindow("Capture Window",
                                               WS_POPUP | WS_CAPTION,
                                               CW_USEDEFAULT, CW_USEDEFAULT,
                                               frameWidth + GetSystemMetrics(SM_CXFIXEDFRAME),
                                               frameHeight + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME),
                                               (HWND)0,
                                               0)) == NULL) {
    PTRACE(1, "capCreateCaptureWindow: failed");
    return FALSE;
  }

  capSetCallbackOnError(hCaptureWindow, ErrorHandler);

  if (!capSetCallbackOnVideoStream(hCaptureWindow, VideoHandler)) {
    PTRACE(1, "capSetCallbackOnVideoStream: failed - " << GetLastError());
    Close();
    return FALSE;
  }

  WORD devId;
  if (deviceName.GetLength() == 1 && isdigit(deviceName[0]))
    devId = (WORD)(deviceName[0] - '0');
  else {
    for (devId = 0; devId < 10; devId++) {
      char name[100];
      char version[200];
      if (capGetDriverDescription(devId, name, sizeof(name), version, sizeof(version)) &&
          (deviceName *= name))
        break;
    }
  }

  // Use first driver available.
  if (!capDriverConnect(hCaptureWindow, devId)) {
    PTRACE(1, "capDriverConnect: failed - " << GetLastError());
    Close();
    return FALSE;
  }

  capSetUserData(hCaptureWindow, this);

  if (!capDriverGetCaps(hCaptureWindow, &driverCaps, sizeof(driverCaps))) {
    PTRACE(1, "capGetDriverCaps: failed - " << GetLastError());
    Close();
    return FALSE;
  }

  CAPTUREPARMS parms;
  if (!capCaptureGetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    PTRACE(1, "capCaptureGetSetup: failed - " << GetLastError());
    Close();
    return FALSE;
  }

  parms.dwRequestMicroSecPerFrame = 1000000 / 15;
  parms.wPercentDropForError = 100;
  parms.fUsingDOSMemory = FALSE;
  parms.wNumVideoRequested = 3;
  parms.fYield = TRUE;
  parms.fMakeUserHitOKToCapture = FALSE;
  parms.fCaptureAudio = FALSE;
  parms.vKeyAbort = 0;
  parms.fAbortLeftMouse = FALSE;
  parms.fAbortRightMouse = FALSE;
  parms.fLimitEnabled = FALSE;
  parms.fMCIControl = FALSE;
  parms.wStepCaptureAverageFrames = 1;

  if (!capCaptureSetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    PTRACE(1, "capCaptureSetSetup: failed - " << GetLastError());
    Close();
    return FALSE;
  }

  bitmapInfoBuffer.SetSize(capGetVideoFormatSize(hCaptureWindow));
  bitmapInfo = (LPBITMAPINFO)bitmapInfoBuffer.GetPointer();
  capGetVideoFormat(hCaptureWindow, bitmapInfo, bitmapInfoBuffer.GetSize());

  if (driverCaps.fHasOverlay)
    capOverlay(hCaptureWindow, TRUE);
  else {
    capPreviewRate(hCaptureWindow, 66);
    capPreview(hCaptureWindow, TRUE);
  }

  if (startImmediate)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice::IsOpen() const
{
  return hCaptureWindow != NULL;
}


BOOL PVideoInputDevice::Close()
{
  if (!IsOpen())
    return FALSE;

  capCaptureStop(hCaptureWindow);
  capDriverDisconnect(hCaptureWindow);
  capSetCallbackOnVideoStream(hCaptureWindow, NULL);

  DestroyWindow(hCaptureWindow);
  hCaptureWindow = NULL;

  return TRUE;
}


BOOL PVideoInputDevice::Start()
{
  if (capCaptureSequenceNoFile(hCaptureWindow))
    return TRUE;

  PTRACE(1, "capCaptureSequenceNoFile: failed - " << GetLastError());
  return FALSE;
}


BOOL PVideoInputDevice::Stop()
{
  if (capCaptureStop(hCaptureWindow))
    return TRUE;

  PTRACE(1, "capCaptureStop: failed - " << GetLastError());
  return FALSE;
}


PStringList PVideoInputDevice::GetDeviceNames() const
{
  PStringList list;

  for (WORD devId = 0; devId < 10; devId++) {
    char name[100];
    char version[200];
    if (capGetDriverDescription(devId, name, sizeof(name), version, sizeof(version)))
      list.AppendString(name);
  }

  return list;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return bitmapInfo->bmiHeader.biSizeImage;
}


BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  memcpy(buffer, lastFramePtr, lastFrameSize);
  if (bytesReturned != NULL)
    *bytesReturned = lastFrameSize;

  return TRUE;
}


    
LRESULT CALLBACK PVideoInputDevice::ErrorHandler(HWND hWnd, int id, LPCSTR err)
{
  if (hWnd == NULL)
    return FALSE;

  return ((PVideoInputDevice *)capGetUserData(hWnd))->HandleError(id, err);
}


LRESULT PVideoInputDevice::HandleError(int id, LPCSTR err)
{
  if (id != 0) {
    PTRACE(1, "ErrorHandler: [id="<< id << "] " << err);
  }

  return TRUE;
}


LRESULT CALLBACK PVideoInputDevice::VideoHandler(HWND hWnd, LPVIDEOHDR vh)
{
  if (hWnd == NULL)
    return FALSE;

  return ((PVideoInputDevice *)capGetUserData(hWnd))->HandleVideo(vh);
}


LRESULT PVideoInputDevice::HandleVideo(LPVIDEOHDR vh)
{
  if (vh->dwFlags & VHDR_DONE) {
    lastFramePtr = vh->lpData;
    lastFrameSize = vh->dwBytesUsed;
    frameAvailable.Signal();
  }

  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
