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
 */

#ifndef PTLIB_CONSOLECHANNEL_H
#define PTLIB_CONSOLECHANNEL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

P_PUSH_MSVC_WARNINGS(4250)

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

    /// Special retrun codes from console input
    enum {
      KeyLeft = 0x10000, // Larger than possible Unicode character return
      KeyRight,
      KeyUp,
      KeyDown,
      KeyPageUp,
      KeyPageDown,
      KeyHome,
      KeyEnd,
      KeyBackSpace,
      KeyDelete,
      KeyInsert,

      KeyFuncBase = 0x10100,
      KeyF1 = KeyFuncBase+1,
      KeyF2,
      KeyF3,
      KeyF4,
      KeyF5,
      KeyF6,
      KeyF7,
      KeyF8,
      KeyF9,
      KeyF10,
      KeyF11,
      KeyF12,
      // Additional function keys have codes from here

      MouseEvent = 0x40000000,
      MouseButton1 = 1,         ///< State of (usually) left button
      MouseButton2 = 2,         ///< State of (usually) right button
      MouseButton3 = 4,         ///< State of (usually) middle button
      MouseButton4 = 8,         ///< State of (on the side?) button
      MouseClickShift = 4,      ///< 3 bits for button that was "clicked", that is went down and up. Zero is none.
      MouseDoubleClick = 0x80,  ///< The click was a double
      MouseShiftKey = 0x100,    ///< The shift key is down
      MouseCtrlKey = 0x100,     ///< The control key is down
      MouseAltKey = 0x200,      ///< The control key is down
      MouseRowShift = 20,       ///< Shift to get 8 bit row
      MouseColShift = 12,       ///< Shift to get 8 bit column
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

  /**@name Overrides from PChannel */
  //@{
    /** Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in <code>PFile</code> type channels.

       @return the name of the channel.
     */
    virtual PString GetName() const;

    /** Close the channel, shutting down the link to the data source.

       @return true if the channel successfully closed.
     */
    virtual PBoolean Close();

    /** Read a single 8 bit byte from the channel. If one was not available
       within the read timeout period, or an I/O error occurred, then the
       function gives with a -1 return value.

       @return
       byte read or -1 if no character could be read.
     */
    virtual int ReadChar();

    /**Set local echo mode.
       For some classes of channel, e.g. PConsoleChannel, data read by this
       channel is automatically echoed. This disables the function so things
       like password entry can work.

       Default behaviour does nothing and return true if the channel is open.
      */
    virtual bool SetLocalEcho(
      bool localEcho
    );

    /**Set line buffered mode.
       For some classes of channel, e.g. PConsoleChannel, data read by this
       channel is not returned until a whole line is .

       Default behaviour does nothing and return true if the channel is open.
      */
    virtual bool SetLineBuffered(
      bool lineBuffered
    );
  //@}

  /**@name Open functions */
  //@{
    /**Open a serial channal.
       The channel is opened it on the specified port and with the specified
       attributes.
     */
    virtual PBoolean Open(
      ConsoleType type  /// Type of console for object
    );
  //@}


// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/conchan.h"
#else
#include "unix/ptlib/conchan.h"
#endif

};


#endif // PTLIB_CONSOLECHANNEL_H


// End Of File ///////////////////////////////////////////////////////////////
