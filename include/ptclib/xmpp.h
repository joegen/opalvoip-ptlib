/*
 * xmpp.h
 *
 * Extensible Messaging and Presence Protocol (XMPP) Core
 *
 * Portable Windows Library
 *
 * Copyright (c) 2004 Reitek S.p.A.
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: xmpp.h,v $
 * Revision 1.1  2004/04/22 12:31:00  rjongbloed
 * Added PNotifier extensions and XMPP (Jabber) support,
 *   thanks to Federico Pinna and Reitek S.p.A.
 *
 *
 */

#ifndef _XMPP
#define _XMPP

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if P_EXPAT

#include <ptclib/pxml.h>
#include <ptclib/psasl.h>
#include <ptlib/notifier_ext.h>


///////////////////////////////////////////////////////

/** This interface is the base class of each XMPP transport class
   
   Derived classes might include an XMPP TCP transport as well as
   classes to handle XMPP incapsulated in SIP messages.
 */
class XMPPTransport : public PIndirectChannel
{
    PCLASSINFO(XMPPTransport, PIndirectChannel);

  public:
    virtual BOOL Open() = 0;
    virtual BOOL Close() = 0;
};

///////////////////////////////////////////////////////

/** This class represents a XMPP stream, i.e. a XML message exchange
    between XMPP entities
 */
class XMPPStream : public PIndirectChannel
{
    PCLASSINFO(XMPPStream, PIndirectChannel);

  protected:
    PXMLStreamParser *  m_Parser;
    PNotifierList       m_OpenHandlers;
    PNotifierList       m_CloseHandlers;

  public:
    XMPPStream(XMPPTransport * transport = 0);
    ~XMPPStream();

    virtual BOOL        OnOpen()            { return m_OpenHandlers.Fire(*this); }
    PNotifierList&      OpenHandlers()      { return m_OpenHandlers; }

    virtual BOOL        Close();
    virtual void        OnClose()           { m_CloseHandlers.Fire(*this); }
    PNotifierList&      CloseHandlers()     { return m_CloseHandlers; }

    virtual BOOL        Write(const void * buf, PINDEX len);

    /** Read a XMPP stanza from the stream
    */
    virtual PXML *      Read();

    /** Reset the parser. The will delete and re-instantiate the
        XML stream parser.
    */
    virtual void        Reset();
};

///////////////////////////////////////////////////////

class XMPPStreamHandler : public PThread
{
    PCLASSINFO(XMPPStreamHandler, PThread);

  protected:
    XMPPStream *        m_Stream;
    BOOL                m_AutoReconnect;
    PTimeInterval       m_ReconnectTimeout;

    PNotifierList       m_ElementHandlers;

    PDECLARE_NOTIFIER(XMPPStream, XMPPStreamHandler, OnOpen);
    PDECLARE_NOTIFIER(XMPPStream, XMPPStreamHandler, OnClose);

  public:
    XMPPStreamHandler();
    ~XMPPStreamHandler();

    virtual BOOL        Start(XMPPTransport * transport);
    virtual BOOL        Stop(const PString& error = PString::Empty());

    void                SetAutoReconnect(BOOL b = TRUE, long timeout = 1000);

    PNotifierList&      ElementHandlers()   { return m_ElementHandlers; }

    virtual void        OnElement(PXML& pdu);

    virtual void        Main();
};


#endif  // P_EXPAT

#endif  // _XMPP


// End of File ///////////////////////////////////////////////////////////////

