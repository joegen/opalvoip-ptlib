/*
 * vfakeio.cxx
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
 * Contributor(s): Derek J Smithies (derek@indranet.co.nz)
 *
 * $Log: vfakeio.cxx,v $
 * Revision 1.19  2003/06/10 03:45:11  dereksmithies
 * Change so box on left moves all the way down left side of image.
 *
 * Revision 1.18  2003/06/10 00:36:57  dereksmithies
 * Formatting changes. Remove rounding errors.
 *
 * Revision 1.17  2003/06/03 04:21:49  dereksmithies
 * Add PTRACE statement, and tidy up format of one if statement.
 *
 * Revision 1.16  2003/03/17 07:46:49  robertj
 * Migrated vflip member variable and functions into PVideoDevice class.
 *
 * Revision 1.15  2002/09/23 07:17:24  robertj
 * Changes to allow winsock2 to be included.
 *
 * Revision 1.14  2002/01/28 21:22:10  dereks
 * Fix the method for returning the device name.
 *
 * Revision 1.13  2002/01/17 03:47:27  dereks
 * Fix latest addition to the fake images gallery.
 *
 * Revision 1.12  2002/01/16 08:02:06  robertj
 * MSVC compatibilty changes
 *
 * Revision 1.11  2002/01/16 03:49:23  dereks
 * Add new test image.
 *
 * Revision 1.10  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
 * Revision 1.9  2001/11/28 04:39:25  robertj
 * Fixed MSVC warning
 *
 * Revision 1.8  2001/11/28 00:07:32  dereks
 * Locking added to PVideoChannel, allowing reader/writer to be changed mid call
 * Enabled adjustment of the video frame rate
 * New fictitous image, a blank grey area
 *
 * Revision 1.7  2001/03/12 03:54:11  dereks
 * Make setting frame rate consistent with that for real video device.
 *
 * Revision 1.6  2001/03/09 00:12:40  robertj
 * Fixed incorrect number of channels returned on fake video.
 *
 * Revision 1.5  2001/03/08 22:56:25  robertj
 * Fixed compatibility with new meaning of channelNumber variable, cannot be negative.
 *
 * Revision 1.4  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.3  2001/03/02 06:52:33  yurik
 * Got rid of unknown for WinCE pragma
 *
 * Revision 1.2  2000/12/19 23:58:14  robertj
 * Fixed MSVC compatibility issues.
 *
 * Revision 1.1  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "vfakeio.h"
#endif

#include <ptlib.h>
#include <ptlib/videoio.h>


#define NUM_PATTERNS 6


///////////////////////////////////////////////////////////////////////////////
// PFakeVideoInputDevice

PFakeVideoInputDevice::PFakeVideoInputDevice()
{
  grabCount = 0;
  SetFrameRate(10);
  doVFlip = FALSE;
}



BOOL PFakeVideoInputDevice::Open(const PString & /*devName*/, BOOL /*startImmediate*/)
{
  return TRUE;    
}


BOOL PFakeVideoInputDevice::IsOpen() 
{
  return TRUE;
}


BOOL PFakeVideoInputDevice::Close()
{
  return TRUE;
}


BOOL PFakeVideoInputDevice::Start()
{
  return TRUE;
}


BOOL PFakeVideoInputDevice::Stop()
{
  return TRUE;
}


BOOL PFakeVideoInputDevice::IsCapturing()
{
  return IsOpen();
}


PStringList PFakeVideoInputDevice::GetInputDeviceNames()
{
  PStringList list;

  list.AppendString("fake");

  return list;
}


BOOL PFakeVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat(newFormat);
}


int PFakeVideoInputDevice::GetNumChannels() 
{
  return NUM_PATTERNS;  
}


BOOL PFakeVideoInputDevice::SetChannel(int newChannel)
{
  return PVideoDevice::SetChannel(newChannel);
}


BOOL PFakeVideoInputDevice::SetColourFormat(const PString & newFormat)
{
  return PVideoDevice::SetColourFormat(newFormat);
}


BOOL PFakeVideoInputDevice::SetFrameRate(unsigned rate)
{
  if ((rate < 1) || (rate > 50))
    PVideoDevice::SetFrameRate(10);
  else
    PVideoDevice::SetFrameRate(rate);

  return TRUE;
}


BOOL PFakeVideoInputDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  minWidth  = 10;
  minHeight = 10;
  maxWidth  = 1000;
  maxHeight =  800;

  return TRUE;
}


BOOL PFakeVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  if (!PVideoDevice::SetFrameSize(width, height))
    return FALSE;

  videoFrameSize = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  return TRUE;
}


PINDEX PFakeVideoInputDevice::GetMaxFrameBytes()
{
  return videoFrameSize;
}

void PFakeVideoInputDevice::WaitFinishPreviousFrame()
{
  frameTimeError += msBetweenFrames;

  PTime now;
  PTimeInterval delay = now - previousFrameTime;
  frameTimeError -= (int)delay.GetMilliSeconds();
  previousFrameTime = now;

  if (frameTimeError > 0) {
    PTRACE(6, "FakeVideo\t Sleep for " << frameTimeError << " milli seconds");
#ifdef P_LINUX
    usleep(frameTimeError * 1000);
#else
    PThread::Current()->Sleep(frameTimeError);
#endif
  } 
}


BOOL PFakeVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{    
  WaitFinishPreviousFrame();

  GetFrameDataNoDelay(buffer, bytesReturned);
  
  *bytesReturned= videoFrameSize;

  return TRUE;
}


BOOL PFakeVideoInputDevice::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * /*bytesReturned*/)
{
     grabCount++;

     // Make sure are NUM_PATTERNS cases here.
     switch(channelNumber){       
     case 0: 
       GrabMovingBlocksTestFrame(destFrame);
       break;
     case 1: 
       GrabMovingLineTestFrame(destFrame);
       break;
     case 2:
       GrabBouncingBoxes(destFrame);
       break;
     case 3:
       GrabBlankImage(destFrame);
       break;
     case 4:
       GrabOriginalMovingBlocksFrame(destFrame);
       break;
     default:
       GrabNTSCTestFrame(destFrame);
     }

     return TRUE;
}


void PFakeVideoInputDevice::FillRect(BYTE * frame, unsigned width, unsigned height,
				     int xPos, int initialYPos,
				     int rectWidth, int rectHeight,
				     int r, int g,  int b)
{
// PTRACE(0,"x,y is"<<xPos<<" "<<yPos<<" and size is "<<rectWidth<<" "<<rectHeight);
	//This routine fills a region of the video image with data. It is used as the central
	//point because one only has to add other image formats here.
  int yPos;
  if ( doVFlip ) {
    yPos = height - ( initialYPos + rectHeight );
    yPos = ( yPos >> 2 ) * 4;
  }
  else
    yPos = initialYPos;

  int offset       = ( yPos * width ) + xPos;
  int colourOffset = ( (yPos * width) >> 2) + (xPos >> 1);

  int Y  =  (int)( (0.257 * r) + (0.504 * g) + (0.098 * b) + 16);
  int Cb =  (int)(-(0.148 * r) - (0.291 * g) + (0.439 * b) + 128);
  int Cr =  (int)( (0.439 * r) - (0.368 * g) - (0.071 * b) + 128);

  unsigned char * Yptr  = frame + offset;
  unsigned char * CbPtr = frame + (width * height) + colourOffset;
  unsigned char * CrPtr = frame + (width * height) + (width * height/4)  + colourOffset;
  
  int rr ;
  int halfRectWidth = rectWidth >> 1;
  int halfWidth     = width >> 1;
  
  for (rr = 0; rr < rectHeight;rr+=2) {
    memset(Yptr, Y, rectWidth);
    Yptr += width;
    memset(Yptr, Y, rectWidth);
    Yptr += width;

    memset(CbPtr, Cb, halfRectWidth);
    memset(CrPtr, Cr, halfRectWidth);

    CbPtr += halfWidth;
    CrPtr += halfWidth;
  }
}

void PFakeVideoInputDevice::GrabBouncingBoxes(BYTE *resFrame)
{
  unsigned width=0;
  unsigned height=0;
  GetFrameSize(width,height);

  grabCount++;

  FillRect(resFrame, width, height,
	            0, 0,                       //Position x,y
              width, height, //Fill the whole frame with the colour.
		          200,200,200); //a light grey colour.
			  
	double t= (grabCount%50) -25 ;
	double h=  t*t*height*0.85/625;
  int    yBox = (int)h;
  yBox= (yBox>>1) * 2;  //yBox is even.
  
  int boxHeight= (int)(height*0.1);
  boxHeight= (boxHeight >>1) * 2;
  int boxWidth = (int)(width*0.1);
  boxWidth = (boxWidth >>1) * 2;

  FillRect(resFrame, width, height,
            width>>2,yBox, 
	           boxWidth,boxHeight,
             255, 0, 0); // Red Box.

	t= (grabCount%40) -20 ;
	h= t*t*height*0.85/400 ;
  yBox = (int)h;
  yBox= (yBox>>1) * 2;  //yBox is even.

  FillRect(resFrame, width, height,
            width>>1,yBox, 
	           boxWidth,boxHeight,
             0, 255, 0); // Green

  t= (grabCount%100) -50 ;
	h= t*t*height*0.85/2500;		  
  yBox = (int)h;
  yBox= (yBox>>1) * 2;  //yBox is even.

  FillRect(resFrame, width, height,
            (width>>1) + (width>>2),yBox, 
	           boxWidth,boxHeight,
             0, 0, 255); // Blue
	     
			   		   
}

void PFakeVideoInputDevice::GrabNTSCTestFrame(BYTE *resFrame)
{
    //  Test image # 1
    //  A static image is generated, consisting of a series of coloured block.
    //  Sample NTSC test frame is found at http://www.displaymate.com/patterns.html
	//
	static int row1[7][3] = {
    { 204, 204, 204 },   // 80% grey
    { 255, 255,   0 },   // yellow
    {   0, 255, 255 },   // cyan
    {   0, 255,   0 },   // green
    { 255,   0, 255 },   // magenta
    { 255,   0,   0 },   // red
    {   0,   0, 255 },   // blue
  };

  static int row2[7][3] = {
    {   0,   0, 255 },   // blue
    {  19,  19,  19 },   // black
    { 255,   0, 255 },   // magenta
    {  19,  19,  19 },   // black
    {   0, 255, 255 },   // cyan
    {  19,  19,  19 },   // black
    { 204, 204, 204 },   // grey
  };

  static int row3a[4][3] = {
    {   8,  62,  89 },   // I
    { 255, 255, 255 },   // white
    {  58,   0, 126 },   // +Q
    {  19,  19,  19 },   // black
  };

  static int row3b[3][3] = {
    {   0,   0,   0 },   // 3.5
    {  19,  19,  19 },   // 7.5
    {  38,  38,  38 },   // 11.5
  };

  static int row3c[3] = { 19,  19,  19 };

  unsigned width=0;
  unsigned height=0;
  GetFrameSize(width,height);
      
  int row1Height = (int)(0.66 * height);
  int row2Height = (int)((0.75 * height) - row1Height);
  row1Height = (row1Height>>1)*2;     //Require that height is even.
  row2Height = (row2Height>>1)*2;     
  int row3Height = height - row1Height - row2Height;

  int columns[8];
  PINDEX i;

  for(i=0;i<7;i++){    
    columns[i]= i*width/7;
    columns[i]= (columns[i]>>1)*2;  // require that columns[i] is even.
  }
  columns[7] = width;


  // first row
  for (i = 0; i < 6; i++) 
    FillRect(resFrame, width, height,
	         columns[i], 0,                       //Position x,y
             columns[i+1]-columns[i], row1Height, //Size (width*height)
             row1[i][0], row1[i][1], row1[i][2]); // rgb



  // second row
  for (i = 0; i < 7; i++) 
    FillRect(resFrame, width, height,
	         columns[i], row1Height, 
             columns[i+1]-columns[i], row2Height, 
             row2[i][0], row2[i][1], row2[i][2]);

  // third row
  int columnBot[5];
  
  for (i=0; i<4; i++) {    
    columnBot[i]= i*columns[5]/4;
    columnBot[i] = 2 * (columnBot[i]>>1);
  }
  columnBot[4]= columns[5];
  
  for (i = 0; i < 4; i++) 
    FillRect(resFrame, width, height,
	         columnBot[i],row1Height + row2Height, 
             columnBot[i+1]-columnBot[i], row3Height, 
             row3a[i][0], row3a[i][1], row3a[i][2]);

  for (i=0; i<3; i++) {
    columnBot[i] = columns[4]+(i*width)/(7*3);
    columnBot[i] = 2 * (columnBot[i]>>1);       //Force even.
  }
  columnBot[3]= columns[5];

  for (i = 0; i < 3; i++) 
    FillRect(resFrame, width, height,
	         columnBot[i], row1Height + row2Height, 
             columnBot[i+1] - columnBot[i], row3Height,
             row3b[i][0], row3b[i][1], row3b[i][2]);

  FillRect(resFrame, width, height,
	       columns[6],     row1Height + row2Height, 
           columns[7]-columns[6], row3Height, 
           row3c[0], row3c[1], row3c[2]);


}


void PFakeVideoInputDevice::GrabMovingBlocksTestFrame(BYTE * resFrame)
{
  // Test image # 2
  /*Brightness is set to alter, left to right.
    Colour component alters top to bottom.

    Image contains lots of high and low resolution areas.
  */
    unsigned width, height, wi,hi, colourIndex,colNo, boxSize;   

#define COL(b,x,y) ((b+x+y)%7)

    static int background[7][3] = {
      { 254, 254, 254 },   // white
      { 255, 255,   0 },   // yellow
      {   0, 255, 255 },   // cyan
      {   0, 255,   0 },   // green
      { 255,   0, 255 },   // magenta
      { 255,   0,   0 },   // red
      {   0,   0, 255 },   // blue
    };

    width = 0;   //Stops compiler warning about unitialized variables.
    height = 0;      
    GetFrameSize(width, height);
      
    int columns[9];
    int heights[9];
    int offset;
    offset = (width >> 3) & 0xfe;

    for(wi = 0; wi < 8; wi++) 
      columns[wi] = wi * offset;
    columns[8] = width;
    
    offset = (height >> 3) & 0xfe;
    for(hi = 0; hi < 9; hi++) 
      heights[hi] = hi * offset;
    heights[8] = height;

    grabCount++;
    colourIndex = time(NULL);//time in seconds since last epoch.
    // Provides a difference if run on two ohphone sessions.
    colNo = (colourIndex / 10) % 7;   //Every 10 seconds, coloured background blocks move.

    for(hi = 0; hi < 8; hi++) //Fill the background in.
      for(wi = 0 ; wi < 8; wi++) {
        FillRect(resFrame, width, height,   //frame details.
                 columns[wi], heights[hi],   //X,Y Pos.
		 columns[wi + 1] - columns[wi], heights[hi + 1] - heights[hi],
                 background[COL(colNo, wi, hi)][0], background[COL(colNo, wi, hi)][1], background[COL(colNo, wi, hi)][2]);
      }
    
    //Draw a black box rapidly moving down the left of the window.
    boxSize= height / 10;
    hi = ((3 * colourIndex) % (height-boxSize)) & 0xfe; //Make certain hi is even.
    FillRect(resFrame, width, height, 10, hi, boxSize, boxSize, 0, 0, 0); //Black Box.
    
    //Draw four parallel black lines, which move up the middle of the window.
    colourIndex = colourIndex / 3;     //Every three seconds, lines move.
    
    for(wi = 0; wi < 2; wi++) 
      columns[wi]= (((wi + 1)  * width) / 3) & 0xfe;// Force columns to be even.
    
    hi = colourIndex % ((height - 16) / 2);
    hi = (height - 16) - (hi * 2);     //hi is even, Lines move in opp. direction to box.
    
    unsigned yi;    
    for(yi = 0; yi < 4; yi++) 
      FillRect(resFrame, width, height, 
	       columns[0], hi+(yi * 4),
	       columns[1] - columns[0], 2, 0, 0, 0);
}


void PFakeVideoInputDevice::GrabMovingLineTestFrame(BYTE *resFrame)
{
    //  Test image # 3
    //  Faster image generation. Same every times system runs.
    //  Colours cycle through. Have a vertical lines style of pattern.
    //  There is a horizontal bar which moves down the screen .
      unsigned width,height;
      static int v=0;
      int r,g,b;

      width=0;   //Stops compiler warning about unitialized variables.
      height=0;
      
      GetFrameSize(width,height);      
      
	  v++;
	  r = (200+v) & 255;
	  g = (100+v) & 255;
	  b = (000+v) & 255;
      
	  FillRect(resFrame, width, height, 0, 0,width, height, r, g, b);

	  int hi = (v % (height-2) >> 1) *2;

	  FillRect(resFrame, width, height, 0, hi,   width, 2, 0, 0, 0);
}

void PFakeVideoInputDevice::GrabBlankImage(BYTE *resFrame)
{
  unsigned width=0;
  unsigned height=0;
  GetFrameSize(width,height);

  grabCount++;

  FillRect(resFrame, width, height,
                    0, 0,                       //Position x,y
              width, height, //Fill the whole frame with the colour.
                          200,200,200); //a light grey colour.                                                             
}

void PFakeVideoInputDevice::GrabOriginalMovingBlocksFrame(BYTE *frame)
{
  unsigned w=0;
  unsigned h=0;
  GetFrameSize(w,h);
  int width  = w;
  int height = h;

  int wi,hi,colourIndex,colourNumber;
  int framesize = width*height;

  static int gCount=0;
  gCount++;

  colourIndex = gCount/10;
  colourNumber= (colourIndex/10)%7;   //Every 10 seconds, coloured background blocks move.
  
  for(hi=0; hi<height; hi++)               //slow moving group of lines going upwards.
    for(wi=0; wi<width; wi++) 
      if ( (wi>width/3)&&(wi<width*2/3)&&
	   ( ((gCount+hi)%height)<16)&&
	   ( (hi%4)<2)                     )
	frame[(hi*width)+wi] = 16;
      else
	frame[(hi*width)+wi] = (BYTE)(((colourNumber+((wi*7)/width))%7)*35+26);

  for(hi=1; hi<=height; hi++)                 //fast moving block going downwards.
    for(wi=width/9; wi<(2*width/9); wi++) 
      if(  (( (gCount*4)+hi)%height)<20)
	frame[((height-hi)*width)+wi] = 16;

  int halfWidth  = width/2;
  int halfHeight = height/2;
  for(hi=1; hi<halfHeight; hi++)  
    for(wi=0; wi<halfWidth; wi++)
      frame[framesize+(hi*halfWidth)+wi] = (BYTE)(((((hi*7)/halfHeight)+colourNumber)%7)*35+26);
}



// End Of File ///////////////////////////////////////////////////////////////
