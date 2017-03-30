/*
 * ssdp.h
 *
 * Simple Service Discovery Protocol classes.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2011 Vox Lucida Pty. Ltd.
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
 * The Initial Developer of the Original Code is Vox Lucida Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_SSDP_H
#define PTLIB_SSDP_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/http.h>


//////////////////////////////////////////////////////////////////////////////
// PSSDP

/** Simple Service Discovery Protocol.
 */
class PSSDP : public PHTTP
{
  PCLASSINFO(PSSDP, PHTTP)

  public:
  // New functions for class.
    enum Commands {
      M_SEARCH = PHTTP::NumCommands,
      NOTIFY,
      NumCommands
    };

    // Common MIME header tags
    static const PCaselessString & MXTag();
    static const PCaselessString & STTag();
    static const PCaselessString & MANTag();
    static const PCaselessString & USNTag();
    static const PCaselessString & NickNameTag();

    /** Create a TCP/IP HTTP protocol channel.
     */
    PSSDP();

    /**Listen for service notifications.
      */
    bool Listen();

    /**Search for device with the specified URN.
       Note that the Listen() must not have been called for this function to
       work correctly.
      */
    bool Search(
      const PString & urn,  ///< Resource identfier to search for
      PMIMEInfo & reply     ///< MIME for the reply
    );

    /**Read a notification.
      */
    bool GetNotify(
      PMIMEInfo & mime,
      const PString & urnRegex = ".*"
    );

  protected:
    bool Close();

    bool m_listening;
};


#endif // PTLIB_SSDP_H


// End Of File ///////////////////////////////////////////////////////////////
