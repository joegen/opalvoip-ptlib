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
 * Revision 1.9  2003/07/22 22:55:20  dereksmithies
 * Add memory allocation feature.
 *
 * Revision 1.8  2003/05/21 03:59:10  dereksmithies
 * Fix close down bug.
 *
 * Revision 1.7  2003/05/17 03:21:26  rjongbloed
 * Removed need to do strange things with main() function.
 *
 * Revision 1.6  2003/05/14 02:34:53  dereksmithies
 * Make SDL display work if only one of two display areas in use.
 *
 * Revision 1.5  2003/05/07 02:40:58  dereks
 * Fix to allow it to exit when the ::Terminate method called.
 *
 * Revision 1.4  2003/04/28 14:30:02  craigs
 * Started rearranging code
 *
 * Revision 1.3  2003/04/28 08:33:00  craigs
 * Linux SDL includes are in a SDL directory, but Windows is not
 *
 * Revision 1.2  2003/04/28 07:27:15  craigs
 * Added missed functions
 *
 * Revision 1.1  2003/04/28 07:03:55  craigs
 * Initial version from ohphone
 *
 */

#ifdef __GNUC__
#pragma implementation "vsdl.h"
#endif

#include <ptlib.h>
#include <ptclib/vsdl.h>

#define new PNEW

#if P_SDL

extern "C" {

#include <SDL/SDL.h>

};

#ifdef _MSC_VER
#pragma comment(lib, P_SDL_LIBRARY)
#endif


///////////////////////////////////////////////////////////////////////////////

PSDLVideoFrame::PSDLVideoFrame(unsigned newWidth, unsigned newHeight, const void *_data)
{
  Initialise(newWidth, newHeight, (Uint8 *)_data);
}


PSDLVideoFrame::PSDLVideoFrame(unsigned newWidth, unsigned newHeight, Uint8 *_data)
{
  Initialise(newWidth, newHeight, _data);
}


PSDLVideoFrame::~PSDLVideoFrame()
{
  delete data;
}


void PSDLVideoFrame::PrintOn(ostream & strm) const
{
  strm << PString(width) << "x" << PString(height);
}


void PSDLVideoFrame::Initialise(unsigned newWidth, 
			       unsigned newHeight, Uint8 *_data)
{
  width = newWidth;
  height = newHeight;
  unsigned size = (width*height *3) >> 1;
  data = new Uint8[size];
  memcpy(data, _data, size);
}


///////////////////////////////////////////////////////////////////////

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
  threadRunning = TRUE;

  PINDEX i;

  for (i = 0; i < 2; i++) {
    width[i]         = 0;
    height[i]        = 0;
    overlay[i]       = NULL;
    displayPosn[i].x = 0;
    displayPosn[i].y = 0;
    displayPosn[i].w = 0;
    displayPosn[i].h = 0;
  }  

  oldScreenWidth = 0;
  oldScreenHeight = 0;

  closeEncWindow = FALSE;
  closeRecWindow = FALSE;
  
  screen            = NULL;
  displayIsShutDown = FALSE;

  nextEncFrame = NULL;
  nextRcvFrame = NULL;
  videoPIP = _videoPIP;
  Resume();
}


PSDLDisplayThread::~PSDLDisplayThread()
{
}


void PSDLDisplayThread::Main()
{
  // initialise the SDL library
  if (::SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
    PTRACE(0, "Couldn't initialize SDL: " << ::SDL_GetError());
    return;      
  }

#ifdef _WIN32
  SDL_SetModuleHandle(GetModuleHandle(NULL));
#endif

  PSDLVideoFrame * frame;

  PThread::Current()->SetPriority(LowestPriority);
  PTRACE(3, "PSDL\tMain loop is underway, with SDL screen initialised");

  for (;;) {

    // wait for a new command to be given
    commandSync.Wait();

    // if to terminate, do it now
    if (!IsOpen())
      break;

    // output next frame
    WriteOutDisplay();

    frame = GetNextFrame(TRUE);
    Redraw(TRUE, frame);
    if (frame != NULL)
      delete frame;
    
    ProcessSDLEvents();

    frame = GetNextFrame(FALSE);
    Redraw(FALSE, frame);
    if (frame != NULL)
      delete frame;
    
    ProcessSDLEvents();
    
    if ((closeRecWindow && closeEncWindow) ||
	(closeEncWindow && (overlay[0] == NULL)) ||
	(closeRecWindow && (overlay[1] == NULL))) {
      CloseWindow(FALSE);     
      CloseWindow(TRUE);
      ProcessSDLEvents();
    }
  }
  
  CloseWindow(TRUE);
  CloseWindow(FALSE);
  ::SDL_Quit();
  PTRACE(3, "PSDL\tEnd of sdl display loop");
}


BOOL PSDLDisplayThread::AddFrame(PSDLVideoFrame *newFrame, BOOL isEncoding)
{
  PTRACE(3, "PSDL\tAddFrame runs here for frame " << *newFrame  << " " << GetDirName(isEncoding));

  PWaitAndSignal m(mutex);

  if (!IsOpen()) { //This frame not wanted, as the display thread is closed.
    PTRACE(2, "PSDL\tDelete unwanted " << GetDirName(isEncoding) << " frame");
    delete newFrame;
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

  commandSync.Signal();
  return TRUE;
}


BOOL PSDLDisplayThread::IsOpen()
{
  return threadRunning;
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
  PTRACE(3, "PSDL\tRequesting SDL thread termination");

  PWaitAndSignal m(mutex);
  // delete any pending frames
  if (nextEncFrame != NULL)
    delete nextEncFrame;
  if (nextRcvFrame != NULL)
    delete nextRcvFrame;
  nextEncFrame = NULL;
  nextRcvFrame = NULL;

  // now signal the thread to finish
  threadRunning = FALSE;
  commandSync.Signal();
}


void PSDLDisplayThread::RequestCloseWindow(BOOL isEncoding)
{
  PTRACE(3, "PSDL\tRequest: Close window " << GetDirName(isEncoding) << " video");

  PWaitAndSignal m(mutex);

  if (isEncoding)
    closeEncWindow = TRUE;
  else
    closeRecWindow = TRUE;

  commandSync.Signal();
  PTRACE(3, "PSDL\tRequest: Close window " << GetDirName(isEncoding) << " video Finished");
}


void PSDLDisplayThread::RequestOpenWindow(BOOL isEncoding)
{
  PTRACE(3, "PSDL\tRequest: Open window " << GetDirName(isEncoding) << " video");

  PWaitAndSignal m(mutex);

  if (isEncoding)
    closeEncWindow = FALSE;
  else
    closeRecWindow = FALSE;
}


PSDLVideoFrame * PSDLDisplayThread::GetNextFrame(BOOL isEncoding)
{
  PSDLVideoFrame *res;
  PWaitAndSignal m(mutex);

  if (!IsOpen())
    return FALSE;

  PSDLVideoFrame **frameQ;

  if (isEncoding) 
    frameQ = &nextEncFrame;
  else
    frameQ = &nextRcvFrame;

  res = *frameQ;
  *frameQ = NULL;

  return res;
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

  screen = ::SDL_SetVideoMode(newWidth, newHeight, 0, SDL_SWSURFACE | SDL_RESIZABLE );
  
  if (!ScreenIsOpen()) {
    PTRACE(0,"Could not open screen to display window: " << ::SDL_GetError());
    ::SDL_Quit();
    return FALSE;
  }
  
  PTRACE(3,"PSDL\tSuccessfully resize a SDL screen. New size= " << newWidth << "x" << newHeight);
  return TRUE;
}


void PSDLDisplayThread::InitDisplayPosn()
{
  PTRACE(6, "PSDL\tInitDisplayPosition now");

  SDL_Surface * currentSurface = ::SDL_GetVideoSurface();
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

    ::SDL_FreeSurface(screen);
    screen = NULL;
    oldScreenWidth = 0;
    oldScreenHeight = 0;
    ::SDL_Quit();
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
    ::SDL_FreeYUVOverlay(overlay[dispIndex]);

  if ((w == 0) || (h == 0)) { //An overlay of size 0x0 is meaningless.
    overlay[dispIndex]= NULL;
    PTRACE(6, "PSDL\tOverlay of null hsize has been created");
    return TRUE;
  }
  
  overlay[dispIndex] = ::SDL_CreateYUVOverlay(w, h, SDL_IYUV_OVERLAY, screen);
 
  if (overlay[dispIndex] == NULL) {
    PTRACE(1, "PSDL\t Could not open overlay to display window: " << ::SDL_GetError());
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
  PTRACE(3, "PSDL\tClose window " << dispIndex << " " << GetDirName(isEncoding) << " video");

  if (!(width[dispIndex] || height[dispIndex])) {
    PTRACE(3, "PSDL\tWindow " << GetDirName(isEncoding) << " video has zero width, and zero height");
    return;
  }

  if(overlay[dispIndex] != NULL) {
    PTRACE(3, "PSDL\tClose the overlay for window " << dispIndex << " " << GetDirName(isEncoding) << " video");
    ::SDL_FreeYUVOverlay(overlay[dispIndex]);
    overlay[dispIndex] = NULL;    
  }
  
  width[dispIndex] = 0;
  height[dispIndex] = 0;
  displayPosn[dispIndex].w = 0;
  displayPosn[dispIndex].h = 0;

  if(overlay[1-dispIndex] == NULL) {
    PTRACE(3, "PSDL\tClose screen as both overlays are NULL");
    CloseScreen();
  } else {
    PTRACE(3, "PSDL\t Resize screen as other overlay is non null, " 
	   << displayPosn[1-dispIndex].w << "x" << displayPosn[1-dispIndex].h);
    ResizeScreen(displayPosn[1-dispIndex].w, displayPosn[1-dispIndex].h);
  }
}


void PSDLDisplayThread::ProcessSDLEvents(void) 
{
  if (!ScreenIsOpen()) {
    PTRACE(6, "PSDL\t Screen not open, so dont process events");
    return;
  }

  SDL_Event event;  
  while (::SDL_PollEvent(&event)) {
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

  PTRACE(6, "PSDL\tRedraw starts now. Window " << dispIndex 
	 << " " << GetDirName(isEncoding) << " video");  
        
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
 
#if 0
  if (overlay[1 - dispIndex] == NULL) {
    PTRACE(6, "PSDL\tRedraw end prematurely, other overlay undefined");
    return TRUE;       //Again, we lost the overlay, exit quickly.
  }
#endif

  unsigned char * base = frame->GetDataPointer();
  ::SDL_LockYUVOverlay(yuvOverlay);

  PINDEX pixelsFrame = yuvOverlay->w * yuvOverlay->h;
  PINDEX pixelsQuartFrame = pixelsFrame >> 2;
  memcpy(yuvOverlay->pixels[0], base,                                  pixelsFrame);
  memcpy(yuvOverlay->pixels[1], base + pixelsFrame,                    pixelsQuartFrame);
  memcpy(yuvOverlay->pixels[2], base + pixelsFrame + pixelsQuartFrame, pixelsQuartFrame);

  ::SDL_UnlockYUVOverlay(yuvOverlay);

  ::SDL_DisplayYUVOverlay(yuvOverlay, displayPosn + dispIndex );    

  PTRACE(6, "PSDL\tFinish the redraw for window " << GetDirName(isEncoding));

  return TRUE;  
}


void PSDLDisplayThread::WriteOutDisplay(void)
{
#ifdef PTRACING
  PINDEX i;
  for(i = 0; i < 2; i++) {
    PTRACE(6, "PSDL\t" << i << " display x,y=" << displayPosn[i].x << " " << displayPosn[i].y );
  
    PTRACE(6, "PSDL\t" << i << " display w,h=" << displayPosn[i].w << " " << displayPosn[i].h );
  
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


#endif // P_SDL


// End of file ////////////////////////////////////////////////////////////////
