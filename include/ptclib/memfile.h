/*
 * memfile.h
 *
 * WAV file I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is
 * Equivalence Pty Ltd
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: memfile.h,v $
 * Revision 1.1  2002/06/26 09:01:19  craigs
 * Initial version
 *
 */

/**
  This class is used to allow a block of memory to substitute for a disk file
	*/

#ifndef _PMEMFILE
#define _PMEMFILE

class PMemoryFile : public PFile
{
  PCLASSINFO(PMemoryFile, PFile);
  public:
    PMemoryFile();
    PMemoryFile(const PBYTEArray & data);

    virtual BOOL Read(
      void * buf,   /// Pointer to a block of memory to receive the read bytes.
      PINDEX len    /// Maximum number of bytes to read into the buffer.
    );

    virtual BOOL Write(
      const void * buf, /// Pointer to a block of memory to write.
      PINDEX len        /// Number of bytes to write.
    );

    off_t GetLength() const;
      
    BOOL SetLength(
      off_t len   // New length of file.
    );

    BOOL SetPosition(
      off_t pos,                         /// New position to set.
      FilePositionOrigin origin = Start  /// Origin for position change.
    );

    off_t GetPosition() const;

  protected:
    PBYTEArray data;
    off_t position;
};

#endif
