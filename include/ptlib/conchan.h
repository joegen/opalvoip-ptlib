/*
 * conchan.h
 *
 * Console I/O channel class.
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
 * $Log: conchan.h,v $
 * Revision 1.1  1999/06/13 13:54:07  robertj
 * Added PConsoleChannel class for access to stdin/stdout/stderr.
 *
 */


#define _PCONSOLECHANNEL

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Console Channel

/**This class defines an I/O channel that communicates via a console.
 */
class PConsoleChannel : public PChannel
{
  PCLASSINFO(PConsoleChannel, PChannel);

  public:
    enum ConsoleType {
      StandardInput,
      StandardOutput,
      StandardError
    };

  /**@name Construction */
  //@{
    /// Create a new console channel object, leaving it unopen.
    PConsoleChannel();

    /// Create a new console channel object, connecting to the I/O stream.
    PConsoleChannel(
      ConsoleType type  /// Type of console for object
    );
  //@}


  /**@name Open functions */
  //@{
    /**Open a serial channal.
       The channel is opened it on the specified port and with the specified
       attributes.
     */
    virtual BOOL Open(
      ConsoleType type  /// Type of console for object
    );
  //@}

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
