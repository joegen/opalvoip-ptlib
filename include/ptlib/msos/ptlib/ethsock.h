/*
 * ethsock.h
 *
 * Direct Ethernet socket I/O channel class.
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
 * $Log: ethsock.h,v $
 * Revision 1.4  1998/11/20 03:17:42  robertj
 * Split rad and write buffers to separate pools.
 *
 * Revision 1.3  1998/09/24 03:30:01  robertj
 * Added open software license.
 *
 * Revision 1.2  1998/08/21 05:27:01  robertj
 * Fine tuning of interface.
 *
 * Revision 1.1  1998/08/20 06:04:29  robertj
 * Initial revision
 *
 */

#ifndef _PETHSOCKET

class PWin32PacketDriver;
class PWin32SnmpLibrary;
class PWin32PacketBuffer;

PARRAY(PWin32PackBufArray, PWin32PacketBuffer);


#include "../../common/ptlib/ethsock.h"
  public:
  // Overrides from class PChannel
    virtual PString GetName() const;

  protected:
    PWin32PacketDriver * driver;
    PWin32SnmpLibrary  * snmp;
    PString              interfaceName;
    PWin32PackBufArray   readBuffers;
    PWin32PackBufArray   writeBuffers;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
