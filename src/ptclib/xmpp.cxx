/*
 * xmpp.cxx
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
 * $Log: xmpp.cxx,v $
 * Revision 1.1  2004/04/22 12:31:00  rjongbloed
 * Added PNotifier extensions and XMPP (Jabber) support,
 *   thanks to Federico Pinna and Reitek S.p.A.
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "xmpp.h"
#endif

#include <ptlib.h>
#include <ptclib/xmpp.h>

#if P_EXPAT

XMPPStream::XMPPStream(XMPPTransport * transport)
  : m_Parser(new PXMLStreamParser)
{
  if (transport)
    Open(transport);
}


XMPPStream::~XMPPStream()
{
  delete m_Parser;
  Close();
}


BOOL XMPPStream::Close()
{
  OnClose();
  return PIndirectChannel::Close();
}


BOOL XMPPStream::Write(const void * buf, PINDEX len)
{
  PTRACE(5, "XMPP\tSND: " << (const char *)buf);
  return PIndirectChannel::Write(buf, len);
}


PXML * XMPPStream::Read()
{
  return m_Parser->Read(this);
}


void XMPPStream::Reset()
{
  delete m_Parser;
  m_Parser = new PXMLStreamParser;
}

///////////////////////////////////////////////////////

XMPPStreamHandler::XMPPStreamHandler() :
PThread(0x1000), m_Stream(0), m_AutoReconnect(TRUE), m_ReconnectTimeout(1000)
{
}


XMPPStreamHandler::~XMPPStreamHandler()
{
  Stop();
}


BOOL XMPPStreamHandler::Start(XMPPTransport * transport)
{
  if (m_Stream)
    Stop();

  m_Stream = new XMPPStream();
  m_Stream->OpenHandlers().Add(new PCREATE_NOTIFIER(OnOpen));
  m_Stream->CloseHandlers().Add(new PCREATE_NOTIFIER(OnClose));

  if (!transport->IsOpen() && !transport->Open())
    return FALSE;

  if (m_Stream->Open(transport))
  {
    if (IsSuspended())
      Resume();
    else
      Restart();
    return TRUE;
  }
  else
    return FALSE;
}


BOOL XMPPStreamHandler::Stop(const PString& _error)
{
  if (!_error.IsEmpty())
  {
    PString error = "<stream:error><";
    error += _error;
    error += " xmlns='urn:ietf:params:xml:ns:xmpp-streams'/></stream:error>";
    m_Stream->Write((const char *)error, error.GetLength());
  }

  m_Stream->Close();

  if (PThread::Current() != this)
    WaitForTermination(10000);

  return FALSE;
}


void XMPPStreamHandler::OnOpen(XMPPStream&, INT)
{
}


void XMPPStreamHandler::OnClose(XMPPStream&, INT)
{
}


void XMPPStreamHandler::SetAutoReconnect(BOOL b, long t)
{
  m_AutoReconnect = b;
  m_ReconnectTimeout = t;
}


void XMPPStreamHandler::OnElement(PXML& pdu)
{
  m_ElementHandlers.Fire(pdu);
}


void XMPPStreamHandler::Main()
{
  PXML * pdu;

  for (;;)
  {
    if (!m_Stream || !m_Stream->IsOpen())
      break;

    pdu = m_Stream->Read();

    if (pdu)
    {
      PTRACE(5, "XMPP\tRCV: " << *pdu);
      OnElement(*pdu);
    }

    delete pdu;
  }

  delete m_Stream;
  m_Stream = 0;
}

#endif // P_EXPAT

// End of File ///////////////////////////////////////////////////////////////

