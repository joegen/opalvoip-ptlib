/*
 * vsdl.h
 *
 * Classes to support video output via SDL
 *
 * Copyright (c) 1999-2000 Equivalence Pty. Ltd.
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
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Derek J Smithies (derek@indranet.co.nz)
 *
 * $Log: vsdl.h,v $
 * Revision 1.8  2005/07/13 12:50:07  csoutheren
 * Backported changes from isvo branch
 *
 * Revision 1.7.8.1  2005/04/25 13:27:26  shorne
 * Added support for capture SDL output to redirect to existing surface
 *
 * Revision 1.7  2004/05/31 01:26:58  dereksmithies
 * Fix the "no newline at end of file" warning.
 *
 * Revision 1.6  2004/05/27 04:47:05  csoutheren
 * Added include guard to file
 *
 * Revision 1.5  2003/12/12 05:11:56  rogerhardiman
 * Add SDL support on FreeBSD. Header files live in SDL11 directory
 *
 * Revision 1.4  2003/05/17 03:20:48  rjongbloed
 * Removed need to do strange things with main() function.
 *
 * Revision 1.3  2003/04/28 14:29:45  craigs
 * Started rearranging code
 *
 * Revision 1.2  2003/04/28 08:44:42  craigs
 * Fixed problem with include on linux
 *
 * Revision 1.1  2003/04/28 07:04:20  craigs
 * Initial version from ohphone
 *
 * Revision 1.8  2003/03/21 00:47:47  dereks
 * Remove surplus PTRACE statements.
 *
 * Revision 1.7  2003/03/20 23:50:41  dereks
 * Fixups resulting from the new PVideoOutputDevice class code.
 *
 * Revision 1.6  2002/12/03 21:45:05  dereks
 * Fixes from Walter Whitlock to cure warnings about u_chars. Thanks!
 *
 * Revision 1.5  2002/06/27 02:17:40  robertj
 * Renamed video format 411 to the correct 420P, thanks Mark Cooke
 *
 * Revision 1.4  2002/04/29 03:51:55  dereks
 * Documentation tidy up. Thanks to Walter Whitlock.
 *
 * Revision 1.3  2002/04/26 03:33:32  dereks
 * Major upgrade. All calls to SDL library are now done by one thread.
 *
 * Revision 1.2  2001/05/25 01:14:44  dereks
 * Alter SetFrameSize & OpenWindo to use unsigned variables. Change type of
 * other variables to eliminate compiler warnings.
 *
 * Revision 1.1  2001/03/07 01:47:45  dereks
 * Initial release of SDL (Simple DirectMedia Layer, a cross-platform multimedia library),
 * a video library code.
 *
 *
 */

#ifndef _PVSDL
#define _PVSDL

#if P_SDL

#include <ptlib.h>
#if defined(P_FREEBSD)
#include <SDL11/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#undef main

class PSDLVideoFrame : public PObject
{
    PCLASSINFO(PSDLVideoFrame, PObject);
  public:
    PSDLVideoFrame(
      unsigned newWidth,
      unsigned newHeight,
      Uint8 *data
    );
    PSDLVideoFrame(
      unsigned newWidth,
      unsigned newHeight,
      const void *_data
    );
  
    ~PSDLVideoFrame();
  
    unsigned GetWidth() { return width; }
    unsigned GetHeight() { return height; }
  
    Uint8 *GetDataPointer() { return data; }
  
    void PrintOn(ostream & str) const;
  
  protected:
    void Initialise(unsigned newWidth, unsigned newHeight, Uint8 *_data);
  
    unsigned width;
    unsigned height;
  
    Uint8 *data;
};


class PSDLDisplayThread : public PThread
{
    PCLASSINFO(PSDLDisplayThread, PThread);
  public:
    PSDLDisplayThread(
      BOOL _videoPIP
    );
    ~PSDLDisplayThread();
  
    void Main();
  
    /** returns FALSE if the thread is closed, so cannot add frame.
    The frame is deleted by the thread - in all cases.*/
    BOOL AddFrame(PSDLVideoFrame *newFrame, BOOL isEncoding);
  
    BOOL IsOpen();
  
    virtual void Terminate();
    void RequestOpenWindow(BOOL isEncoding);
    void RequestCloseWindow(BOOL isEncoding);
  
  protected:
    BOOL ScreenIsOpen();
    BOOL DisplayIsShutDown();
    void CloseWindow(BOOL isEncoding);
  
    PSDLVideoFrame *GetNextFrame(BOOL isEncoding);
  
    virtual BOOL ResizeScreen(unsigned newWidth, unsigned newHeight);
    void InitDisplayPosn();
    void InitDisplayPosn(unsigned w, unsigned h);
    virtual void CloseScreen();
    BOOL CreateOverlay(BOOL isEncoding);
    BOOL SetOverlaySize (BOOL isEncoding, unsigned _width, unsigned _height);
  
    void WriteOutDisplay();
  
    unsigned GetDisplayIndex(BOOL isEncoding);
  
    /**Store the height,widths etc for this classes window
    */
    BOOL SetFrameSize(BOOL isEncoding, unsigned _width, unsigned _height);
  
    /**Handles all events that occur in the SDL window (resize and quit)
    */
    void ProcessSDLEvents(void);
  
    BOOL Redraw(BOOL isEncoding, PSDLVideoFrame *frame);
  
    enum { RemoteIndex = 0 };
    enum { EncodeIndex = 1 };
  
    const char * GetDirName(BOOL isEncoding) 
      { return (isEncoding ? "local" : "remote"); }
  
    PMutex     mutex;  
    PSyncPoint commandSync;
    BOOL       threadRunning;
  
    SDL_Surface  *screen;
    SDL_Overlay  *overlay[2];
    SDL_Rect      displayPosn[2];
  
    unsigned   width[2];
    unsigned   height[2];
    unsigned   oldScreenWidth, oldScreenHeight;
  
    PString  remoteName;
    BOOL   displayIsShutDown;
    BOOL   videoPIP;
  
    BOOL  closeEncWindow;
    BOOL  closeRecWindow;
  
    PSDLVideoFrame *nextEncFrame;
    PSDLVideoFrame *nextRcvFrame;
};


/**Display data to the SDL screen.
  */
class PSDLVideoDevice : public PVideoOutputDevice
{
    PCLASSINFO(PSDLVideoDevice, PVideoOutputDevice);
  
  public:
    /**Constructor. Does not make a window.
      */
    PSDLVideoDevice(
      const PString & _remoteName,
      BOOL _isEncoding, 
      PSDLDisplayThread *_sdlThread
    );
  
      /**Destructor.  Closes window if necessary, (which initializes all variables)
    */
    ~PSDLVideoDevice();
  
    /**Open the device given the device name.
    */
    virtual BOOL Open(
      const PString & /*deviceName*/,   /// Device name to open
      BOOL /*startImmediate*/ = TRUE    /// Immediately start device
      ) { return TRUE; }
  
    /**Synonymous with the destructor.
    */
    BOOL Close();
  
    /**Global test function to determine if this video rendering
    class is open.*/
    BOOL IsOpen();
  
    unsigned GetFrameWidth() const { return width; }
  
    unsigned GetFrameHeight() const { return height; }
  
    /**Take a YUV420P format image, render it on the existing window. 
    If the window is not present, there is no rendering.
    */
    BOOL Redraw (const void *frame);
  
    /**Get a list of all of the drivers available.
    */
    virtual PStringList GetDeviceNames() const;
  
    /**Get the maximum frame size in bytes.
  
      Note a particular device may be able to provide variable length
      frames (eg motion JPEG) so will be the maximum size of all frames.
    */
    virtual PINDEX GetMaxFrameBytes()
      { return 352 * 288 * 3 * 2; }
  
    /**Set size of the window. Closes existing window, opens new window.
    */
    BOOL SetFrameSize (unsigned _width ,unsigned _height);
  
    virtual PString GetRemoteName() const
      { return remoteName ; }
  
    /**Name of remote computer, which is used for window title.
    */
    virtual void SetRemoteName(
      const PString & _remoteName
    ) { remoteName = _remoteName; }
  
    /**Specifies required number of bitplanes in the image. Does nothing.
    */
    void ForceDepth(int /*d*/) { }
  
  
    BOOL SetFrameData(
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height,
      const BYTE * data,
      BOOL endFrame = TRUE
    ) ;
  
      /**Indicate frame may be displayed.
    */
    BOOL EndFrame();
  
  private:
    BOOL     isEncoding;
    PString  remoteName;
    PSDLDisplayThread *sdlThread;
    unsigned     width, height;
};

#endif    // P_SDL

#endif

