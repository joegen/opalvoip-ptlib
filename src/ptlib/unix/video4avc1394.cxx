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

#pragma implementation "videoio1394avc.h"

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/videoio1394avc.h>
#include <ptlib/vconvert.h>
#include <ptlib/file.h>
#include <sys/utsname.h>

//#define ESTIMATE_CAPTURE_PERFORMANCE

#ifdef ESTIMATE_CAPTURE_PERFORMANCE
// for debugging
static PInt64 start_time;
static int num_captured;
#endif

#ifndef RAW_BUFFER_SIZE
#define RAW_BUFFER_SIZE 512
#endif 

static u_int8_t raw_buffer[RAW_BUFFER_SIZE];


///////////////////////////////////////////////////////////////////////////////
// PVideoInput1394AVC

PVideoInput1394AvcDevice::PVideoInput1394AvcDevice()
{
  PTRACE(3, "PVideo1394AvcDevice::PVideo1394AvcDevice()");
  handle = NULL;
  is_capturing = FALSE;
  capturing_duration = 10000; // arbitrary large value suffices
  dv_decoder = NULL;
}

PVideoInput1394AvcDevice::~PVideoInput1394AvcDevice()
{
  Close();
}

static int kernel_version_ok(void)
{
  PTRACE(3, "kernel_version_ok()");
  raw1394handle_t handle;
  handle = raw1394_new_handle();
  if (handle == NULL && errno == 0) {
  	return FALSE;
  }
  raw1394_destroy_handle(handle);
  return TRUE;
}

BOOL PVideoInput1394AvcDevice::Open(const PString & devName, BOOL startImmediate)
{
  PTRACE(3, "PVideoInput1394AvcDevice::Open");
  if (!kernel_version_ok()) {
    PTRACE(0, "The Linux kernel version is not compatible");
    return FALSE;
  }

  if (IsOpen()) {
    PTRACE(0, "You cannot open PVideoInput1394AvcDevice twice.");
    return FALSE;
  }

// Haven't figure how to use DMA, but let's set anyways
// Not sure what we need device name for anyway
  if (devName == "/dev/raw1394")
    UseDMA = FALSE;
  // Don't forget /dev/video1394/0
  else if (strncmp(devName, "/dev/video1394", 14) == 0)
    UseDMA = TRUE;
  else {
    PTRACE(0, "devName must be /dev/raw1394 or /dev/video1394*");
    return FALSE;
  }

  // See if devName is accessible.
  if (!PFile::Exists(devName)) {
    PTRACE(1, devName << " is not accessible.");
    return FALSE;
  }

  /*-----------------------------------------------------------------------
   *  Open ohci and asign handle to it
   *-----------------------------------------------------------------------*/
  handle = raw1394_new_handle();
  if (handle==NULL)
  {
    PTRACE(0, "Unable to aquire a raw1394 handle"
	   "did you insmod the drivers?\n");
    return FALSE;
  }

  /*-----------------------------------------------------------------------
   *  get the camera nodes and describe them as we find them
   *-----------------------------------------------------------------------*/
  // Still not letting the user choose a camera though
  struct raw1394_portinfo g_pinf[16];
  int num_cards = raw1394_get_port_info(handle, g_pinf, 16);
  if (num_cards < 0) {
  	PTRACE(0,"Couldn't get card info");
	raw1394_destroy_handle(handle);
	handle = NULL;
	return FALSE;
  }
  for (int i=0;i<num_cards;i++) {
  	PTRACE(3, "Found card "<< g_pinf[i].name << " with " << g_pinf[i].nodes << " nodes");
  }

  GetAvcCameras();
  if (numCameras < 1) {
    PTRACE(0, "no cameras found");
    raw1394_destroy_handle(handle);
    handle = NULL;
    return FALSE;
  }

//  SetFrameSize(CIFHeight, CIFWidth);
  /*
  frameWidth = CIFHeight;
  frameHeight = CIFWidth;
//  colourFormat = "UYVY422";
  colourFormat = "RGB24";
  */
  frameWidth = CIFWidth;
  frameHeight = CIFHeight;
  colourFormat = "RGB24F";
  desiredFrameHeight = CIFHeight;
  desiredFrameWidth = CIFWidth;
  desiredColourFormat = "RGB24F";

  capturing_duration = 10000; // arbitrary large value suffices
  deviceName = devName;

  // select the specified input and video format

  if (!SetChannel(channelNumber) ||
      !SetVideoFormat(videoFormat)) {
    PTRACE(1, "SetChannel() or SetVideoFormat() failed");
    Close();
    return FALSE;
  }

  if (startImmediate && !Start()) {
    Close();
    return FALSE;
  }
  PTRACE(3, "Successfully opened avc1394");
  return TRUE;
}


BOOL PVideoInput1394AvcDevice::IsOpen() 
{
  return handle != NULL;
}


BOOL PVideoInput1394AvcDevice::Close()
{
  PTRACE(3, "Close()");
  if (IsOpen()) {
    if (IsCapturing())
      Stop();
    raw1394_destroy_handle(handle);
    handle = NULL;
    return TRUE;
  } else
    return FALSE;
}

BOOL PVideoInput1394AvcDevice::Start()
{
  PTRACE(3, "Start()");
  if (!IsOpen()) return FALSE;
  if (IsCapturing()) return TRUE;
  /*
  if (frameWidth == 320 && frameHeight == 240)
  	;
  else if (frameWidth == 160 && frameHeight == 120)
  	;
  else {
    PTRACE(1, "Frame size is neither 320x240 or 160x120" << frameWidth <<
	   "x" << frameHeight);
    return FALSE;
  }
  */
  PTRACE(1, "Using " << deviceName << " at channel " << channelNumber);

  /*-----------------------------------------------------------------------
   *  have the camera start sending us data
   *-----------------------------------------------------------------------*/
   // will figure channel numbers later // channelNumber = 63??
  if (raw1394_set_iso_handler(handle, 63, & RawISOHandler)!= NULL) {
  	PTRACE (0, "Cannot set_iso_handler");
	return FALSE;
  }
#if 0
  // channelNumber 63
  if (raw1394_start_iso_rcv(handle, 63) < 0) {
  	PTRACE (0, "Cannot start_iso_rcv");
	return FALSE;
  }
#endif
  is_capturing = TRUE;
#ifdef ESTIMATE_CAPTURE_PERFORMANCE
  PTime now;
  start_time = now.GetTimestamp();
  num_captured = 0;
#endif
  return TRUE;
}


BOOL PVideoInput1394AvcDevice::Stop()
{
  PTRACE(3,"Stop()");
  if (IsCapturing()) {
  // channelNumber
  #if 0
    raw1394_stop_iso_rcv(handle, 63);
  #endif
    is_capturing = FALSE;
    return TRUE;
  } else
    return FALSE;
}


BOOL PVideoInput1394AvcDevice::IsCapturing()
{
  return is_capturing;
}

PStringList PVideoInput1394AvcDevice::GetInputDeviceNames()
{
  PTRACE(3, "GetInputDeviceNames()");
  PStringList list;

// The control of the device does not require the device name anywhere, but still...
  if (PFile::Exists("/dev/raw1394"))
    list.AppendString("/dev/raw1394");
  if (PFile::Exists("/dev/video1394/0"))
    // DEVFS naming scheme
    for (int i=0; ; i++) {
      PString devname = PString("/dev/video1394/") + PString(i);
      if (PFile::Exists(devname))
	list.AppendString(devname);
      else
	break;
    }
  else if (PFile::Exists("/dev/video1394"))
    /* traditional naming */
    list.AppendString("/dev/video1394");
  return list;
}


BOOL PVideoInput1394AvcDevice::SetVideoFormat(VideoFormat newFormat)
{
  PTRACE(3, "SetVideoFormat("<<newFormat<<")");
  if (!PVideoDevice::SetVideoFormat(newFormat)) {
    PTRACE(3,"PVideoDevice::SetVideoFormat\t failed for format "<<newFormat);
    return FALSE;
  }
  return TRUE;
}

int PVideoInput1394AvcDevice::GetBrightness()
{
  return -1;
}


BOOL PVideoInput1394AvcDevice::SetBrightness(unsigned newBrightness)
{
  return FALSE;
}


int PVideoInput1394AvcDevice::GetHue()
{
  return -1;
}


BOOL PVideoInput1394AvcDevice::SetHue(unsigned newHue)
{
  return FALSE;
}


int PVideoInput1394AvcDevice::GetContrast()
{
  return -1;
}


BOOL PVideoInput1394AvcDevice::SetContrast(unsigned newContrast)
{
  return FALSE;
}

BOOL PVideoInput1394AvcDevice::SetColour(unsigned newColour) 
{
  return -1;
}

int PVideoInput1394AvcDevice::GetColour()
{
  return -1;
}


BOOL PVideoInput1394AvcDevice::SetWhiteness(unsigned newWhiteness) 
{
  return FALSE;
}

int PVideoInput1394AvcDevice::GetWhiteness()
{
  return -1;
}


BOOL PVideoInput1394AvcDevice::GetParameters (int *whiteness, int *brightness,
                                       int *colour, int *contrast, int *hue)
{
  *whiteness = -1;
  *brightness = -1;
  *colour = -1;
  *hue = -1;
  return FALSE;
}


int PVideoInput1394AvcDevice::GetNumChannels() 
{
  return numCameras;
}


BOOL PVideoInput1394AvcDevice::SetChannel(int newChannel)
{
  PTRACE(3, "SetChannel("<<newChannel<<")");
  if (PVideoDevice::SetChannel(newChannel) == FALSE)
    return FALSE;
  if(IsCapturing()) {
    Stop();
    Start();
  }
  return TRUE;
}



BOOL PVideoInput1394AvcDevice::SetFrameRate(unsigned rate)
{
  return PVideoDevice::SetFrameRate(rate);
}


BOOL PVideoInput1394AvcDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  PTRACE(3, "GetFrameSizeLimits()");
  minWidth = 160;
  maxWidth = 320;
  minHeight = 120;
  maxHeight = 240;
  return TRUE;
}



PINDEX PVideoInput1394AvcDevice::GetMaxFrameBytes()
{
  PTRACE(3, "GetMaxFrameBytes()");
  if (converter != NULL) {
    PINDEX bytes = converter->GetMaxDstFrameBytes();
    if (bytes > frameBytes)
      return bytes;
  }

  return frameBytes;
}

BOOL dump_ppm(PString name, int w, int h, BYTE * data)
{
	PFile file(name, PFile::WriteOnly);
	if (!file.IsOpen()) return FALSE;
	file.WriteString("P6\n");
	file.WriteString(w);
	file.WriteString("\n");
	file.WriteString(h);
	file.WriteString("\n255\n");
	file.Write(data, w*h*3);
	file.Close();
	return TRUE;
}

BOOL PVideoInput1394AvcDevice::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  PTRACE(3, "GetFrameDataNoDelay()");
  if (!IsCapturing()) return FALSE;

  // get a DV frame first

  BOOL frame_complete = FALSE;
  BOOL found_first_frame = FALSE;
  int skipped = 0;
  int broken_frames = 0;
  BYTE capture_buffer[150000];
  BYTE * capture_buffer_end = capture_buffer;
  
  if (raw1394_start_iso_rcv(handle, 63) < 0) {
  	PTRACE(1, "Cannot receive data on channel 63");
	return FALSE;
  }
  while (! frame_complete) {
  	raw1394_loop_iterate(handle);
	if (*(uint32_t *)raw_buffer >= 492) {
		if (!found_first_frame) {
			if (raw_buffer[16] == 0x1f && raw_buffer[17] == 0x07)
				found_first_frame = TRUE;
			else skipped ++;
		}
		if (skipped > 500) {
			PTRACE (0, "Skipped too many frames");
			return FALSE;
		}
		if (found_first_frame) {
			if (raw_buffer[16] == 0x1f && raw_buffer[17] == 0x07 && (capture_buffer_end - capture_buffer > 480)) {
				// check for a short read. check if we read less than a NTSC frame
				// because it is the smaller one. still not sure how to handle NTSC vs. PAL
				if (capture_buffer_end - capture_buffer < 120000) {
					broken_frames ++;
					capture_buffer_end = capture_buffer;
				} else {
					frame_complete = TRUE;
				}
			}
			if (!frame_complete) {
				memcpy (capture_buffer_end, raw_buffer+16, 480);
				capture_buffer_end += 480;
			}
		}
		if (broken_frames > 30) {
			PTRACE(1, "Received too many broken frames from the camera. Giving up for now");
			return FALSE;
		}
	}
  }
  raw1394_stop_iso_rcv(handle, 63);
  PTRACE(3, "got "<< capture_buffer_end - capture_buffer << " bytes... now convert DV -> RGB");

  dv_decoder_t * dv;
  dv = dv_decoder_new(TRUE, FALSE, FALSE);
  dv->quality = DV_QUALITY_BEST;
  if (dv_parse_header(dv, capture_buffer) < 0) {
  	PTRACE(0, "cannot parse header");
	return FALSE;
  }
 
  dv_color_space_t color_space;
  BYTE * pixels[3];
  int  pitches[3];
  
  PTRACE(3, "Captured image from camera is w"<<dv->width<<"x"<<dv->height);
  pitches[0] = dv->width * 3;
  pitches[1] = pitches[2] = 0;
  pixels[0] = (uint8_t *) malloc (dv->width * dv->height * 3);
  pixels[1] = pixels[2] = NULL;
  color_space = e_dv_color_rgb;

  dv_decode_full_frame(dv, capture_buffer, color_space, pixels, pitches);
//  dump_ppm("/tmp/ohphone-before.ppm", dv->width, dv->height, pixels[0]);
#if 1 
  // gotta resize manually :( ... do I?
  PTRACE(3, "Manual resize "<<dv->width<<"x"<<dv->height<<" -> "<<frameWidth<<"x"<<frameHeight);
  float xRatio = dv->width / (float)frameWidth;
  float yRatio = dv->height/ (float)frameHeight;
  for (uint y=0;y<frameHeight;y++)
  	for (uint x=0;x<frameWidth;x++) {
		uint16_t sourceX = (uint16_t) (x * xRatio);
		uint16_t sourceY = (uint16_t) (y * yRatio);
		memcpy (pixels[0]+3*(y*frameWidth+x), pixels[0]+3*(sourceY*dv->width+sourceX), 3);
		// Temporary workaround for RGB -> BGR
		#if 1
		BYTE temp;
		int offset= (y*frameWidth+x)*3;
		temp = pixels[0][offset+0];
		pixels[0][offset+0] = pixels[0][offset+2];
		pixels[0][offset+2] = temp;
		#endif
	}
#endif
//  dump_ppm("/tmp/ohphone-after.ppm", frameWidth, frameHeight,pixels[0]);
  if (converter != NULL) {
    converter->Convert((const BYTE *)pixels[0], buffer, bytesReturned);
    if (pixels[0] != NULL) {
    	free(pixels[0]);
    }
  } else {
    PTRACE(1, "Converter must exist. Something goes wrong.");
    return FALSE;
  }
  PTRACE(3, "1394avc: return frame: "<< frameWidth << "x"<<frameHeight);
#ifdef ESTIMATE_CAPTURE_PERFORMANCE
  ++num_captured;
  PTime now;
  double capturing_time = (double)((now.GetTimestamp()-start_time))/1000000;
  ::fprintf(stderr, "time %f, num_captured=%d, fps=%f\n",
	    capturing_time, num_captured, num_captured/capturing_time);
#endif

  return TRUE;
}

BOOL PVideoInput1394AvcDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  PTRACE(2, "1394avc::getframedata");
  if(frameRate>0) {
    if (msBetweenFrames > capturing_duration)
      PThread::Current()->Sleep(msBetweenFrames - capturing_duration);
    PTime start;
    if ( !GetFrameDataNoDelay(buffer, bytesReturned))
      return FALSE;
    PTime end;
    capturing_duration = (int)((end-start).GetMilliSeconds());
    return TRUE;
  }
  return GetFrameDataNoDelay(buffer,bytesReturned);
}


void PVideoInput1394AvcDevice::ClearMapping()
{
  PTRACE(3, "ClearMapping()");
}


BOOL PVideoInput1394AvcDevice::TestAllFormats()
{
  return TRUE;
}

BOOL PVideoInput1394AvcDevice::SetColourFormat(const PString & newFormat)
{
  PTRACE(3, "SetColourFormat("<<newFormat<<")");
  if (newFormat != colourFormat) {
    return FALSE;
  }
  return TRUE;
}


BOOL PVideoInput1394AvcDevice::SetFrameSize(unsigned width, unsigned height)
{
  PTRACE(3, "SetFrameSize("<<width<<","<<height<<")");
  /*
  if ((!(width == 320 && height == 240)) &&
      (!(width == 160 && height == 120)))
    return FALSE;
    */
    #if 0
  if ((width != 720) || (height != 480))
  	return FALSE;
	#endif
  frameWidth = width;
  frameHeight = height;
/*
  if (frameWidth == 320 && frameHeight == 240)
    colourFormat = "UYVY422";
  else if (frameWidth == 160 && frameHeight == 120)
    colourFormat = "UYV444";
*/
  colourFormat = "RGB24F";
  frameBytes = PVideoDevice::CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  // Don't really need the next lines - AVC cameras don't support change in resolution
/*
  if (IsCapturing()) {
    Stop(); Start();
  }
*/
  return TRUE;
}


BOOL PVideoInput1394AvcDevice::SetFrameSizeConverter(unsigned width, unsigned height,
					 BOOL bScaleNotCrop)
{
  PTRACE(3, "SetFrameSizeConverter("<<width<<","<<height<<","<<bScaleNotCrop<<")");
  /*
  if (width == CIFWidth && height == CIFHeight)
    SetFrameSize(320, 240);
  else if (width == QCIFWidth && height == QCIFHeight)
    SetFrameSize(160, 120);
  else {
    PTRACE(1, width << "x" << height << " is not supported.");
    return FALSE;
  }
*/
  SetFrameSize(width,height);
  if (converter != NULL) 
    delete converter;
  
  desiredFrameWidth = width;
  desiredFrameHeight = height;

  PTRACE(3, "Set Converter, colourFormat="<<colourFormat<<" desiredformat="<<desiredColourFormat<<" width="<<width<<" height="<<height<<" frameWidth="<<frameWidth<<" frameHeight="<<frameHeight);
  converter = PColourConverter::Create(colourFormat, desiredColourFormat, width, height);
  if (converter == NULL) {
    PTRACE(1, "Failed to make a converter.");
    return FALSE;
  }
  if (converter->SetSrcFrameSize(width,height) == FALSE) {
    PTRACE(1, "Failed to set source frame size of a converter.");
    return FALSE;
  }
  if (converter->SetDstFrameSize(desiredFrameWidth, desiredFrameHeight, FALSE) == FALSE) {
    PTRACE(1, "Failed to set destination frame size (+scaling) of a converter.");
    return FALSE;
  }
  return TRUE;
}

BOOL PVideoInput1394AvcDevice::SetColourFormatConverter(const PString & colourFmt)
{
  PTRACE(3, "SetColourFormatConverter("<<colourFmt<<")");
/*
  if (colourFmt != "YUV420P") {
    PTRACE(1, colourFmt << " is unsupported.");
    return FALSE;
  }
*/
  desiredColourFormat = colourFmt;
  return SetFrameSizeConverter(desiredFrameWidth, desiredFrameHeight, FALSE);
}

void PVideoInput1394AvcDevice::GetAvcCameras()
{
	PTRACE(3, "1394AVC :: GetAvcCameras");
	camera_nodes = (nodeid_t *) malloc (sizeof(nodeid_t)*16);
	if (camera_nodes == NULL) {
		PTRACE(0, "getavccameras :: out of memory");
		return;
	}
	numCameras = 0;
	rom1394_directory rom_dir;
	if (handle == NULL) {
		PTRACE(0, "Weird, handle not created yet :-/");
		return;
	}
	if (raw1394_set_port(handle, 0) < 0) {
		PTRACE(0, "Cannot set port");
		return;
	}
	int nodecount = raw1394_get_nodecount(handle);
	for (int i=0; i<nodecount;i++) {
		PTRACE(3, "check node " << i);
		if (rom1394_get_directory(handle, i, &rom_dir) < 0) {
			PTRACE(0, "Cannot read ROM data for node" << i);
			return;
		}
		if ((rom1394_get_node_type(&rom_dir) == ROM1394_NODE_TYPE_AVC)) {
			PTRACE(3, "Found a camera at node " << i);
			camera_nodes[numCameras++] = i;
		}
	}
}

int RawISOHandler (raw1394handle_t handle, int channel, size_t length, u_int32_t * data)
{
	PTRACE(4, "1394avc :: rawisohandler with length " << length);
	if (length < RAW_BUFFER_SIZE) {
		*(u_int32_t *) raw_buffer = length;
		memcpy (raw_buffer + 4, data, length);
	}
	return 0;
}
// End Of File ///////////////////////////////////////////////////////////////
