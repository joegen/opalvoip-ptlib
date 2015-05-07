/*
 * ssdp.cxx
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "ssdp.h"
#endif

#include <ptlib.h>

#include <ptclib/ssdp.h>
#include <ptlib/udpsock.h>


static const PIPSocket::Address ssdpMulticast(239,255,255,250);
static const WORD ssdpPort = 1900;

const PCaselessString & PSSDP::MXTag()       { static const PConstCaselessString s("MX");       return s; }
const PCaselessString & PSSDP::STTag()       { static const PConstCaselessString s("ST");       return s; }
const PCaselessString & PSSDP::MANTag()      { static const PConstCaselessString s("MAN");      return s; }
const PCaselessString & PSSDP::USNTag()      { static const PConstCaselessString s("USN");      return s; }
const PCaselessString & PSSDP::NickNameTag() { static const PConstCaselessString s("NickName"); return s; }


//////////////////////////////////////////////////////////////////////////////
// PSSDP

static const char * const SSDPCommands[PSSDP::NumCommands-PHTTP::NumCommands] = 
{
  "M-SEARCH", "NOTIFY"
};

PSSDP::PSSDP()
  : PHTTP("ssdp 1900")
  , m_listening(false)
{
  for (PINDEX i = PHTTP::NumCommands; i < PSSDP::NumCommands; ++i)
    commandNames.AppendString(PCaselessString(SSDPCommands[i-PHTTP::NumCommands]));
}


bool PSSDP::Listen()
{
  PUDPSocket * socket = new PUDPSocket(ssdpPort);
  if (!socket->Listen(ssdpMulticast)) {
    PTRACE(1, "SSDP\tListen failed: " << socket->GetErrorText());
    return false;
  }

  if (!Open(socket, true))
    return false;

  socket->SetSendAddress(ssdpMulticast, ssdpPort);
  m_listening = true;
  return true;
}


bool PSSDP::Close()
{
  m_listening = false;
  return PHTTP::Close();
}


bool PSSDP::Search(const PString & urn, PMIMEInfo & reply)
{
  if (m_listening)
    return false;

  if (!IsOpen()) {
    PUDPSocket * socket = new PUDPSocket(ssdpPort);
    if (!Open(socket, true))
      return false;
    socket->SetSendAddress(ssdpMulticast, ssdpPort);
    SetReadTimeout(1000);
    SetReadLineTimeout(100);
  }

  PMIMEInfo mime;
  mime.SetInteger(MXTag, 3);
  mime.Set(STTag, urn);
  mime.Set(HostTag, ssdpMulticast.AsString());
  mime.Set(MANTag, "\"ssdp:discover\"");

  PTRACE(4, "SSDP\tSent " << commandNames[M_SEARCH] << '\n' << mime);
  if (!WriteCommand(M_SEARCH, "* HTTP/1.1", mime))
    return false;

  int code;
  PString info;
  while (ReadResponse(code, info, reply)) {
    PTRACE(4, "SSDP\tRecevied response code " << code << ' ' << info << '\n' << reply);
    if (code >= 300)
      return false;
    if (code >= 200)
      return true;
  }

  return false;
}


bool PSSDP::GetNotify(PMIMEInfo & mime, const PString & urnRegex)
{
  PINDEX cmd;
  PString args;
  PRegularExpression regex(urnRegex);

  do {
    if (!ReadCommand(cmd, args, mime))
      return false;

    PTRACE(4, "SSDP\tReceived " << commandNames[cmd] << '\n' << mime);
  } while (cmd != PSSDP::NOTIFY || mime.Get(USNTag).FindRegEx(regex) == P_MAX_INDEX);

  return true;
}


// End Of File ///////////////////////////////////////////////////////////////
