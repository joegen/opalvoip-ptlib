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
 */

#ifdef __GNUC__
#pragma implementation "vsdl.h"
#endif

#include <ptlib.h>

#if P_SDL

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/vsdl.h>
#include <ptlib/vconvert.h>

#define new PNEW
#define PTraceModule() "SDL"


extern "C" {
  #include <SDL.h>
};

#ifdef _MSC_VER
  #pragma comment(lib, P_SDL_LIBRARY)
#endif


PCREATE_VIDOUTPUT_PLUGIN_EX(SDL,

  virtual const char * GetFriendlyName() const
  {
    return "Simple DirectMedia Layer Video Output";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    return deviceName.NumCompare(GetServiceName()) == PObject::EqualTo;
  }
);


///////////////////////////////////////////////////////////////////////

class PSDL_System : public PMutex
{
  public:
    static PSDL_System & GetInstance()
    {
      static PSDL_System instance;
      return instance;
    }


    enum UserEvents {
      e_Open,
      e_Close,
      e_SetFrameSize,
      e_SetFrameData
    };


    void Run()
    {
      if (m_thread == NULL) {
        PTRACE(3, "Shutting down SDL thread.");
        m_thread = new PThreadObj<PSDL_System>(*this, &PSDL_System::MainLoop, true, "SDL");
        m_started.Wait();
      }
    }


  private:
    PThread     * m_thread;
    PSyncPoint    m_started;
  
    PSyncQueue<SDL_Event> m_queue;

    typedef std::list<PVideoOutputDevice_SDL *> DeviceList;
    DeviceList m_devices;

    PSDL_System()
      : m_thread(NULL)
    {
    }

    ~PSDL_System()
    {
      if (m_thread == NULL)
        return;
      
      m_queue.Close(true);
      m_thread->WaitForTermination();
      delete m_thread;
    }

    static int AddToLocalQueue(void * userdata, SDL_Event * event)
    {
      reinterpret_cast<PSDL_System *>(userdata)->m_queue.Enqueue(*event);
      return 1;
    }
  
  
    virtual void MainLoop()
    {
#if PTRACING
      PTRACE(4, "Start of event thread");

      SDL_version hdrVer, binVer;
      SDL_VERSION(&hdrVer);
      SDL_GetVersion(&binVer);
      PTRACE(3, "Compiled version: "
             << (unsigned)hdrVer.major << '.' << (unsigned)hdrVer.minor << '.' << (unsigned)hdrVer.patch
             << "  Run-Time version: "
             << (unsigned)binVer.major << '.' << (unsigned)binVer.minor << '.' << (unsigned)binVer.patch);
#endif

      // initialise the SDL library
      int err = ::SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS);
      if (err != 0) {
        PTRACE(1, "Couldn't initialize SDL: error=" << err << ' ' << ::SDL_GetError());
        return;
      }

      SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
      
#ifdef _WIN32
      SDL_SetModuleHandle(GetModuleHandle(NULL));
#endif

      // We need to redirect events to our own queue as on some platforms (Mac OS-X)
      // this needs to be in the "main thread", and this is not always practical.
      SDL_SetEventFilter(&PSDL_System::AddToLocalQueue, this);
      
      m_started.Signal();

      PTRACE(4, "Starting main event loop");
      SDL_Event sdlEvent;
      while (m_queue.Dequeue(sdlEvent) && sdlEvent.type != SDL_QUIT)
        HandleEvent(sdlEvent);

      PTRACE(3, "Quitting SDL");
      for (DeviceList::iterator it = m_devices.begin(); it != m_devices.end(); ++it)
        (*it)->InternalClose();
      
      m_devices.clear();
      
      ::SDL_Quit();
      m_thread = NULL;

      PTRACE(4, "End of event thread");
    }


    void HandleEvent(SDL_Event & sdlEvent)
    {
      PWaitAndSignal mutex(*this);

      switch (sdlEvent.type) {
        case SDL_USEREVENT :
        {
          PVideoOutputDevice_SDL * device = reinterpret_cast<PVideoOutputDevice_SDL *>(sdlEvent.user.data1);
          switch (sdlEvent.user.code) {
            case e_Open :
              if (device->InternalOpen())
                m_devices.push_back(device);
              break;

            case e_Close :
              device->InternalClose();
              m_devices.remove(device);
              break;

            case e_SetFrameSize :
              device->InternalSetFrameSize();
              break;

            case e_SetFrameData :
              device->InternalSetFrameData();
              break;

            default :
              PTRACE(5, "Unhandled user event " << sdlEvent.user.code);
          }
          break;
        }

        case SDL_WINDOWEVENT :
          switch (sdlEvent.window.event) {
            case SDL_WINDOWEVENT_RESIZED :
              PTRACE(4, "Resize window to " << sdlEvent.window.data1 << " x " << sdlEvent.window.data2 << " on " << sdlEvent.window.windowID);
              break;
              
            default :
              PTRACE(5, "Unhandled windows event " << (unsigned)sdlEvent.window.event);
          }
          break;
          
        default :
          PTRACE(5, "Unhandled event " << (unsigned)sdlEvent.type);
      }
    }
};


///////////////////////////////////////////////////////////////////////

PVideoOutputDevice_SDL::PVideoOutputDevice_SDL()
  : m_window(NULL)
  , m_renderer(NULL)
  , m_texture(NULL)
{
  m_colourFormat = PVideoFrameInfo::YUV420P();
  PTRACE(5, "Constructed.");
}


PVideoOutputDevice_SDL::~PVideoOutputDevice_SDL()
{ 
  Close();
  PTRACE(5, "Destroyed.");
}


PStringArray PVideoOutputDevice_SDL::GetOutputDeviceNames()
{
  return PPlugin_PVideoOutputDevice_SDL::ServiceName();
}


PStringArray PVideoOutputDevice_SDL::GetDeviceNames() const
{
  return GetOutputDeviceNames();
}


PBoolean PVideoOutputDevice_SDL::Open(const PString & name, PBoolean /*startImmediate*/)
{
  Close();

  m_deviceName = name;

  PSDL_System::GetInstance().Run();
  PostEvent(PSDL_System::e_Open, true);
  return IsOpen();
}


bool PVideoOutputDevice_SDL::InternalOpen()
{
  PWaitAndSignal sync(m_operationComplete, false);
  
  int x = SDL_WINDOWPOS_UNDEFINED;
  int y = SDL_WINDOWPOS_UNDEFINED;
  PINDEX x_pos = m_deviceName.Find("X=");
  PINDEX y_pos = m_deviceName.Find("Y=");
  if (x_pos != P_MAX_INDEX && y_pos != P_MAX_INDEX) {
    x = atoi(&m_deviceName[x_pos+2]);
    y = atoi(&m_deviceName[y_pos+2]);
  }
  
  PString title = "Video Output";
  PINDEX pos = m_deviceName.Find("TITLE=\"");
  if (pos != P_MAX_INDEX) {
    pos += 6;
    PINDEX quote = m_deviceName.FindLast('"');
    title = PString(PString::Literal, m_deviceName(pos, quote > pos ? quote : P_MAX_INDEX));
  }
  
  m_window = SDL_CreateWindow(title, x, y, GetFrameWidth(), GetFrameHeight(), SDL_WINDOW_RESIZABLE);
  if (m_window == NULL) {
    PTRACE(1, "Couldn't create SDL window: " << ::SDL_GetError());
    return false;
  }
  
  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);
  if (m_renderer == NULL) {
    PTRACE(1, "Couldn't create SDL renderer: " << ::SDL_GetError());
    return false;
  }
  
  m_texture = SDL_CreateTexture(m_renderer,
                                SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING,
                                GetFrameWidth(), GetFrameHeight());
  if (m_texture == NULL) {
    PTRACE(1, "Couldn't create SDL texture: " << ::SDL_GetError());
    return false;
  }
  
  PTRACE(3, "Opened SDL device: " << m_deviceName);
  return true;
}


PBoolean PVideoOutputDevice_SDL::IsOpen()
{
  return m_texture != NULL;
}


PBoolean PVideoOutputDevice_SDL::Close()
{
  if (!IsOpen())
    return false;
  
  PTRACE(3, "Closing SDL device: " << m_deviceName);
  PostEvent(PSDL_System::e_Close, true);
  return true;
}


void PVideoOutputDevice_SDL::InternalClose()
{
  SDL_DestroyTexture(m_texture);   m_texture  = NULL;
  SDL_DestroyRenderer(m_renderer); m_renderer = NULL;
  SDL_DestroyWindow(m_window);     m_window   = NULL;
  m_operationComplete.Signal();
}


PBoolean PVideoOutputDevice_SDL::SetColourFormat(const PString & colourFormat)
{
  if (colourFormat *= PVideoFrameInfo::YUV420P())
    return PVideoOutputDevice::SetColourFormat(colourFormat);

  return false;
}


PBoolean PVideoOutputDevice_SDL::SetFrameSize(unsigned width, unsigned height)
{
  if (width == m_frameWidth && height == m_frameHeight)
    return true;

  if (!PVideoOutputDevice::SetFrameSize(width, height))
    return false;

  if (IsOpen())
    PostEvent(PSDL_System::e_SetFrameSize, true);

  return true;
}


void PVideoOutputDevice_SDL::InternalSetFrameSize()
{
  PWaitAndSignal sync(m_operationComplete, false);
  
  if (m_renderer == NULL)
    return;
  
  if (m_texture != NULL)
    SDL_DestroyTexture(m_texture);

  m_texture = SDL_CreateTexture(m_renderer,
                                SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING,
                                GetFrameWidth(), GetFrameHeight());
  PTRACE_IF(1, m_texture == NULL, "Couldn't create SDL texture: " << ::SDL_GetError());
}


PINDEX PVideoOutputDevice_SDL::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat));
}


PBoolean PVideoOutputDevice_SDL::SetFrameData(unsigned x, unsigned y,
                                          unsigned width, unsigned height,
                                          const BYTE * data,
                                          PBoolean endFrame) 
{
  if (!IsOpen())
    return false;

  if (x != 0 || y != 0 || width != m_frameWidth || height != m_frameHeight || data == NULL || !endFrame)
    return false;

  PWaitAndSignal mutex(PSDL_System::GetInstance());

  SDL_UpdateTexture(m_texture, NULL, data, width);
  
  PostEvent(PSDL_System::e_SetFrameData, false);
  return true;
}


void PVideoOutputDevice_SDL::InternalSetFrameData()
{
  PWaitAndSignal sync(m_operationComplete, false);
  
  SDL_RenderClear(m_renderer);
  SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
  SDL_RenderPresent(m_renderer);
}


void PVideoOutputDevice_SDL::PostEvent(unsigned code, bool wait)
{
  SDL_Event sdlEvent;
  sdlEvent.type = SDL_USEREVENT;
  sdlEvent.user.code = code;
  sdlEvent.user.data1 = this;
  sdlEvent.user.data2 = NULL;
  if (::SDL_PushEvent(&sdlEvent) < 0) {
    PTRACE(1, "Couldn't post user event " << (unsigned)sdlEvent.user.code << ": " << ::SDL_GetError());
    return;
  }

  PTRACE(5, "Posted user event " << (unsigned)sdlEvent.user.code);
  if (wait)
    PAssert(m_operationComplete.Wait(10000),
            PSTRSTRM("Couldn't process user event " << (unsigned)sdlEvent.user.code));
}
#endif // P_SDL
