/*
 * vsdl.cxx
 *
 * Classes to support video output via SDL
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
 * $Log: vsdl.cxx,v $
 * Revision 1.1  2003/04/28 07:03:55  craigs
 * Initial version from ohphone
 *
 */

#ifdef __GNUC__
#pragma implementation "vsdl.h"
#endif

#include <ptlib.h>
#include <ptclib/vsdl.h>

#if	P_SDL

extern "C" {

#include <SDL.h>

};

PSDLVideoDevice::PSDLVideoDevice(const PString & _remoteName,
			                                      BOOL _isEncoding, 
                               PSDLDisplayThread *_sdlThread)
{
  PTRACE(3, "PSDL\tGeneric SDL video constructor start for " << (isEncoding ? "local" : "remote") );
  
  deviceName = PString("SDL");
  remoteName = _remoteName; 
  isEncoding = _isEncoding;
  sdlThread  = _sdlThread;
       
  width=0;
  height=0;

  sdlThread->RequestOpenWindow(isEncoding);
}

PSDLVideoDevice::~PSDLVideoDevice()
{ 
  Close();
}

BOOL PSDLVideoDevice::Close()
{
  sdlThread->RequestCloseWindow(isEncoding);
  return TRUE;
}

BOOL PSDLVideoDevice::SetFrameSize (unsigned _width, unsigned _height)
{
  width = _width;
  height = _height;

  return TRUE;
}

BOOL PSDLVideoDevice::Redraw (const void *frame)
{
  PSDLVideoFrame *newWindow = new PSDLVideoFrame(width, height, frame);

  sdlThread->AddFrame(newWindow, isEncoding);

  return TRUE;
}

BOOL PSDLVideoDevice::IsOpen()
{
  return TRUE;
}

BOOL PSDLVideoDevice::SetFrameData(unsigned /*x*/,
                                   unsigned /*y*/,
                                   unsigned width,
                                   unsigned height,
                               const BYTE * data,
                                       BOOL endFrame) 
{
  if (!endFrame)
    return FALSE;

  PSDLVideoFrame *newWindow = new PSDLVideoFrame(width, height, data);

  sdlThread->AddFrame(newWindow, isEncoding);

  return TRUE;
}


BOOL PSDLVideoDevice::EndFrame()
{
  return TRUE;
}

PStringList PSDLVideoDevice::GetDeviceNames() const
{
  PStringList  devlist;
  devlist.AppendString(GetDeviceName());

  return devlist;
}

//////////////////////////////////////////////////////////////

PSDLDisplayThread::PSDLDisplayThread(BOOL _videoPIP)
	: PThread(1000, NoAutoDeleteThread, LowPriority, "SDL display")
{
	PINDEX i;
  for (i = 0; i < 2; i++) {
    width[i] = 0;
    height[i] = 0;
    overlay[i] = NULL;
    displayPosn[i].x = 0;
    displayPosn[i].y = 0;
    displayPosn[i].w = 0;
    displayPosn[i].h = 0;
  }  

  oldScreenWidth = 0;
  oldScreenHeight = 0;

  closeEncWindow = FALSE;
  closeRecWindow = FALSE;
  
  screen     = NULL;
  threadOpen = TRUE;
  displayIsShutDown = FALSE;

  nextEncFrame = NULL;
  nextRcvFrame = NULL;
  videoPIP = _videoPIP;
  Resume();
}

PSDLDisplayThread::~PSDLDisplayThread()
{
}

BOOL PSDLDisplayThread::AddFrame(PSDLVideoFrame *newFrame, BOOL isEncoding)
{
  PTRACE(3, "PSDL\tAddFrame runs here for frame " << *newFrame  << " " << GetDirName(isEncoding));

  accessLock.Wait();

  if (!IsOpen()) { //This frame not wanted, as the display thread is closed.
    PTRACE(2, "PSDL\tDelete unwanted " << GetDirName(isEncoding) << " frame");
    delete newFrame;
    accessLock.Signal();
    return FALSE;
  }

  PSDLVideoFrame **frameQ;

  if (isEncoding) 
    frameQ = &nextEncFrame;
  else
    frameQ = &nextRcvFrame;

  if ((*frameQ) != NULL) {
    PTRACE(2, "PSDL\tDelete unused " << GetDirName(isEncoding) << " frame");
    delete *frameQ;
  }

  *frameQ = newFrame;
  PTRACE(4, "PSDL\tAdd new " << GetDirName(isEncoding) << " frame");

  accessLock.Signal();
  readFlag.Signal();
  return TRUE;
}

BOOL PSDLDisplayThread::IsOpen()
{
  return threadOpen;
}

BOOL PSDLDisplayThread::ScreenIsOpen()
{
  return (screen != NULL);
}

BOOL PSDLDisplayThread::DisplayIsShutDown()
{
  return displayIsShutDown;
}

void PSDLDisplayThread::Terminate()
{
  PTRACE(3, "PSDL\tSdl has been told to terminate. Set flags to close");
  accessLock.Wait();
  
  if (nextEncFrame != NULL)
    delete nextEncFrame;

  if (nextRcvFrame != NULL)
    delete nextRcvFrame;

  nextEncFrame = NULL;
  nextRcvFrame = NULL;

  threadOpen = FALSE;

  accessLock.Signal();
  readFlag.Signal();
}

void PSDLDisplayThread::RequestCloseWindow(BOOL isEncoding)
{
  PTRACE(3, "PSDL\tRequest: Close window " << GetDirName(isEncoding) << " video");

  accessLock.Wait();

  if (isEncoding)
    closeEncWindow = TRUE;
  else
    closeRecWindow = TRUE;

  accessLock.Signal();
  readFlag.Signal();
}

void PSDLDisplayThread::RequestOpenWindow(BOOL isEncoding)
{
  PTRACE(3, "PSDL\tRequest: Open window " << GetDirName(isEncoding) << " video");

  PWaitAndSignal m(accessLock);
  if (isEncoding)
    closeEncWindow = FALSE;
  else
    closeRecWindow = FALSE;
}


PSDLVideoFrame * PSDLDisplayThread::GetNextFrame(BOOL isEncoding)
{
  PSDLVideoFrame *res;
  accessLock.Wait();

  if (!IsOpen()) {
    accessLock.Signal();
    return FALSE;
  }

  PSDLVideoFrame **frameQ;

  if (isEncoding) 
    frameQ = &nextEncFrame;
  else
    frameQ = &nextRcvFrame;

  res = *frameQ;
  *frameQ = NULL;
  accessLock.Signal();  

  return res;
}

BOOL PSDLDisplayThread::InitialiseSdl()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    PTRACE(0,"Couldn't initialize SDL: " << SDL_GetError());
    return FALSE;      
    }
  PTRACE(6, "PSDL\thas just initialised SDL for Video usage");
  return TRUE;
}

BOOL PSDLDisplayThread::ResizeScreen(unsigned newWidth, unsigned newHeight)
{   
  if ((oldScreenWidth == newWidth) && (oldScreenHeight == newHeight)) 
    return TRUE;

  PTRACE(6, "PSDL\tResize screen to " << newWidth << "x" << newHeight);
  oldScreenWidth  = newWidth;
  oldScreenHeight = newHeight;
 
  PTRACE(3,"SDL Open SDL Screen of size " << newWidth << "x" << newHeight);
  
  WriteOutDisplay();

  screen = SDL_SetVideoMode(newWidth, newHeight, 0, SDL_SWSURFACE | SDL_RESIZABLE );
  
  if (!ScreenIsOpen()) {
    PTRACE(0,"Could not open screen to display window: " << SDL_GetError());
    SDL_Quit();
    return FALSE;
  }
  
  PTRACE(3,"PSDL\tSuccessfully resize a SDL screen. New size= " << newWidth << "x" << newHeight);
  return TRUE;
}

void PSDLDisplayThread::InitDisplayPosn()
{
  PTRACE(6, "PSDL\tInitDisplayPosition now");

  SDL_Surface * currentSurface = SDL_GetVideoSurface();
  if (currentSurface != NULL)
    InitDisplayPosn(currentSurface->w, currentSurface->h);
}

void PSDLDisplayThread::InitDisplayPosn(unsigned w, unsigned h)
{  
  displayPosn[RemoteIndex].x = 0;
  displayPosn[RemoteIndex].y = 0;
  displayPosn[EncodeIndex].y = 0;

  double halfWidth;
  double halfHeight1;
  double halfHeight2;
  if (videoPIP) {
    halfWidth  =  width[RemoteIndex] / (double)((width[EncodeIndex] >> 1) + width[RemoteIndex]);
    halfHeight1 = (height[EncodeIndex] >> 1) / (double)PMAX(height[EncodeIndex] >> 1, height[RemoteIndex]);
    halfHeight2 = height[RemoteIndex]  / (double)PMAX(height[EncodeIndex] >> 1, height[RemoteIndex]);
  } else {
    halfWidth = width[RemoteIndex] / (double)(width[RemoteIndex] + width[EncodeIndex]);
    halfHeight1 = height[EncodeIndex] / (double)PMAX(height[EncodeIndex], height[RemoteIndex]);
    halfHeight2 = height[RemoteIndex] / (double)PMAX(height[EncodeIndex], height[RemoteIndex]);
  }

  displayPosn[EncodeIndex].x = (short) (halfWidth * w);
  displayPosn[EncodeIndex].w = (short) (w - displayPosn[EncodeIndex].x);
  displayPosn[EncodeIndex].h = (short) (halfHeight1 * h);

  displayPosn[RemoteIndex].w = displayPosn[EncodeIndex].x;
  displayPosn[RemoteIndex].h = (short) (halfHeight2 * h);

  WriteOutDisplay();
}

void PSDLDisplayThread::CloseScreen(void)
{ 
  if (screen != NULL) {    
    PTRACE(3,"PSDL\t close the SDL display screen.");

    SDL_FreeSurface(screen);
    screen = NULL;
    oldScreenWidth = 0;
    oldScreenHeight = 0;
    SDL_Quit();
  } 
}

BOOL PSDLDisplayThread::CreateOverlay(BOOL isEncoding)
{
  unsigned dispIndex = GetDisplayIndex(isEncoding);
  unsigned w = width[dispIndex];
  unsigned h = height[dispIndex];
  
  if ((overlay[dispIndex] != NULL) && 
      ((unsigned)overlay[dispIndex]->w == w)  &&
      ((unsigned)overlay[dispIndex]->h == h)  )
   return TRUE;
  
  //AT this stage, we decide that we need a new overlay.
  //Get rid of the old overlay, then make the new overlay.    
  if (overlay[dispIndex] != NULL) 
    SDL_FreeYUVOverlay(overlay[dispIndex]);

  if ((w == 0) || (h == 0)) { //An overlay of size 0x0 is meaningless.
    overlay[dispIndex]= NULL;
    PTRACE(6, "PSDL\tOverlay of null hsize has been created");
    return TRUE;
  }
  
  overlay[dispIndex] = SDL_CreateYUVOverlay(w, h, SDL_IYUV_OVERLAY, screen);
 
  if (overlay[dispIndex] == NULL) {
    PTRACE(1, "PSDL\t Could not open overlay to display window: " << SDL_GetError());
    return FALSE;
  }
  
  PTRACE(3, "PSDL\t Successfully create the YUV overlay of size " << w << "x" << h );
  return TRUE;
}

BOOL PSDLDisplayThread::SetOverlaySize (BOOL isEncoding, unsigned _width, unsigned _height)
{
  unsigned dispIndex = GetDisplayIndex(isEncoding);

  if ((width[dispIndex] != _width) || (height[dispIndex] != _height)) {
    PTRACE(3, "PSDL\t Requested internal size of " << _width << "x" << _height);
    PTRACE(3, "PSDL\t did not match internal size of " 
	   << width[dispIndex] << "x" << height[dispIndex]);
    
    if((_width != 0) && (_height != 0)) {
      width[dispIndex] = _width;
      height[dispIndex] = _height;
      InitDisplayPosn();
      CreateOverlay(isEncoding);
    }    
  }

  return TRUE;
}

void PSDLDisplayThread::CloseWindow(BOOL isEncoding)
{
  PINDEX dispIndex = GetDisplayIndex(isEncoding);
  PTRACE(3, "PSDL\tClose window " << dispIndex);

  if (!(width[dispIndex] || height[dispIndex])) {
    return;
  }

  if(overlay[dispIndex] != NULL) {
    PTRACE(3, "PSDL\tClose the overlay for window " << dispIndex);
    SDL_FreeYUVOverlay(overlay[dispIndex]);
    overlay[dispIndex] = NULL;    
  }
  
  width[dispIndex] = 0;
  height[dispIndex] = 0;
  displayPosn[dispIndex].w = 0;
  displayPosn[dispIndex].h = 0;

  if(overlay[1-dispIndex] == NULL)
    CloseScreen();
  else
    ResizeScreen(displayPosn[1-dispIndex].w, displayPosn[1-dispIndex].h);
}


void PSDLDisplayThread::ProcessSdlEvents(void) 
{
  if (!ScreenIsOpen()) {
    PTRACE(6, "PSDL\t Screen not open, so dont process events");
    return;
  }

  SDL_Event event;  
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {//User selected cross
      
      PTRACE(3, "PSDL\t user selected cross on window, close window");
      CloseWindow(TRUE);
      CloseWindow(FALSE);
      CloseScreen();
      displayIsShutDown = TRUE;
      return;
    }

    if (event.type == SDL_VIDEORESIZE){
      PTRACE(3, "PSDL\t Resize window to " << event.resize.w << " x " << event.resize.h);
      ResizeScreen(event.resize.w, event.resize.h);
      InitDisplayPosn();
    }    //end event.type==SDL_VIDEORESIZE
  }      //end while (SDL_PollEvent())
}

BOOL PSDLDisplayThread::Redraw(BOOL isEncoding, PSDLVideoFrame *frame)
{
  if (frame == NULL)
    return TRUE;

  unsigned dispIndex = GetDisplayIndex(isEncoding);

  PTRACE(6, "PSDL\tRedraw starts now. Window " << dispIndex);  
        
  if (DisplayIsShutDown()) {   //Some task somewhere has closed our display device.
    PTRACE(6, "PSDL\tScreen is closed. ::Redraw() returning immediately"); 
    return TRUE;   //(could have been user selecting cross on the window)
  }

  if ((width[dispIndex] == 0) || (height[dispIndex] == 0)) {
    unsigned w = oldScreenWidth;
    unsigned newH;
    if (isEncoding && videoPIP) {
      w += frame->GetWidth() / 2;
      newH = frame->GetHeight() / 2; 
    } else {
      w += frame->GetWidth();
      newH = frame->GetHeight();
    }
    unsigned h = PMAX(oldScreenHeight, newH);

    ResizeScreen(w, h);
  }

  SetOverlaySize(isEncoding, frame->GetWidth(), frame->GetHeight());

  SDL_Overlay *yuvOverlay= overlay[dispIndex];

  if (yuvOverlay == NULL) {
    PTRACE(6, "PSDL\tRedraw end prematurely, no overlay for window " << dispIndex);
    return TRUE;       //Again, we lost the overlay, exit quickly.
  }
  
  if (overlay[1 - dispIndex] == NULL) {
    PTRACE(6, "PSDL\tRedraw end prematurely, other overlay undefined");
    return TRUE;       //Again, we lost the overlay, exit quickly.
  }

  unsigned char * base = frame->GetDataPointer();
  SDL_LockYUVOverlay(yuvOverlay);

  PINDEX pixelsFrame = yuvOverlay->w * yuvOverlay->h;
  PINDEX pixelsQuartFrame = pixelsFrame >> 2;
  memcpy(yuvOverlay->pixels[0], base,                                  pixelsFrame);
  memcpy(yuvOverlay->pixels[1], base + pixelsFrame,                    pixelsQuartFrame);
  memcpy(yuvOverlay->pixels[2], base + pixelsFrame + pixelsQuartFrame, pixelsQuartFrame);

  SDL_UnlockYUVOverlay(yuvOverlay);

  SDL_DisplayYUVOverlay(yuvOverlay, displayPosn + dispIndex );    

  PTRACE(6, "PSDL\tFinish the redraw for window " << GetDirName(isEncoding));

  return TRUE;  
}

void PSDLDisplayThread::WriteOutDisplay(void)
{
#ifdef PTRACING
  PINDEX i;
  for(i = 0; i < 2; i++) {
    PTRACE(6, "PSDL\t" << i << " display x,y=" << displayPosn[i].x << " "
	   << displayPosn[i].y );
  
    PTRACE(6, "PSDL\t" << i << " display w,h=" << displayPosn[i].w << " "
	 << displayPosn[i].h );
  
    PTRACE(6, "PSDL\t" << i << " actual  w,h = " << width[i] << " x " << height[i]);

    PStringStream str;
    if (overlay[i] != NULL)
      str << (void *) overlay[i] << "  size is " << overlay[i]->w << " x " << overlay[i]->h;
    else
      str << "NULL";
    PTRACE(6, "PSDL\t Window:" << i << " Overlay  is "  << str);
  }

  PTRACE(6, "PSDL\t Screen is " << (void *) screen);
#endif
}

unsigned PSDLDisplayThread::GetDisplayIndex(BOOL isEncoding)
{
  if (isEncoding)
    return EncodeIndex;
  else
    return RemoteIndex;
}

void PSDLDisplayThread::Main()
{
  PSDLVideoFrame *frame;
  InitialiseSdl();
  PThread::Current()->SetPriority(LowestPriority);
  PTRACE(3, "PSDL\tMain loop is underway, with SDL screen initialised");

  do {
    readFlag.Wait();

    WriteOutDisplay();
    frame = GetNextFrame(TRUE);
    Redraw(TRUE, frame);
    if (frame != NULL)
      delete frame;
    
    ProcessSdlEvents();

    frame = GetNextFrame(FALSE);
    Redraw(FALSE, frame);
    if (frame != NULL)
      delete frame;
    
    ProcessSdlEvents();
    
    if (closeRecWindow) {
      CloseWindow(FALSE);     
      ProcessSdlEvents();
    }

    if (closeEncWindow) {
      CloseWindow(TRUE);
      ProcessSdlEvents();
    }

  }
  while (IsOpen());

  CloseWindow(TRUE);
  CloseWindow(FALSE);
  PTRACE(3, "PSDL\tEnd of sdl display loop");
}

///////////////////////////////////////////////////

// P_SDL
#endif
