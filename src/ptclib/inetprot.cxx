/*
 * inetprot.cxx
 *
 * Internet Protocol ancestor class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "inetprot.h"
#pragma implementation "mime.h"
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>
#include <ptclib/inetprot.h>
#include <ptclib/mime.h>


static const char * CRLF = "\r\n";


#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// PInternetProtocol

PInternetProtocol::PInternetProtocol(const char * svcName,
                                     PINDEX cmdCount,
                                     char const * const * cmdNames)
  : defaultServiceName(svcName),
    commandNames(cmdCount, cmdNames, TRUE),
    readLineTimeout(0, 10)   // 10 seconds
{
  SetReadTimeout(PTimeInterval(0, 0, 10));  // 10 minutes
  stuffingState = DontStuff;
  newLineToCRLF = TRUE;
  unReadCount = 0;
}


void PInternetProtocol::SetReadLineTimeout(const PTimeInterval & t)
{
  readLineTimeout = t;
}


BOOL PInternetProtocol::Read(void * buf, PINDEX len)
{
  lastReadCount = PMIN(unReadCount, len);
  const char * unReadPtr = ((const char *)unReadBuffer)+unReadCount;
  char * bufptr = (char *)buf;
  while (unReadCount > 0 && len > 0) {
    *bufptr++ = *--unReadPtr;
    unReadCount--;
    len--;
  }

  if (unReadCount == 0)
    unReadBuffer.SetSize(0);

  if (len > 0) {
    PINDEX saveCount = lastReadCount;
    PIndirectChannel::Read(bufptr, len);
    lastReadCount += saveCount;
  }

  return lastReadCount > 0;
}


BOOL PInternetProtocol::Write(const void * buf, PINDEX len)
{
  if (len == 0 || stuffingState == DontStuff)
    return PIndirectChannel::Write(buf, len);

  PINDEX totalWritten = 0;
  const char * base = (const char *)buf;
  const char * current = base;
  while (len-- > 0) {
    switch (stuffingState) {
      case StuffIdle :
        switch (*current) {
          case '\r' :
            stuffingState = StuffCR;
            break;

          case '\n' :
            if (newLineToCRLF) {
              if (current > base) {
                if (!PIndirectChannel::Write(base, current - base))
                  return FALSE;
                totalWritten += lastWriteCount;
              }
              if (!PIndirectChannel::Write("\r", 1))
                return FALSE;
              totalWritten += lastWriteCount;
              base = current;
            }
        }
        break;

      case StuffCR :
        stuffingState = *current != '\n' ? StuffIdle : StuffCRLF;
        break;

      case StuffCRLF :
        if (*current == '.') {
          if (current > base) {
            if (!PIndirectChannel::Write(base, current - base))
              return FALSE;
            totalWritten += lastWriteCount;
          }
          if (!PIndirectChannel::Write(".", 1))
            return FALSE;
          totalWritten += lastWriteCount;
          base = current;
        }
        // Then do default state

      default :
        stuffingState = StuffIdle;
        break;
    }
    current++;
  }

  if (current > base) {  
    if (!PIndirectChannel::Write(base, current - base))  
      return FALSE;  
    totalWritten += lastWriteCount;  
  }  
  
  lastWriteCount = totalWritten;  
  return lastWriteCount > 0;
}


BOOL PInternetProtocol::AttachSocket(PIPSocket * socket)
{
  if (socket->IsOpen()) {
    if (Open(socket))
      return TRUE;
    Close();
    SetErrorValues(Miscellaneous, 0x41000000);
  }
  else {
    SetErrorValues(socket->GetErrorCode(), socket->GetErrorNumber());
    delete socket;
  }

  return FALSE;
}


BOOL PInternetProtocol::Connect(const PString & address, WORD port)
{
  if (port == 0)
    return Connect(address, defaultServiceName);

  if (readTimeout == PMaxTimeInterval)
    return AttachSocket(new PTCPSocket(address, port));

  PTCPSocket * s = new PTCPSocket(port);
  s->SetReadTimeout(readTimeout);
  s->Connect(address);
  return AttachSocket(s);
}


BOOL PInternetProtocol::Connect(const PString & address, const PString & service)
{
  if (readTimeout == PMaxTimeInterval)
    return AttachSocket(new PTCPSocket(address, service));

  PTCPSocket * s = new PTCPSocket;
  s->SetReadTimeout(readTimeout);
  s->SetPort(service);
  s->Connect(address);
  return AttachSocket(s);
}


BOOL PInternetProtocol::Accept(PSocket & listener)
{
  if (readTimeout == PMaxTimeInterval)
    return AttachSocket(new PTCPSocket(listener));

  PTCPSocket * s = new PTCPSocket;
  s->SetReadTimeout(readTimeout);
  s->Accept(listener);
  return AttachSocket(s);
}


const PString & PInternetProtocol::GetDefaultService() const
{
  return defaultServiceName;
}


PIPSocket * PInternetProtocol::GetSocket() const
{
  PChannel * channel = GetBaseReadChannel();
  if (channel != NULL && PIsDescendant(channel, PIPSocket))
    return (PIPSocket *)channel;
  return NULL;
}


BOOL PInternetProtocol::WriteLine(const PString & line)
{
  if (line.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString(line + CRLF);

  PStringArray lines = line.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); i++)
    if (!WriteString(lines[i] + CRLF))
      return FALSE;

  return TRUE;
}


BOOL PInternetProtocol::ReadLine(PString & str, BOOL allowContinuation)
{
  str = PString();

  PCharArray line(100);
  PINDEX count = 0;
  BOOL gotEndOfLine = FALSE;

  int c = ReadChar();
  if (c < 0)
    return FALSE;

  PTimeInterval oldTimeout = GetReadTimeout();
  SetReadTimeout(readLineTimeout);

  while (c >= 0 && !gotEndOfLine) {
    if (unReadCount == 0) {
      char readAhead[1000];
      SetReadTimeout(0);
      if (PIndirectChannel::Read(readAhead, sizeof(readAhead)))
        UnRead(readAhead, GetLastReadCount());
      SetReadTimeout(readLineTimeout);
    }
    switch (c) {
      case '\b' :
      case '\177' :
        if (count > 0)
          count--;
        c = ReadChar();
        break;

      case '\r' :
        c = ReadChar();
        switch (c) {
          case -1 :
          case '\n' :
            break;

          case '\r' :
            c = ReadChar();
            if (c == '\n')
              break;
            UnRead(c);
            c = '\r';
            // Then do default case

          default :
            UnRead(c);
        }
        // Then do line feed case

      case '\n' :
        if (count == 0 || !allowContinuation || (c = ReadChar()) < 0)
          gotEndOfLine = TRUE;
        else if (c != ' ' && c != '\t') {
          UnRead(c);
          gotEndOfLine = TRUE;
        }
        break;

      default :
        if (count >= line.GetSize())
          line.SetSize(count + 100);
        line[count++] = (char)c;
        c = ReadChar();
    }
  }

  SetReadTimeout(oldTimeout);

  if (count > 0)
    str = PString(line, count);
  return gotEndOfLine;
}


void PInternetProtocol::UnRead(int ch)
{
  unReadBuffer.SetSize((unReadCount+256)&~255);
  unReadBuffer[unReadCount++] = (char)ch;
}


void PInternetProtocol::UnRead(const PString & str)
{
  UnRead((const char *)str, str.GetLength());
}


void PInternetProtocol::UnRead(const void * buffer, PINDEX len)
{
  char * unreadptr =
               unReadBuffer.GetPointer((unReadCount+len+255)&~255)+unReadCount;
  const char * bufptr = ((const char *)buffer)+len;
  unReadCount += len;
  while (len-- > 0)
    *unreadptr++ = *--bufptr;
}


BOOL PInternetProtocol::WriteCommand(PINDEX cmdNumber)
{
  if (cmdNumber >= commandNames.GetSize())
    return FALSE;
  return WriteLine(commandNames[cmdNumber]);
}


BOOL PInternetProtocol::WriteCommand(PINDEX cmdNumber, const PString & param)
{
  if (cmdNumber >= commandNames.GetSize())
    return FALSE;
  if (param.IsEmpty())
    return WriteLine(commandNames[cmdNumber]);
  else
    return WriteLine(commandNames[cmdNumber] & param);
}


BOOL PInternetProtocol::ReadCommand(PINDEX & num, PString & args)
{
  do {
    if (!ReadLine(args))
      return FALSE;
  } while (args.IsEmpty());

  PINDEX endCommand = args.Find(' ');
  if (endCommand == P_MAX_INDEX)
    endCommand = args.GetLength();
  PCaselessString cmd = args.Left(endCommand);

  num = commandNames.GetValuesIndex(cmd);
  if (num != P_MAX_INDEX)
    args = args.Mid(endCommand+1);

  return TRUE;
}


BOOL PInternetProtocol::WriteResponse(unsigned code, const PString & info)
{
  return WriteResponse(psprintf("%03u", code), info);
}


BOOL PInternetProtocol::WriteResponse(const PString & code,
                                       const PString & info)
{
  if (info.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString(code & info + CRLF);

  PStringArray lines = info.Lines();
  PINDEX i;
  for (i = 0; i < lines.GetSize()-1; i++)
    if (!WriteString(code + '-' + lines[i] + CRLF))
      return FALSE;

  return WriteString(code & lines[i] + CRLF);
}


BOOL PInternetProtocol::ReadResponse()
{
  PString line;
  if (!ReadLine(line)) {
    lastResponseCode = -1;
    if (GetErrorCode(LastReadError) != NoError)
      lastResponseInfo = GetErrorText(LastReadError);
    else {
      lastResponseInfo = "Remote shutdown";
      SetErrorValues(ProtocolFailure, 0, LastReadError);
    }
    return FALSE;
  }

  PINDEX continuePos = ParseResponse(line);
  if (continuePos == 0)
    return TRUE;

  PString prefix = line.Left(continuePos);
  char continueChar = line[continuePos];
  while (line[continuePos] == continueChar ||
         (!isdigit(line[0]) && strncmp(line, prefix, continuePos) != 0)) {
    lastResponseInfo += '\n';
    if (!ReadLine(line)) {
      if (GetErrorCode(LastReadError) != NoError)
        lastResponseInfo += GetErrorText(LastReadError);
      else
        SetErrorValues(ProtocolFailure, 0, LastReadError);
      return FALSE;
    }
    if (line.Left(continuePos) == prefix)
      lastResponseInfo += line.Mid(continuePos+1);
    else
      lastResponseInfo += line;
  }

  return TRUE;
}


BOOL PInternetProtocol::ReadResponse(int & code, PString & info)
{
  BOOL retval = ReadResponse();

  code = lastResponseCode;
  info = lastResponseInfo;

  return retval;
}


PINDEX PInternetProtocol::ParseResponse(const PString & line)
{
  PINDEX endCode = line.FindOneOf(" -");
  if (endCode == P_MAX_INDEX) {
    lastResponseCode = -1;
    lastResponseInfo = line;
    return 0;
  }

  lastResponseCode = line.Left(endCode).AsInteger();
  lastResponseInfo = line.Mid(endCode+1);
  return line[endCode] != ' ' ? endCode : 0;
}


int PInternetProtocol::ExecuteCommand(PINDEX cmd)
{
  return ExecuteCommand(cmd, PString());
}


int PInternetProtocol::ExecuteCommand(PINDEX cmd,
                                       const PString & param)
{
  PTimeInterval oldTimeout = GetReadTimeout();
  SetReadTimeout(0);
  while (ReadChar() >= 0)
    ;
  SetReadTimeout(oldTimeout);
  return WriteCommand(cmd, param) && ReadResponse() ? lastResponseCode : -1;
}


int PInternetProtocol::GetLastResponseCode() const
{
  return lastResponseCode;
}


PString PInternetProtocol::GetLastResponseInfo() const
{
  return lastResponseInfo;
}


//////////////////////////////////////////////////////////////////////////////
// PMIMEInfo

PMIMEInfo::PMIMEInfo(istream & strm)
{
  ReadFrom(strm);
}


PMIMEInfo::PMIMEInfo(PInternetProtocol & socket)
{
  Read(socket);
}


void PMIMEInfo::PrintOn(ostream &strm) const
{
  BOOL output_cr = strm.fill() == '\r';
  strm.fill(' ');
  for (PINDEX i = 0; i < GetSize(); i++) {
    PString name = GetKeyAt(i) + ": ";
    PString value = GetDataAt(i);
    if (value.FindOneOf("\r\n") != P_MAX_INDEX) {
      PStringArray vals = value.Lines();
      for (PINDEX j = 0; j < vals.GetSize(); j++) {
        strm << name << vals[j];
        if (output_cr)
          strm << '\r';
        strm << '\n';
      }
    }
    else {
      strm << name << value;
      if (output_cr)
        strm << '\r';
      strm << '\n';
    }
  }
  if (output_cr)
    strm << '\r';
  strm << '\n';
}


void PMIMEInfo::ReadFrom(istream &strm)
{
  RemoveAll();

  PString line;
  PString lastLine;
  while (!strm.bad() && !strm.eof()) {
    strm >> line;
    if (line.IsEmpty())
      break;
    if (line[0] == ' ') 
      lastLine += line;
    else {
      AddMIME(lastLine);
      lastLine = line;
    }
  }

  if (!lastLine.IsEmpty()) {
    AddMIME(lastLine);
  }
}


BOOL PMIMEInfo::Read(PInternetProtocol & socket)
{
  RemoveAll();

  PString line;
  while (socket.ReadLine(line, TRUE)) {
    if (line.IsEmpty())
      return TRUE;
    AddMIME(line);
  }

  return FALSE;
}


BOOL PMIMEInfo::AddMIME(const PString & line)
{
  PINDEX colonPos = line.Find(':');
  if (colonPos == P_MAX_INDEX)
    return FALSE;

  PCaselessString fieldName  = line.Left(colonPos).Trim();
  PString fieldValue = line(colonPos+1, P_MAX_INDEX).Trim();

  return AddMIME(fieldName, fieldValue);
}

BOOL PMIMEInfo::AddMIME(const PString & fieldName, const PString & _fieldValue)
{
  PString fieldValue(_fieldValue);

  if (Contains(fieldName))
    fieldValue = (*this)[fieldName] + '\n' + fieldValue;

  SetAt(fieldName, fieldValue);

  return TRUE;
}


BOOL PMIMEInfo::Write(PInternetProtocol & socket) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PString name = GetKeyAt(i) + ": ";
    PString value = GetDataAt(i);
    if (value.FindOneOf("\r\n") != P_MAX_INDEX) {
      PStringArray vals = value.Lines();
      for (PINDEX j = 0; j < vals.GetSize(); j++) {
        if (!socket.WriteLine(name + vals[j]))
          return FALSE;
      }
    }
    else {
      if (!socket.WriteLine(name + value))
        return FALSE;
    }
  }

  return socket.WriteString(CRLF);
}


PString PMIMEInfo::GetString(const PString & key, const PString & dflt) const
{
  if (GetAt(PCaselessString(key)) == NULL)
    return dflt;
  return operator[](key);
}


long PMIMEInfo::GetInteger(const PString & key, long dflt) const
{
  if (GetAt(PCaselessString(key)) == NULL)
    return dflt;
  return operator[](key).AsInteger();
}


void PMIMEInfo::SetInteger(const PCaselessString & key, long value)
{
  SetAt(key, PString(PString::Unsigned, value));
}


static const PStringToString::Initialiser DefaultContentTypes[] = {
  { ".txt", "text/plain" },
  { ".text", "text/plain" },
  { ".html", "text/html" },
  { ".htm", "text/html" },
  { ".aif", "audio/aiff" },
  { ".aiff", "audio/aiff" },
  { ".au", "audio/basic" },
  { ".snd", "audio/basic" },
  { ".wav", "audio/wav" },
  { ".gif", "image/gif" },
  { ".xbm", "image/x-bitmap" },
  { ".tif", "image/tiff" },
  { ".tiff", "image/tiff" },
  { ".jpg", "image/jpeg" },
  { ".jpe", "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".avi", "video/avi" },
  { ".mpg", "video/mpeg" },
  { ".mpeg", "video/mpeg" },
  { ".qt", "video/quicktime" },
  { ".mov", "video/quicktime" }
};

PStringToString & PMIMEInfo::GetContentTypes()
{
  static PStringToString contentTypes(PARRAYSIZE(DefaultContentTypes),
                                      DefaultContentTypes,
                                      TRUE);
  return contentTypes;
}


void PMIMEInfo::SetAssociation(const PStringToString & allTypes, BOOL merge)
{
  PStringToString & types = GetContentTypes();
  if (!merge)
    types.RemoveAll();
  for (PINDEX i = 0; i < allTypes.GetSize(); i++)
    types.SetAt(allTypes.GetKeyAt(i), allTypes.GetDataAt(i));
}


PString PMIMEInfo::GetContentType(const PString & fType)
{
  if (fType.IsEmpty())
    return "text/plain";

  PStringToString & types = GetContentTypes();
  if (types.Contains(fType))
    return types[fType];

  return "application/octet-stream";
}

// End Of File ///////////////////////////////////////////////////////////////
