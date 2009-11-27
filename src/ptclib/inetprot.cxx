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
    commandNames(cmdCount, cmdNames, true),
    readLineTimeout(0, 10)   // 10 seconds
{
  SetReadTimeout(PTimeInterval(0, 0, 10));  // 10 minutes
  stuffingState = DontStuff;
  newLineToCRLF = true;
  unReadCount = 0;
}


void PInternetProtocol::SetReadLineTimeout(const PTimeInterval & t)
{
  readLineTimeout = t;
}


PBoolean PInternetProtocol::Read(void * buf, PINDEX len)
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


PBoolean PInternetProtocol::Write(const void * buf, PINDEX len)
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
                  return false;
                totalWritten += lastWriteCount;
              }
              if (!PIndirectChannel::Write("\r", 1))
                return false;
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
              return false;
            totalWritten += lastWriteCount;
          }
          if (!PIndirectChannel::Write(".", 1))
            return false;
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
      return false;  
    totalWritten += lastWriteCount;  
  }  
  
  lastWriteCount = totalWritten;  
  return lastWriteCount > 0;
}


PBoolean PInternetProtocol::AttachSocket(PIPSocket * socket)
{
  if (socket->IsOpen()) {
    if (Open(socket))
      return true;
    Close();
    SetErrorValues(Miscellaneous, 0x41000000);
  }
  else {
    SetErrorValues(socket->GetErrorCode(), socket->GetErrorNumber());
    delete socket;
  }

  return false;
}


PBoolean PInternetProtocol::Connect(const PString & address, WORD port)
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


PBoolean PInternetProtocol::Connect(const PString & address, const PString & service)
{
  if (readTimeout == PMaxTimeInterval)
    return AttachSocket(new PTCPSocket(address, service));

  PTCPSocket * s = new PTCPSocket;
  s->SetReadTimeout(readTimeout);
  s->SetPort(service);
  s->Connect(address);
  return AttachSocket(s);
}


PBoolean PInternetProtocol::Accept(PSocket & listener)
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


PBoolean PInternetProtocol::WriteLine(const PString & line)
{
  if (line.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString(line + CRLF);

  PStringArray lines = line.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); i++)
    if (!WriteString(lines[i] + CRLF))
      return false;

  return true;
}


PBoolean PInternetProtocol::ReadLine(PString & str, PBoolean allowContinuation)
{
  str = PString();

  PCharArray line(100);
  PINDEX count = 0;
  PBoolean gotEndOfLine = false;

  int c = ReadChar();
  if (c < 0)
    return false;

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
          gotEndOfLine = true;
        else if (c != ' ' && c != '\t') {
          UnRead(c);
          gotEndOfLine = true;
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


PBoolean PInternetProtocol::WriteCommand(PINDEX cmdNumber)
{
  if (cmdNumber >= commandNames.GetSize())
    return false;
  return WriteLine(commandNames[cmdNumber]);
}


PBoolean PInternetProtocol::WriteCommand(PINDEX cmdNumber, const PString & param)
{
  if (cmdNumber >= commandNames.GetSize())
    return false;
  if (param.IsEmpty())
    return WriteLine(commandNames[cmdNumber]);
  else
    return WriteLine(commandNames[cmdNumber] & param);
}


PBoolean PInternetProtocol::ReadCommand(PINDEX & num, PString & args)
{
  do {
    if (!ReadLine(args))
      return false;
  } while (args.IsEmpty());

  PINDEX endCommand = args.Find(' ');
  if (endCommand == P_MAX_INDEX)
    endCommand = args.GetLength();
  PCaselessString cmd = args.Left(endCommand);

  num = commandNames.GetValuesIndex(cmd);
  if (num != P_MAX_INDEX)
    args = args.Mid(endCommand+1);

  return true;
}


PBoolean PInternetProtocol::WriteResponse(unsigned code, const PString & info)
{
  return WriteResponse(psprintf("%03u", code), info);
}


PBoolean PInternetProtocol::WriteResponse(const PString & code,
                                       const PString & info)
{
  if (info.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString((code & info) + CRLF);

  PStringArray lines = info.Lines();
  PINDEX i;
  for (i = 0; i < lines.GetSize()-1; i++)
    if (!WriteString(code + '-' + lines[i] + CRLF))
      return false;

  return WriteString((code & lines[i]) + CRLF);
}


PBoolean PInternetProtocol::ReadResponse()
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
    return false;
  }

  PINDEX continuePos = ParseResponse(line);
  if (continuePos == 0)
    return true;

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
      return false;
    }
    if (line.Left(continuePos) == prefix)
      lastResponseInfo += line.Mid(continuePos+1);
    else
      lastResponseInfo += line;
  }

  return true;
}


PBoolean PInternetProtocol::ReadResponse(int & code, PString & info)
{
  PBoolean retval = ReadResponse();

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
  PrintContents(strm);
  if (strm.fill() == '\r')
    strm << '\r';
  strm << '\n';
}

ostream & PMIMEInfo::PrintContents(ostream &strm) const
{
  PBoolean output_cr = strm.fill() == '\r';
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
  return strm;
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
    if (line[0] == ' ' || line[0] == '\t') // RFC 2822 section 2.2.2 & 2.2.3
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


PBoolean PMIMEInfo::Read(PInternetProtocol & socket)
{
  RemoveAll();

  PString line;
  while (socket.ReadLine(line, true)) {
    if (line.IsEmpty())
      return true;
    AddMIME(line);
  }

  return false;
}


bool PMIMEInfo::AddMIME(const PString & line)
{
  PINDEX colonPos = line.Find(':');
  if (colonPos == P_MAX_INDEX)
    return false;

  PCaselessString fieldName  = line.Left(colonPos).Trim();
  PString fieldValue = line(colonPos+1, P_MAX_INDEX).Trim();

  return AddMIME(fieldName, fieldValue);
}


bool PMIMEInfo::AddMIME(const PString & fieldName, const PString & fieldValue)
{
  PString * str = GetAt(fieldName);
  if (str == NULL)
    return SetAt(fieldName, fieldValue);

  *str += '\n';
  *str += fieldValue;
  return true;
}


bool PMIMEInfo::AddMIME(const PMIMEInfo & mime)
{
  for (PINDEX i = 0; i < mime.GetSize(); ++i) {
    if (!AddMIME(mime.GetKeyAt(i), mime.GetDataAt(i)))
      return false;
  }

  return true;
}


PBoolean PMIMEInfo::Write(PInternetProtocol & socket) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    PString name = GetKeyAt(i) + ": ";
    PString value = GetDataAt(i);
    if (value.FindOneOf("\r\n") != P_MAX_INDEX) {
      PStringArray vals = value.Lines();
      for (PINDEX j = 0; j < vals.GetSize(); j++) {
        if (!socket.WriteLine(name + vals[j]))
          return false;
      }
    }
    else {
      if (!socket.WriteLine(name + value))
        return false;
    }
  }

  return socket.WriteString(CRLF);
}


PString PMIMEInfo::GetString(const PString & key, const PString & dflt) const
{
  PString * str = GetAt(PCaselessString(key));
  return str != NULL ? *str : dflt;
}


long PMIMEInfo::GetInteger(const PString & key, long dflt) const
{
  PString * str = GetAt(PCaselessString(key));
  return str != NULL ? str->AsInteger() : dflt;
}


void PMIMEInfo::SetInteger(const PCaselessString & key, long value)
{
  SetAt(key, PString(PString::Unsigned, value));
}


bool PMIMEInfo::GetComplex(const PString & key, PStringToString & info) const
{
  PString * ptr = GetAt(PCaselessString(key));
  if (ptr == NULL)
    return false;

  PString field = *ptr;
  if (field[0] == ';')
    return false;

  info.RemoveAll();

  PINDEX semi;
  if (field[0] != '<') {
    semi = field.Find(';');
    info.SetAt(PString::Empty(), field.Left(semi).Trim());
  }
  else {
    PINDEX endtoken = field.Find('>');
    info.SetAt(PString::Empty(), field(1, endtoken-1));
    semi = field.Find(';', endtoken);
  }

  while (semi != P_MAX_INDEX) {
    PINDEX pos = field.FindOneOf("=;", semi);
    PCaselessString tag = field(semi+1, pos-1).Trim();

    if (pos == P_MAX_INDEX) {
      info.SetAt(tag, PString::Empty());
      return true;
    }

    if (field[pos] == ';') {
      info.SetAt(tag, PString::Empty());
      semi = pos + 1;
      continue;
    }

    do {
      ++pos;
    } while (isspace(field[pos]));

    if (pos != '"') {
      PINDEX end = field.Find(';', pos);
      info.SetAt(tag, PCaselessString(field(pos, end-1).RightTrim()));

      if (end == P_MAX_INDEX)
        return true;

      semi = pos + 1;
      continue;
    }

    ++pos;
    PINDEX quote = pos;
    while ((quote = field.Find('"', quote)) != P_MAX_INDEX && field[quote-1] == '\\')
      ++quote;

    PString value = field(pos, quote-1);
    value.Replace("\\", "", true);
    info.SetAt(tag, value);

    semi = field.Find(';', quote);
  }

  return true;
}


bool PMIMEInfo::DecodeMultiPartList(PMultiPartList & parts, const PString & body, const PString & key) const
{
  PStringToString info;
  return GetComplex(key, info) && parts.Decode(body, info);
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
                                      true);
  return contentTypes;
}


void PMIMEInfo::SetAssociation(const PStringToString & allTypes, PBoolean merge)
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


const PString & PMIMEInfo::ContentTypeTag()             { static PString s = "Content-Type";              return s; }
const PString & PMIMEInfo::ContentDispositionTag()      { static PString s = "Content-Disposition";       return s; }
const PString & PMIMEInfo::ContentTransferEncodingTag() { static PString s = "Content-Transfer-Encoding"; return s; }
const PString & PMIMEInfo::ContentDescriptionTag()      { static PString s = "Content-Description";       return s; }
const PString & PMIMEInfo::ContentIdTag()               { static PString s = "Content-ID";                return s; }


//////////////////////////////////////////////////////////////////////////////

bool PMultiPartList::Decode(const PString & entityBody, const PStringToString & contentInfo)
{
  RemoveAll();

  if (entityBody.IsEmpty())
    return false;

  PCaselessString multipartContentType = contentInfo(PString::Empty());
  if (multipartContentType.NumCompare("multipart/") != EqualTo)
    return false;

  if (!contentInfo.Contains("boundary")) {
    PTRACE(2, "MIME\tNo boundary in multipart Content-Type");
    return false;
  }

  PCaselessString startContentId, startContentType;
  if (multipartContentType == "multipart/related") {
    startContentId = contentInfo("start");
    startContentType = contentInfo("type");
  }

  PString seperator = "--" + contentInfo["boundary"];

  PINDEX sepLen = seperator.GetLength();
  const char * sep = (const char *)seperator;

  // split body into parts, assuming binary data
  const char * body = (const char *)entityBody;
  PINDEX entityOffs = 0;
  PINDEX entityLen = entityBody.GetSize()-1;

  bool ignore = true;
  bool last = false;

  while (!last && (entityOffs < entityLen)) {

    // find end of part
    PINDEX partStart = entityOffs;
    PINDEX partLen;
    PBoolean foundSep = false;

    // collect length of part until seperator
    for (partLen = 0; (partStart + partLen) < entityLen; partLen++) {
      if ((partLen >= sepLen) && (memcmp(body + partStart + partLen - sepLen, sep, sepLen) == 0)) {
        foundSep = true;
        break;
      }
    }

    // move entity ptr to the end of the part
    entityOffs = partStart + partLen;

    // if no seperator found, then this is the last part
    // otherwise, look for "--" trailer on seperator and remove CRLF
    if (!foundSep)
      last = true;
    else {
      partLen -= sepLen;

      // determine if this is the last block
      if (((entityOffs + 2) <= entityLen) && (body[entityOffs] == '-') && (body[entityOffs+1] == '-')) {
        last = true;
        entityOffs += 2;
      }

      // remove crlf
      if (((entityOffs + 2) <= entityLen) && (body[entityOffs] == '\r') && (body[entityOffs+1] == '\n')) 
        entityOffs += 2;
    }

    // ignore everything up to the first seperator, 
    // then adjust seperator to include leading CRLF
    if (ignore) {
      ignore = false;
      seperator = PString("\r\n") + seperator;
      sepLen = seperator.GetLength();
      sep = (const char *)seperator;
      continue;
    }

    // extract the MIME header, by looking for a double CRLF
    PINDEX ptr;
    PINDEX nlCount = 0;
    for (ptr = partStart;(ptr < (partStart + partLen)) && (nlCount < 2); ptr++) {
      if (body[ptr] == '\r') {
        nlCount++;
        if ((ptr < entityLen-1) && (body[ptr+1] == '\n'))
          ptr++;
      } else
        nlCount = 0;
    }

    // create the new part info
    PMultiPartInfo * info = new PMultiPartInfo;

    // read MIME information
    PStringStream strm(PString(body + partStart, ptr - partStart));
    info->m_mime.ReadFrom(strm);

    PStringToString typeInfo;
    info->m_mime.GetComplex(PMIMEInfo::ContentTypeTag, typeInfo);
    PCaselessString encoding = info->m_mime.GetString(PMIMEInfo::ContentTransferEncodingTag);

    // save the entity body, being careful of binary files
    if (encoding == "7bit" || encoding == "8bit" || (typeInfo("charset") *= "UTF-8"))
      info->m_textBody = PString(body + ptr, partStart + partLen - ptr);
    else if (encoding == "base64")
      PBase64::Decode(PString(body + ptr, partStart + partLen - ptr), info->m_binaryBody);
    else if (typeInfo("charset") *= "UCS-2")
      info->m_textBody = PString((const wchar_t *)(body + ptr), (partStart + partLen - ptr)/2);
    else
      info->m_binaryBody = PBYTEArray((BYTE *)body + ptr, partStart + partLen - ptr);

    // add the data to the array
    if (startContentId.IsEmpty() || startContentId != info->m_mime.GetString(PMIMEInfo::ContentIdTag))
      Append(info);
    else {
      // Make sure start Content Type is set
      if (!info->m_mime.Contains(PMIMEInfo::ContentTypeTag))
        info->m_mime.SetAt(PMIMEInfo::ContentTypeTag, startContentType);

      // Make sure "start" mime entry is at the beginning of list
      InsertAt(0, info);
      startContentId.MakeEmpty();
    }

  }

  return !IsEmpty();
}


// End Of File ///////////////////////////////////////////////////////////////
