/*
 * macvideo.mm
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2013 Equivalence Pty. Ltd.
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
 * Contributor(s):
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#if P_VIDEO

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#ifdef BOOL
#undef BOOL
#endif

#include <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLock.h>
#import <AVFoundation/AVFoundation.h>

#define PTraceModule() "MacVideo"
#define PTRACE_DETAILED(...) PTRACE(__VA_ARGS__)


class PLocalMemoryPool
{
  NSAutoreleasePool * m_pool;
public:
   PLocalMemoryPool() {  m_pool = [[NSAutoreleasePool alloc] init]; }
  ~PLocalMemoryPool() { [m_pool drain]; }
};


@interface PVideoInputDevice_MacFrame : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
  CMSampleBufferRef m_videoFrame;
  PSyncPoint       m_grabbed;
  bool             m_frameAvailable;
}

@end


@implementation PVideoInputDevice_MacFrame

- (id)init
{
  if ((self = [super init]))
  {
    m_frameAvailable = false;
  }
  
  return self;
}


// AVCapture delegate method, called when a frame has been loaded by the camera
- (void)captureOutput:        (AVCaptureOutput *)captureOutput
        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:       (AVCaptureConnection *)connection
{
  PTRACE_DETAILED(5, "Frame received: m_frameAvailable=" << m_frameAvailable);

  // If we have not processed previous frame, ignore this one
  if (m_frameAvailable)
    return;

  // The Apple docs state that this action must be synchronized
  // as this method will be run on another thread
  @synchronized (self) {
    m_videoFrame = sampleBuffer;
    m_frameAvailable = true;
    m_grabbed.Signal();
  }
}


- (void)stop
{
  PTRACE(4, "Breaking grab block");
  m_grabbed.Signal();
}


- (void)waitFrame
{
  PTRACE_DETAILED(5, "Waiting");
  m_grabbed.Wait();
}


- (void)grabFrame:(BYTE *)buffer withWidth:(unsigned)expectedWidth andHeight:(unsigned)expectedHeight
{
  if (!m_frameAvailable)
    m_grabbed.Wait();

  CVPixelBufferRef pixels;
  
  @synchronized (self){
    pixels = CMSampleBufferGetImageBuffer(m_videoFrame);
    m_frameAvailable = false;
  }

  unsigned actualWidth = CVPixelBufferGetWidth(pixels);
  unsigned actualHeight = CVPixelBufferGetHeight(pixels);
  
  PTRACE_IF(3, expectedWidth != actualWidth || expectedHeight != actualHeight,
            "Not grabbing the size we expected: "
            << actualWidth << 'x' << actualHeight << "!=" << expectedWidth << 'x' << expectedHeight);

  PTRACE_DETAILED(5, "Frame grabbed: " << actualWidth << 'x' << actualHeight);

  if (actualWidth > expectedWidth)
    actualWidth = expectedWidth;
  if (actualHeight > expectedHeight)
    actualHeight = expectedHeight;
  
  CVPixelBufferLockBaseAddress(pixels, 0);

  const uint8_t * src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 0);
  size_t rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 0);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }

  actualWidth /= 2;
  actualHeight /= 2;
  expectedWidth /= 2;
  expectedHeight /= 2;
  
  src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 1);
  rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 1);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }
  
  src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 2);
  rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 2);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }
    
  CVPixelBufferUnlockBaseAddress(pixels, 0);
  CVBufferRelease(pixels);
}

@end



class PVideoInputDevice_Mac : public PVideoInputDevice
{
    PCLASSINFO(PVideoInputDevice_Mac, PVideoInputDevice);
  public:
    PVideoInputDevice_Mac();
    ~PVideoInputDevice_Mac();

    PBoolean Open(const PString & deviceName, PBoolean startImmediate);
    PBoolean IsOpen() ;
    PBoolean Close();
    PBoolean Start();
    PBoolean Stop();
    PBoolean IsCapturing();
    static PStringArray GetInputDeviceNames();
    virtual PStringArray GetDeviceNames() const;
    static bool GetDeviceCapabilities(const PString &, Capabilities *);
    virtual bool GetDeviceCapabilities(Capabilities * caps) { return GetDeviceCapabilities(PString::Empty(), caps); }
    virtual PINDEX GetMaxFrameBytes();
    virtual PBoolean GetFrameData(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean SetColourFormat(const PString & colourFormat);
    virtual PBoolean SetFrameRate(unsigned rate);
    virtual PBoolean SetFrameSize(unsigned width, unsigned height);

  protected:
    AVCaptureSession * m_session;
    AVCaptureDevice * m_device;
    AVCaptureDeviceInput * m_captureInput;
    AVCaptureVideoDataOutput * m_captureOutput;
    PVideoInputDevice_MacFrame * m_captureFrame;
  
    PINDEX m_frameSizeBytes;
    PReadWriteMutex m_mutex;
    PBYTEArray m_tempFrame;
};

PCREATE_VIDINPUT_PLUGIN(Mac);


PVideoInputDevice_Mac::PVideoInputDevice_Mac()
  : m_session(nil)
  , m_device(nil)
  , m_captureInput(nil)
  , m_captureOutput(nil)
  , m_captureFrame(nil)
{
  m_colourFormat = "YUV420P";
  m_frameWidth = 640;
  m_frameHeight = 480;
  m_frameSizeBytes = CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat);
  PTRACE(5, "Constructed.");
}


PVideoInputDevice_Mac::~PVideoInputDevice_Mac()
{
  Close();
  PTRACE(5, "Destroyed.");
}


PBoolean PVideoInputDevice_Mac::Open(const PString & devName, PBoolean startImmediate)
{
  Close();
  
  PLocalMemoryPool localPool;
  NSError *error = nil;
  
  PWriteWaitAndSignal mutex(m_mutex);
  
  m_session = [[AVCaptureSession alloc] init];

  if (devName.IsEmpty() || (devName *= "default"))
    m_device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
  else {
    NSString * name = [NSString stringWithUTF8String:devName];
    NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice * device in devices) {
      if ([[device localizedName] caseInsensitiveCompare:name] == 0) {
        m_device = [AVCaptureDevice deviceWithUniqueID:[device uniqueID]];
        break;
      }
    }
  }

  if (m_device == nil || error != nil) {
    PTRACE(2, "Could not open device \"" << devName << "\": " << [error localizedDescription]);
    return false;
  }

  [m_device setActiveVideoMinFrameDuration:CMTimeMake(1, m_frameRate)];
  [m_device setActiveVideoMaxFrameDuration:CMTimeMake(1, m_frameRate)];

  m_captureInput = [AVCaptureDeviceInput deviceInputWithDevice:m_device error:&error];
  if (error != nil) {
    PTRACE(2, "Could not get input device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  if (![m_session canAddInput:m_captureInput]) {
    PTRACE(2, "Could not add input device \"" << devName << '"');
    return false;
  }

  [m_session addInput:m_captureInput];
  
  m_captureOutput = [[AVCaptureVideoDataOutput alloc] init];

  m_captureOutput.alwaysDiscardsLateVideoFrames = NO;
  m_captureOutput.videoSettings =
      [NSDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithInt:m_frameWidth],
                  (id)kCVPixelBufferWidthKey,
          [NSNumber numberWithInt:m_frameHeight],
                  (id)kCVPixelBufferHeightKey,
          [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8Planar],
                  (id)kCVPixelBufferPixelFormatTypeKey,
          nil
      ];

  [m_session addOutput:m_captureOutput];
  
  m_captureFrame = [[PVideoInputDevice_MacFrame alloc] init];
  dispatch_queue_t captureQueue = dispatch_queue_create( "captureQueue", DISPATCH_QUEUE_SERIAL );
  [m_captureOutput setSampleBufferDelegate:m_captureFrame queue:captureQueue];
  
  m_deviceName = devName;
  m_frameSizeBytes = CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat);

  PTRACE(3, "Opened \"" << devName << "\""
            " res=" << m_frameWidth << 'x' << m_frameHeight << '@' << m_frameRate);
  
  if (startImmediate)
    return Start();
  
  return true;
}


PBoolean PVideoInputDevice_Mac::IsOpen() 
{
  return m_captureFrame != nil;
}


PBoolean PVideoInputDevice_Mac::Close()
{
  PTRACE_IF(4, IsOpen(), "Closing \"" << m_deviceName << '"');
  
  Stop();

  m_mutex.StartWrite();

  PLocalMemoryPool localPool;

  if (m_captureInput != nil) {
    [m_session removeInput:m_captureInput];
    [m_captureInput release];
    m_captureInput = nil;
  }

  if (m_captureOutput != nil) {
    [m_session removeOutput:m_captureOutput];
    [m_captureOutput setSampleBufferDelegate:nil queue:nil];
    [m_captureOutput release];
    m_captureOutput = nil;
  }

  if (m_captureFrame != nil) {
    [m_captureFrame release];
    m_captureFrame = nil;
  }
  
  if (m_device != nil) {
    [m_device release];
    m_device = nil;
  }

  if (m_session != nil) {
    [m_session release];
    m_session = nil;
  }

  m_mutex.EndWrite();

  PTRACE(5, "Closed \"" << m_deviceName << '"');
  return true;
}


PBoolean PVideoInputDevice_Mac::Start()
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsOpen())
    return false;

  PLocalMemoryPool localPool;
  
  if ([m_session isRunning])
    return true;

  PTRACE(3, "Starting \"" << m_deviceName << '"');
  [m_session startRunning];
  for (int retry = 0; retry < 50; ++retry) {
    if ([m_session isRunning])
      return true;
    PThread::Sleep(20);
  }
  
  [m_session stopRunning];
  PTRACE(2, "Could not start \"" << m_deviceName << '"');
  return false;
}


PBoolean PVideoInputDevice_Mac::Stop()
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsOpen())
    return false;
  
  PLocalMemoryPool localPool;
  
  PTRACE(3, "Stopping \"" << m_deviceName << '"');
  [m_session stopRunning];
  [m_captureFrame stop];
  
  for (int retry = 0; retry < 50; ++retry) {
    if (![m_session isRunning]) {
      PTRACE(5, "Stopped \"" << m_deviceName << '"');
      return true;
    }
    PThread::Sleep(20);
  }
  
  PTRACE(2, "Could not stop \"" << m_deviceName << '"');
  return false;
}


PBoolean PVideoInputDevice_Mac::IsCapturing()
{
  PReadWaitAndSignal mutex(m_mutex);
  return IsOpen() && [m_session isRunning];
}


PStringArray PVideoInputDevice_Mac::GetInputDeviceNames()
{
  PVideoInputDevice_Mac dev;
  return dev.GetDeviceNames();
}


PStringArray PVideoInputDevice_Mac::GetDeviceNames() const
{
  PLocalMemoryPool localPool;
  
  PStringArray names;

  NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
  for (AVCaptureDevice * device in devices)
    names += [[device localizedName] cStringUsingEncoding:NSUTF8StringEncoding];

  return names;
}


PBoolean PVideoInputDevice_Mac::SetColourFormat(const PString & newFormat)
{
  return newFormat == "YUV420P";
}


PBoolean PVideoInputDevice_Mac::SetFrameRate(unsigned rate)
{
  if (rate < 1)
    rate = 1;
  else if (rate > 50)
    rate = 50;

  if (rate == m_frameRate)
    return true;

  if (!PVideoDevice::SetFrameRate(rate))
    return false;

  if (!IsOpen())
    return true;
  
  bool restart = IsCapturing();
  if (restart)
    Stop();
  
  PTRACE(3, "Setting frame rate of \"" << m_deviceName << "\" to " << rate);
  
  PLocalMemoryPool localPool;
  
  m_mutex.StartRead();
  
  [m_device setActiveVideoMinFrameDuration:CMTimeMake(1, m_frameRate)];
  [m_device setActiveVideoMaxFrameDuration:CMTimeMake(1, m_frameRate)];

  m_mutex.EndRead();
  
  if (restart)
    return Start();
  
  return true;
}


PBoolean PVideoInputDevice_Mac::SetFrameSize(unsigned width, unsigned height)
{
  if (width == m_frameWidth && height == m_frameHeight)
    return true;

  // Searched and searched but cannot figure out how to do this programmatically.
  if (!((width == 160 && height == 120) ||
        (width == 320 && height == 240) ||
        (width == 640 && height == 480)))
    return false;

  if (!PVideoDevice::SetFrameSize(width, height))
    return false;

  m_frameSizeBytes = CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat);
  
  if (IsOpen())
    return Open(m_deviceName, IsCapturing());
  
  return true;
}


bool PVideoInputDevice_Mac::GetDeviceCapabilities(const PString &, Capabilities * caps)
{
  PVideoFrameInfo frameInfo;
  frameInfo.SetFrameSize(160, 120);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(320, 240);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(640, 480);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(176, 144);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(352, 288);
  caps->m_frameSizes.push_back(frameInfo);
  return true;
}


PINDEX PVideoInputDevice_Mac::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(m_frameSizeBytes);
}


PBoolean PVideoInputDevice_Mac::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsCapturing())
    return false;
  
  [m_captureFrame waitFrame];
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_Mac::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  PTRACE_DETAILED(5, "Get frame, converter=" << m_converter);
  
  if (!IsCapturing())
    return false;

  PReadWaitAndSignal mutex(m_mutex);

  if (m_converter != NULL) {
    [m_captureFrame grabFrame:m_tempFrame.GetPointer(m_frameSizeBytes) withWidth:m_frameWidth andHeight:m_frameHeight];
    if (!m_converter->Convert(m_tempFrame, destFrame, bytesReturned))
      return false;
  }
  else {
    [m_captureFrame grabFrame:destFrame withWidth:m_frameWidth andHeight:m_frameHeight];
    if (bytesReturned != NULL)
      *bytesReturned = m_frameSizeBytes;
  }
  
  return true;
}


#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////

