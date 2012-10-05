/*
 * file.h
 *
 * Disk file I/O channel class.
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
 * $Revision$
 * $Author$
 * $Date$
 */


#ifdef __BORLANDC__
#define _open ::open
#define _close ::close
#define _read ::read
#define _write ::write
#define _lseek ::lseek
#define _chsize ::chsize
#define _access ::access
#define _chmod ::chmod
#define _mkdir ::mkdir
#define _rmdir ::rmdir
#define _chdir ::chdir
#define _mktemp ::mktemp
#define _S_IWRITE S_IWRITE
#define _S_IREAD S_IREAD
#define _O_TEXT O_TEXT
#define _O_BINARY O_BINARY
#endif


///////////////////////////////////////////////////////////////////////////////
// PFile

  protected:
    virtual PBoolean IsTextFile() const;
      // Return true if text file translation is required

// End Of File ///////////////////////////////////////////////////////////////
