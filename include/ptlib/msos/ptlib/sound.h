/*
 * sound.h
 *
 * Sound class.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: sound.h,v $
 * Revision 1.12  2001/10/10 03:28:58  yurik
 * Added open with format other than PCM
 *
 * Revision 1.11  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.10  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.9  2000/05/01 05:59:11  robertj
 * Added mutex to PSoundChannel buffer structure.
 *
 * Revision 1.8  1999/09/23 04:28:43  robertj
 * Allowed some Win32 only access to wave format in sound channel
 *
 * Revision 1.7  1999/05/24 03:02:32  robertj
 * Added include for compiling under different environments.
 *
 * Revision 1.6  1999/02/22 10:15:15  robertj
 * Sound driver interface implementation to Linux OSS specification.
 *
 * Revision 1.5  1999/02/16 06:02:39  robertj
 * Major implementation to Linux OSS model
 *
 * Revision 1.4  1998/11/30 02:55:33  robertj
 * New directory structure
 *
 * Revision 1.3  1998/09/24 03:30:26  robertj
 * Added open software license.
 *
 * Revision 1.2  1996/08/08 10:10:45  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1994/04/12 08:21:52  robertj
 * Initial revision
 *
 */


#ifndef _PSOUND

#include <mmsystem.h>


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


class PSound;

class PWaveBuffer : public PBYTEArray
{
  PCLASSINFO(PWaveBuffer, PBYTEArray);
  private:
    PWaveBuffer(PINDEX sz = 0);
    ~PWaveBuffer();

    PWaveBuffer & operator=(const PSound & sound);

    DWORD Prepare(HWAVEOUT hWaveOut, PINDEX & count);
    DWORD Prepare(HWAVEIN hWaveIn);
    DWORD Release();

    void PrepareCommon(PINDEX count);

    HWAVEOUT hWaveOut;
    HWAVEIN  hWaveIn;
    WAVEHDR  header;

  friend class PSoundChannel;
};

PARRAY(PWaveBufferArray, PWaveBuffer);



///////////////////////////////////////////////////////////////////////////////
// PSound

#define _PSOUND_PLATFORM_INCLUDE
#include "../../sound.h"

#endif
#ifdef _PSOUND_PLATFORM_INCLUDE
#undef _PSOUND_PLATFORM_INCLUDE

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


    PString GetErrorText(ErrorGroup group = NumErrorGroups) const;
    // Get a text form of the last error encountered.

    BOOL SetFormat(const PWaveFormat & format);

    BOOL Open(const PString & device, Directions dir,const PWaveFormat & format);
	// Open with format other than PCM
	
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

	BOOL GetDeviceID(const PString & device, Directions dir, unsigned& id);
#endif


// End Of File ///////////////////////////////////////////////////////////////
