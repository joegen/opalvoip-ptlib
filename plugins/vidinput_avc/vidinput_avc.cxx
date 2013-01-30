/*
 * This file is essentially a rewrite of video4dc1394.cxx
 * Check that one for more explanations
 *
 * A lot of code "borrowed" from
 * - dvgrab.c from libdv (http://libdv.sf.net/)
 * - kino (http://kino.schirmacher.de/)
 * - video4dc1394.cxx from ptlib
 * - ... probably others too
 *
 * The code is highly experimental.
 * You should expect any of :
 * - plenty of segfaults
 * - loss of performance
 * - not working at all for you
 * 
 * Known Bugs / Limitations / Room for improvement (feel free to patch/suggest)
 * - Colors are no good after a Convert 
 *   Can someone look at the code and tell me what I have to tell the convert function
 *   that my source colors are? I thought it is pure RGB24, but obviously not.
 *   Dumping the binary data directly to a PPM file works like a charm, though :-/
 * - Occasional segfaults (I think these are fixed, but don't be surprised if it works not)
 * - grabs first camera by default (not sure how to go about selection of cameras/ports)
 * - still haven't figured what the channel parameter in start_iso_rcv(handle,channel) means,
 *   but it seems that we are in for a long wait if channel != 63
 * - code depends on libavc1394 to figure out if a device is a camera
 *   I am not really sure there isn't a smarter way of going about it
 *   since capturing video only requires libraw1394 (for the moment)
 *   Maybe we can drop that dependency?
 * - Still not sure how to go about setting frame size.
 *   Resizing manually at the moment, since the frame size of a captured frame
 *   from an AVC camera is not settable.
 *   An AVC camera supports either NTSC (720x480) or PAL (720x576) resolution
 *   and the only way to check which one it is, seems to be to parse the header
 *   of a captured frame. Will wait for a suggestion on the proper way to handle it.
 * - bus resets not handled (yet?)
 * - Still not sure what to use the device name for (beats me :( )
 * - not sure if TRY_1394AVC and TRY_1394DC will not break something if used at the same time
 * - Overuse of PTRACE?
 * - I am not sure how most of the stuff works
 * - ... everything else
 *
 * Technical Notes
 * ------------------------------------------------------------
 *
 * Test Environment:
 * This module was tested against:
 * Hardware:
 *   AthlonXP 1800+
 *   Asus A7S333
 *   Sony DCR-TRV20 NTSC (http://www.sony.jp/products/Consumer/VD/DCR-TRV20/)
 *   Texas Instruments TSB12LV26 IEEE-1394 Controller 
 * Software:
 *   Linux vanilla kernel 2.4.20
 *   libraw1394 0.9.0
 *   libavc1394 0.4.1
 *   libdv 0.98
 *
 * Author: Georgi Georgiev <chutz@gg3.net>
 *
 */

#pragma implementation "vidinput_avc.h"

#include "vidinput_avc.h"

#ifndef RAW_BUFFER_SIZE
#define RAW_BUFFER_SIZE 512
#endif 

PCREATE_VIDINPUT_PLUGIN(1394AVC);

static PMutex mutex;
static PDictionary<PString, PString> *dico;
static u_int8_t raw_buffer[RAW_BUFFER_SIZE];
static BYTE const my_channel = 63;


raw1394_iso_disposition RawISO_receiver(raw1394handle_t handle,
 unsigned char* data,
 unsigned int length,
 unsigned char channel,
 unsigned char tag,
 unsigned char sy,
 unsigned int cycle,
 unsigned int dropped)
{
  if (length < RAW_BUFFER_SIZE) {
    *(u_int32_t *) raw_buffer = length;
    memcpy (raw_buffer + 4, data, length);
  }
  return RAW1394_ISO_OK;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoInput1394AVC

PVideoInputDevice_1394AVC::PVideoInputDevice_1394AVC()
{
  handle = NULL;
  is_capturing = false;
}

PVideoInputDevice_1394AVC::~PVideoInputDevice_1394AVC()
{
  Close();
}

PBoolean PVideoInputDevice_1394AVC::Open(const PString & devName, PBoolean startImmediate)
{
  PTRACE(3, "trying to open " << devName);

  if (IsOpen())
    Close();

  UseDMA = true; // FIXME: useful?

  handle = raw1394_new_handle();
  if (handle == NULL) {
    PTRACE(3, "No handle.");
    return false;
  }
  
  mutex.Wait();
  if(dico != NULL && sscanf((char *)dico->GetAt(devName), "%d", &port) == 1)
    ; // well, got it
  else
    port = 0;
  mutex.Signal();
  
  if(raw1394_set_port(handle, port) != 0) {
    PTRACE(3, "couldn't set the port");
    Close();
    return false;
  }
  
  frameWidth = CIFWidth;
  frameHeight = CIFHeight;
  colourFormat = "RGB24";
  
  deviceName = devName; // FIXME: looks useless
  
  if (!SetChannel(channelNumber)
      || !SetVideoFormat(videoFormat)) {
    PTRACE(3, "SetChannel() or SetVideoFormat() failed");
    Close();
    return false;
  }
  
  if (startImmediate && !Start()) {
    Close();
    return false;
  }
  
  PTRACE(3, "Successfully opened avc1394");
  return true;
}

PBoolean PVideoInputDevice_1394AVC::IsOpen() 
{
  return handle != NULL;
}

PBoolean PVideoInputDevice_1394AVC::Close()
{
  PTRACE(3, "Close()");
  if (IsOpen()) {
    if (IsCapturing())
      Stop();
    raw1394_destroy_handle(handle);
    handle = NULL;
    return true;
  }
  else
    return false;
}

PBoolean PVideoInputDevice_1394AVC::Start()
{
  if (!IsOpen()) return false;
  if (IsCapturing()) return true;

  if (raw1394_iso_recv_init(handle, RawISO_receiver, 2000, 1234, my_channel, RAW1394_DMA_DEFAULT, -1)) {
    PTRACE(3, "Cannot initialise for receiving on channel " << my_channel);	
    return false;
  }
  
  is_capturing = true;
  return true;
}

PBoolean PVideoInputDevice_1394AVC::Stop()
{
  if (IsCapturing()) {
    is_capturing = false;
    raw1394_iso_shutdown(handle);
    return true;
  }
  else
    return false;
}

PBoolean PVideoInputDevice_1394AVC::IsCapturing()
{
  return is_capturing;
}

PStringArray PVideoInputDevice_1394AVC::GetInputDeviceNames()
{
  PStringArray Result;
  raw1394handle_t hdl = NULL;
  
  hdl = raw1394_new_handle();
  
  if (hdl == NULL)
    return Result;
  
  // scan all nodes of all ports, check the real name of the device
  int nb_ports = raw1394_get_port_info(hdl, NULL, 0);
  for(int pt = 0; pt < nb_ports; pt++) {
    if (raw1394_set_port(hdl, pt) >= 0) {
      int nb_nodes = raw1394_get_nodecount(hdl);
      for (int nd = 0; nd < nb_nodes; nd++) {
        rom1394_directory dir;
        rom1394_get_directory(hdl, nd, &dir);
        if (rom1394_get_node_type(&dir) == ROM1394_NODE_TYPE_AVC) {
          PString ufname = (PString)dir.label;
          PString *devname = new PString(pt);
 	  if (ufname.IsEmpty ())
 	    ufname = "Nameless device";
          mutex.Wait();
          if (dico == NULL)
            dico = new PDictionary<PString, PString>;
          if (dico->Contains(ufname) && *dico->GetAt(ufname) != *devname) {
            PString altname = ufname+ " (2)";
            int i = 2;
            while(dico->Contains(altname) && *dico->GetAt(altname) != *devname) {
              i++;
              altname = ufname+ " ("+(PString)i+")";
            }
            dico->SetAt(altname, devname);
            Result.AppendString(altname);
          }
          else {
            dico->SetAt(ufname, devname);
            Result.AppendString(ufname);
          }
          mutex.Signal();
        }
      }
    }
  }
  
  raw1394_destroy_handle(hdl);
  return Result;
}

PBoolean PVideoInputDevice_1394AVC::SetVideoFormat(VideoFormat newFormat)
{
  // FIXME: isn't it inherited from PVideoDevice anyway?
  if (!PVideoDevice::SetVideoFormat(newFormat)) {
    PTRACE(3,"PVideoDevice::SetVideoFormat failed");
    return false;
  }
  else
    return true;
}

int PVideoInputDevice_1394AVC::GetNumChannels() 
{
  int Result;
  mutex.Wait();
  if(dico != NULL)
    Result = dico->GetSize();
  else
    Result = 0;

  mutex.Signal();
  return Result;
}

PBoolean PVideoInputDevice_1394AVC::SetChannel(int newChannel)
{
  if (PVideoDevice::SetChannel(newChannel) == false)
    return false;

  if(IsCapturing()) {
    Stop();
    Start();
  }
  
  return true;
}

PBoolean PVideoInputDevice_1394AVC::SetFrameRate(unsigned rate)
{
  return PVideoDevice::SetFrameRate(rate);
}

PBoolean PVideoInputDevice_1394AVC::GetFrameSizeLimits(unsigned & minWidth,
                                                   unsigned & minHeight,
                                                   unsigned & maxWidth,
                                                   unsigned & maxHeight) 
{
  minWidth = CIFWidth;
  maxWidth = CIFWidth;
  minHeight = CIFHeight;
  maxHeight = CIFHeight;
  return true;
}


PINDEX PVideoInputDevice_1394AVC::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(frameBytes);
}


PBoolean PVideoInputDevice_1394AVC::GetFrameDataNoDelay(BYTE * buffer,
                                                  PINDEX * bytesReturned)
{
  if (!IsCapturing()) return false;

  PBoolean frame_complete = false;
  PBoolean found_first_frame = false;
  int skipped = 0;
  int broken_frames = 0;
  BYTE capture_buffer[150000];
  BYTE * capture_buffer_end = capture_buffer;
  
  // this starts the bytes' rain
  if (raw1394_iso_recv_start(handle, -1, -1, -1)) {
    PTRACE(3, "Cannot start to receive data on channel " << my_channel);	
    return false;
  }
  // calling the raw1394 event manager, to get a frame:
  while(!frame_complete) {
    raw1394_loop_iterate(handle);
    if (*(uint32_t *)raw_buffer >= 492) {
      if (!found_first_frame) {
        if (raw_buffer[16] == 0x1f && raw_buffer[17] == 0x07)
          found_first_frame = true;
        else
          skipped ++;
      }
      if (skipped > 500) {
        PTRACE (3, "Skipped much too many frames");
        return false;
      }
      if (found_first_frame) {
        if (raw_buffer[16] == 0x1f
            && raw_buffer[17] == 0x07
            && (capture_buffer_end - capture_buffer > 480)) {
          // check for a short read. check if we read less
          // than a NTSC frame because it is the smaller one.
          // still not sure how to handle NTSC vs. PAL
          if (capture_buffer_end - capture_buffer < 120000) {
            broken_frames++;
            capture_buffer_end = capture_buffer;
          }
          else
            frame_complete = true;
        }
        if (!frame_complete) {
          memcpy (capture_buffer_end, raw_buffer+16, 480);
          capture_buffer_end += 480;
        }
      } // found_first_frame
      if (broken_frames > 30) {
        PTRACE(3, "Too many broken frames!");
        return false;
      }
    }
  }
  // stops the bytes from coming at us!
  raw1394_iso_stop(handle);
  
  dv_decoder_t *dv;
  dv = dv_decoder_new(true, false, false);
  dv->quality = DV_QUALITY_BEST; // FIXME: best!?
  if(dv_parse_header(dv, capture_buffer) < 0) {
    PTRACE(3, "cannot parse dv frame header");
    return false;
  }
  
  dv_color_space_t color_space;
  BYTE * pixels[3];
  int  pitches[3];
  
  pitches[0] = dv->width * 3;
  pitches[1] = pitches[2] = 0;
  
  pixels[0] = (uint8_t *)malloc(dv->width * dv->height * 3);
  pixels[1] = NULL;
  pixels[2] = NULL;
  color_space = e_dv_color_rgb;
  
  dv_decode_full_frame(dv, capture_buffer, color_space, pixels, pitches);
  
  // FIXME: this is a manual resize ; the original author wondered about it
  float xRatio = dv->width / (float)frameWidth;
  float yRatio = dv->height/ (float)frameHeight;
  for(uint y = 0; y < frameHeight; y++)
    for (uint x = 0; x < frameWidth; x++) {
      uint16_t sourceX = (uint16_t) (x * xRatio);
      uint16_t sourceY = (uint16_t) (y * yRatio);
      memcpy (pixels[0]+3*(y*frameWidth+x),
              pixels[0]+3*(sourceY*dv->width+sourceX),
              3);
    }
  if (converter != NULL) {
    converter->Convert((const BYTE *)pixels[0], buffer, bytesReturned);
    if (pixels[0] != NULL)
      free(pixels[0]);
  }
  else {
    PTRACE(3, "Converter must exist. Something goes wrong.");
    return false;
  }
  
  return true;
  
}

PBoolean PVideoInputDevice_1394AVC::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  m_pacing.Delay(1000/GetFrameRate());
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

void PVideoInputDevice_1394AVC::ClearMapping()
{
  // do nothing...
}

PBoolean PVideoInputDevice_1394AVC::TestAllFormats()
{
  return true;
}

PBoolean PVideoInputDevice_1394AVC::SetColourFormat(const PString & newFormat)
{
  return newFormat == colourFormat;
}

PBoolean PVideoInputDevice_1394AVC::SetFrameSize(unsigned width, unsigned height)
{
  if ( ! ( (width == CIFWidth && height == CIFHeight) ) )
    return false;

  frameWidth = width;
  frameHeight = height;
  colourFormat = "RGB24";
  nativeVerticalFlip = true;
  frameBytes = PVideoDevice::CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  return true;
}
// End Of File ///////////////////////////////////////////////////////////////
