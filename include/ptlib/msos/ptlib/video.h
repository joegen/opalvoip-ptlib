/*
 * video.h
 *
 * Video class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: video.h,v $
 * Revision 1.1  2000/11/09 00:33:23  dereks
 * Initial release. Required for PVideoChannel class.
 *
 *
 */


#ifndef _PVIDEO

#include <mmsystem.h>


///////////////////////////////////////////////////////////////////////////////
// PVideo


class PWaveFormat : public PObject
{
    PCLASSINFO(PWaveFormat, PObject)
  public:
    PWaveFormat();
    ~PWaveFormat();
    PWaveFormat(const PWaveFormat & fmt);
    PWaveFormat & operator=(const PWaveFormat & fmt);

    void PrintOn(ostream &) const;
    void ReadFrom(istream &);

    void SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample);
    void SetFormat(const void * data, PINDEX size);

    BOOL           SetSize   (PINDEX sz);
    PINDEX         GetSize   () const { return  size;       }
    void         * GetPointer() const { return  waveFormat; }
    WAVEFORMATEX * operator->() const { return  waveFormat; }
    WAVEFORMATEX & operator *() const { return *waveFormat; }
    operator   WAVEFORMATEX *() const { return  waveFormat; }

  protected:
    PINDEX         size;
    WAVEFORMATEX * waveFormat;
};


class PVideo;

class PWaveBuffer : public PBYTEArray
{
  PCLASSINFO(PWaveBuffer, PBYTEArray);
  private:
    PWaveBuffer(PINDEX sz = 0);
    ~PWaveBuffer();

    PWaveBuffer & operator=(const PVideo & video);

    DWORD Prepare(HWAVEOUT hWaveOut, PINDEX & count);
    DWORD Prepare(HWAVEIN hWaveIn);
    DWORD Release();

    void PrepareCommon(PINDEX count);

    HWAVEOUT hWaveOut;
    HWAVEIN  hWaveIn;
    WAVEHDR  header;

  friend class PVideoChannel;
};

PARRAY(PWaveBufferArray, PWaveBuffer);



#include "../../video.h"
  public:
    // Overrides from class PChannel
    virtual PString GetName() const;
      // Return the name of the channel.

      
    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL Close();
      // Close the channel.


    PString GetErrorText() const;
    // Get a text form of the last error encountered.

    BOOL SetFormat(const PWaveFormat & format);

  protected:
    PString     deviceName;
    Directions  direction;
    HWAVEIN     hWaveIn;
    HWAVEOUT    hWaveOut;
    HANDLE      hEventDone;
    PWaveFormat waveFormat;

    PWaveBufferArray buffers;
    PINDEX           bufferIndex;
    PINDEX           bufferByteOffset;
    PMutex           bufferMutex;

  private:
    BOOL OpenDevice(unsigned id);
};


#endif
