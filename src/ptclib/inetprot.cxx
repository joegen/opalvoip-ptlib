/*
 * $Id: inetprot.cxx,v 1.12 1996/02/25 11:16:07 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: inetprot.cxx,v $
 * Revision 1.12  1996/02/25 11:16:07  robertj
 * Fixed bug in ReadResponse() for multi-line responses under FTP..
 *
 * Revision 1.11  1996/02/25 03:05:12  robertj
 * Added decoding of Base64 to a block of memory instead of PBYTEArray.
 *
 * Revision 1.10  1996/02/19 13:31:26  robertj
 * Changed stuff to use new & operator..
 *
 * Revision 1.9  1996/02/15 14:42:41  robertj
 * Fixed warning for long to int conversion.
 *
 * Revision 1.8  1996/02/13 12:57:49  robertj
 * Added access to the last response in an application socket.
 *
 * Revision 1.7  1996/02/03 11:33:17  robertj
 * Changed RadCmd() so can distinguish between I/O error and unknown command.
 *
 * Revision 1.6  1996/01/28 14:11:11  robertj
 * Fixed bug in MIME content types for non caseless strings.
 * Added default value in string for service name.
 *
 * Revision 1.5  1996/01/28 02:48:27  robertj
 * Removal of MemoryPointer classes as usage didn't work for GNU.
 *
 * Revision 1.4  1996/01/26 02:24:29  robertj
 * Further implemetation.
 *
 * Revision 1.3  1996/01/23 13:18:43  robertj
 * Major rewrite for HTTP support.
 *
 * Revision 1.2  1995/11/09 12:19:29  robertj
 * Fixed missing state assertion in state machine.
 *
 * Revision 1.1  1995/06/17 00:50:37  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <mailsock.h>


static const PString CRLF = "\r\n";
static const PString CRLFdotCRLF = "\r\n.\r\n";


//////////////////////////////////////////////////////////////////////////////
// PApplicationSocket

PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       WORD port)
  : PTCPSocket(port), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & service)
  : PTCPSocket(service), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       WORD port)
  : PTCPSocket(address, port), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       const PString & service)
  : PTCPSocket(address, service), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       PSocket & socket)
  : PTCPSocket(socket), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


void PApplicationSocket::Construct()
{
  SetReadTimeout(PTimeInterval(0, 0, 10));  // 10 minutes
  readLineTimeout = PTimeInterval(0, 10);   // 10 seconds
  stuffingState = DontStuff;
  newLineToCRLF = TRUE;
}


BOOL PApplicationSocket::Read(void * buf, PINDEX len)
{
  PINDEX numUnRead = unReadBuffer.GetSize();
  if (numUnRead >= len) {
    memcpy(buf, unReadBuffer, len);
    if (numUnRead > len)
      memcpy(unReadBuffer.GetPointer(), &unReadBuffer[len], numUnRead - len);
    unReadBuffer.SetSize(numUnRead - len);
    lastReadCount = len;
    return TRUE;
  }

  if (numUnRead > 0) {
    memcpy(buf, unReadBuffer, numUnRead);
    unReadBuffer.SetSize(0);
  }

  PTCPSocket::Read(((char *)buf) + numUnRead, len - numUnRead);

  lastReadCount += numUnRead;
  return lastReadCount > 0;
}


BOOL PApplicationSocket::Write(const void * buf, PINDEX len)
{
  if (len == 0 || stuffingState == DontStuff)
    return PTCPSocket::Write(buf, len);

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
                if (!PTCPSocket::Write(base, current - base))
                  return FALSE;
                totalWritten += lastWriteCount;
              }
              if (!PTCPSocket::Write("\r", 1))
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
            if (!PTCPSocket::Write(base, current - base))
              return FALSE;
            totalWritten += lastWriteCount;
          }
          if (!PTCPSocket::Write(".", 1))
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

  if (current > base)
    if (!PTCPSocket::Write(base, current - base))
      return FALSE;

  lastWriteCount += totalWritten;
  return lastWriteCount > 0;
}


BOOL PApplicationSocket::WriteLine(const PString & line)
{
  if (line.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString(line + CRLF);

  PStringArray lines = line.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); i++)
    if (!WriteString(lines[i] + CRLF))
      return FALSE;

  return TRUE;
}


BOOL PApplicationSocket::ReadLine(PString & str, BOOL allowContinuation)
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
    switch (c) {
      case '\b' :
      case '\177' :
        if (count > 0)
          count--;
        c = ReadChar();
        break;

      case '\r' :
        c = ReadChar();
        if (c >= 0 && c != '\n')
          UnRead(c);
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


void PApplicationSocket::UnRead(int ch)
{
  PINDEX size = unReadBuffer.GetSize();
  unReadBuffer.SetSize(size+1);
  unReadBuffer[size] = (char)ch;
}


void PApplicationSocket::UnRead(void * buffer, PINDEX len)
{
  PINDEX size = unReadBuffer.GetSize();
  memcpy(unReadBuffer.GetPointer(size+len)+size, buffer, len);
}


BOOL PApplicationSocket::WriteCommand(PINDEX cmdNumber,  const PString & param)
{
  if (cmdNumber >= commandNames.GetSize())
    return FALSE;
  if (param.IsEmpty())
    return WriteLine(commandNames[cmdNumber]);
  else
    return WriteLine(commandNames[cmdNumber] & param);
}


BOOL PApplicationSocket::ReadCommand(PINDEX & num, PString & args)
{
  if (!ReadLine(args))
    return FALSE;

  PINDEX endCommand = args.Find(' ');
  if (endCommand == P_MAX_INDEX)
    endCommand = args.GetLength();
  PCaselessString cmd = args.Left(endCommand);

  num = commandNames.GetValuesIndex(cmd);
  if (num != P_MAX_INDEX)
    args = args.Mid(endCommand+1);

  return TRUE;
}


BOOL PApplicationSocket::WriteResponse(unsigned code, const PString & info)
{
  return WriteResponse(psprintf("%03u", code), info);
}


BOOL PApplicationSocket::WriteResponse(const PString & code,
                                       const PString & info)
{
  if (info.FindOneOf(CRLF) == P_MAX_INDEX)
    return WriteString(code & info + CRLF);

  PStringArray lines = info.Lines();
  for (PINDEX i = 0; i < lines.GetSize()-1; i++)
    if (!WriteString(code + '-' + lines[i] + CRLF))
      return FALSE;

  return WriteString(code & lines[i] + CRLF);
}


BOOL PApplicationSocket::ReadResponse()
{
  return ReadResponse(lastResponseCode, lastResponseInfo);
}


BOOL PApplicationSocket::ReadResponse(PString & code, PString & info)
{
  PString line;
  if (!ReadLine(line))
    return FALSE;

  PINDEX endCode = line.FindOneOf(" -", 1);
  if (endCode == P_MAX_INDEX) {
    code = line;
    info = PString();
    return TRUE;
  }

  code = line.Left(endCode);
  info = line.Mid(endCode+1);

  while (!isdigit(line[0]) || line[endCode] == '-') {
    info += '\n';
    if (!ReadLine(line))
      return FALSE;
    info += line.Mid(endCode+1);
  }

  lastResponseInfo = info;
  lastResponseCode = code;

  return TRUE;
}


char PApplicationSocket::ExecuteCommand(PINDEX cmdNumber,
                                        const PString & param)
{
  if (!WriteCommand(cmdNumber, param))
    return '\0';

  if (!ReadResponse())
    return '\0';

  return lastResponseCode[0];
}

PINDEX PApplicationSocket::GetLastResponseCode() const
{
  return (PINDEX)lastResponseCode.AsInteger();
}

PString PApplicationSocket::GetLastResponseInfo() const
{
  return lastResponseInfo;
}

//////////////////////////////////////////////////////////////////////////////
// PMIMEInfo

PMIMEInfo::PMIMEInfo(istream & strm)
{
  ReadFrom(strm);
}


PMIMEInfo::PMIMEInfo(PApplicationSocket & socket)
{
  Read(socket);
}


void PMIMEInfo::PrintOn(ostream &strm) const
{
  for (PINDEX i = 0; i < GetSize(); i++)
    strm << GetKeyAt(i) << ": " << GetDataAt(i) << CRLF;
  strm << CRLF;
}


void PMIMEInfo::ReadFrom(istream &strm)
{
  PString line;
  while (strm.good()) {
    strm >> line;
    if (line.IsEmpty())
      break;

    PINDEX colonPos = line.Find(':');
    if (colonPos != P_MAX_INDEX) {
      PCaselessString fieldName  = line.Left(colonPos).Trim();
      PString fieldValue = line(colonPos+1, P_MAX_INDEX).Trim();
      SetAt(fieldName, fieldValue);
    }
  }
}


BOOL PMIMEInfo::Read(PApplicationSocket & socket)
{
  PString line;
  while (socket.ReadLine(line, TRUE)) {
    if (line.IsEmpty())
      return TRUE;

    PINDEX colonPos = line.Find(':');
    if (colonPos != P_MAX_INDEX) {
      PCaselessString fieldName  = line.Left(colonPos).Trim();
      PString fieldValue = line(colonPos+1, P_MAX_INDEX).Trim();
      SetAt(fieldName, fieldValue);
    }
  }

  return FALSE;
}


BOOL PMIMEInfo::Write(PApplicationSocket & socket)
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (!socket.WriteLine(GetKeyAt(i) + ": " + GetDataAt(i)))
      return FALSE;
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

PStringToString PMIMEInfo::contentTypes(PARRAYSIZE(DefaultContentTypes),
                                        DefaultContentTypes,
                                        TRUE);


void PMIMEInfo::SetAssociation(const PStringToString & allTypes, BOOL merge)
{
  if (!merge)
    contentTypes.RemoveAll();
  for (PINDEX i = 0; i < allTypes.GetSize(); i++)
    contentTypes.SetAt(allTypes.GetKeyAt(i), allTypes.GetDataAt(i));
}


PString PMIMEInfo::GetContentType(const PString & fileType)
{
  if (contentTypes.GetAt(PCaselessString(fileType)) != NULL)
    return contentTypes[fileType];
  return "application/octet-stream";
}


///////////////////////////////////////////////////////////////////////////////
// PBase64

PBase64::PBase64()
{
  StartEncoding();
  StartDecoding();
}


void PBase64::StartEncoding(BOOL useCRLF)
{
  encodedString = "";
  encodeLength = nextLine = saveCount = 0;
  useCRLFs = useCRLF;
}


void PBase64::ProcessEncoding(const PString & str)
{
  ProcessEncoding((const char *)str);
}


void PBase64::ProcessEncoding(const char * cstr)
{
  ProcessEncoding((const BYTE *)cstr, strlen(cstr));
}


void PBase64::ProcessEncoding(const PBYTEArray & data)
{
  ProcessEncoding(data, data.GetSize());
}


static const char Binary2Base64[65] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void PBase64::OutputBase64(const BYTE * data)
{
  char * out = encodedString.GetPointer((encodeLength&-256) + 256);

  out[encodeLength++] = Binary2Base64[data[0] >> 2];
  out[encodeLength++] = Binary2Base64[((data[0]&3)<<4) | (data[1]>>4)];
  out[encodeLength++] = Binary2Base64[((data[1]&15)<<2) | (data[2]>>6)];
  out[encodeLength++] = Binary2Base64[data[2]&0x3f];

  if (++nextLine > 76) {
    if (useCRLFs)
      out[encodeLength++] = '\r';
    out[encodeLength++] = '\n';
    nextLine = 0;
  }
}


void PBase64::ProcessEncoding(const BYTE * data, PINDEX length)
{
  while (saveCount < 3) {
    saveTriple[saveCount++] = *data++;
    if (--length <= 0)
      return;
  }

  OutputBase64(saveTriple);

  for (PINDEX i = 0; i+2 < length; i += 3)
    OutputBase64(data+i);

  saveCount = length - i;
  switch (saveCount) {
    case 2 :
      saveTriple[0] = data[i++];
      saveTriple[1] = data[i];
      break;
    case 1 :
      saveTriple[0] = data[i];
  }
}


PString PBase64::GetEncodedString()
{
  PString retval = encodedString;
  encodedString = "";
  encodeLength = 0;
  return retval;
}


PString PBase64::CompleteEncoding()
{
  char * out = encodedString.GetPointer(encodeLength + 5)+encodeLength;

  switch (saveCount) {
    case 1 :
      *out++ = Binary2Base64[saveTriple[0] >> 2];
      *out++ = Binary2Base64[(saveTriple[0]&3)<<4];
      *out++ = '=';
      *out   = '=';
      break;

    case 2 :
      *out++ = Binary2Base64[saveTriple[0] >> 2];
      *out++ = Binary2Base64[((saveTriple[0]&3)<<4) | (saveTriple[1]>>4)];
      *out++ = Binary2Base64[((saveTriple[1]&15)<<2)];
      *out   = '=';
  }

  return encodedString;
}


PString PBase64::Encode(const PString & str)
{
  return Encode((const char *)str);
}


PString PBase64::Encode(const char * cstr)
{
  return Encode((const BYTE *)cstr, strlen(cstr));
}


PString PBase64::Encode(const PBYTEArray & data)
{
  return Encode(data, data.GetSize());
}


PString PBase64::Encode(const BYTE * data, PINDEX length)
{
  PBase64 encoder;
  encoder.ProcessEncoding(data, length);
  return encoder.CompleteEncoding();
}


void PBase64::StartDecoding()
{
  perfectDecode = TRUE;
  quadPosition = 0;
  decodedData.SetSize(0);
  decodeSize = 0;
}


BOOL PBase64::ProcessDecoding(const PString & str)
{
  return ProcessDecoding((const char *)str);
}


BOOL PBase64::ProcessDecoding(const char * cstr)
{
  static const BYTE Base642Binary[256] = {
    96, 99, 99, 99, 99, 99, 99, 99, 99, 99, 98, 99, 99, 98, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 97, 99, 99,
    99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99,
    99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
  };

  for (;;) {
    BYTE value = Base642Binary[(BYTE)*cstr++];
    switch (value) {
      case 96 : // end of string
        return FALSE;

      case 97 : // '=' sign
        if (quadPosition == 3 || (quadPosition == 2 && *cstr == '='))
          return TRUE; // Stop decoding now as must be at end of data
        perfectDecode = FALSE;  // Ignore '=' sign but flag decode as suspect
        break;

      case 98 : // CRLFs
        break;  // Ignore totally

      case 99 :  // Illegal characters
        perfectDecode = FALSE;  // Ignore rubbish but flag decode as suspect
        break;

      default : // legal value from 0 to 63
        BYTE * out = decodedData.GetPointer(((decodeSize+1)&-256) + 256);
        switch (quadPosition) {
          case 0 :
            out[decodeSize] = (BYTE)(value << 2);
            break;
          case 1 :
            out[decodeSize++] |= (BYTE)(value >> 4);
            out[decodeSize] = (BYTE)((value&15) << 4);
            break;
          case 2 :
            out[decodeSize++] |= (BYTE)(value >> 2);
            out[decodeSize] = (BYTE)((value&3) << 6);
            break;
          case 3 :
            out[decodeSize++] |= (BYTE)value;
            break;
        }
        quadPosition = (quadPosition+1)&3;
    }
  }
}


PBYTEArray PBase64::GetDecodedData()
{
  decodedData.SetSize(decodeSize);
  PBYTEArray retval = decodedData;
  retval.MakeUnique();
  decodedData.SetSize(0);
  decodeSize = 0;
  return retval;
}


BOOL PBase64::GetDecodedData(BYTE * dataBlock, PINDEX length)
{
  if (decodeSize > length) {
    decodeSize = length;
    perfectDecode = FALSE;
  }
  memcpy(dataBlock, decodedData, decodeSize);
  decodedData.SetSize(0);
  decodeSize = 0;
  return perfectDecode;
}


PString PBase64::Decode(const PString & str)
{
  PBYTEArray data;
  Decode(str, data);
  return PString((const char *)(const BYTE *)data, data.GetSize());
}


BOOL PBase64::Decode(const PString & str, PBYTEArray & data)
{
  PBase64 decoder;
  decoder.ProcessDecoding(str);
  data = decoder.GetDecodedData();
  return decoder.IsDecodeOK();
}


BOOL PBase64::Decode(const PString & str, BYTE * dataBlock, PINDEX length)
{
  PBase64 decoder;
  decoder.ProcessDecoding(str);
  return decoder.GetDecodedData(dataBlock, length);
}


//////////////////////////////////////////////////////////////////////////////
// PSMTPSocket

static char const * SMTPCommands[PSMTPSocket::NumCommands] = {
  "HELO", "EHLO", "QUIT", "HELP", "NOOP",
  "TURN", "RSET", "VRFY", "EXPN", "RCPT",
  "MAIL", "SEND", "SAML", "SOML", "DATA"
};


PSMTPSocket::PSMTPSocket()
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp 25")
{
  Construct();
}


PSMTPSocket::PSMTPSocket(const PString & address)
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp 25")
{
  Construct();
  Connect(address);
}


PSMTPSocket::PSMTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, SMTPCommands)
{
  Construct();
  Accept(socket);
}


PSMTPSocket::~PSMTPSocket()
{
  Close();
}


void PSMTPSocket::Construct()
{
  haveHello = FALSE;
  extendedHello = FALSE;
  messageBufferSize = 30000;
  ServerReset();
}


void PSMTPSocket::ServerReset()
{
  eightBitMIME = FALSE;
  sendCommand = WasMAIL;
  fromName = PString();
  toNames.RemoveAll();
}


BOOL PSMTPSocket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (ReadResponse() && lastResponseCode[0] == '2')
    return TRUE;

  Close();
  return FALSE;
}


BOOL PSMTPSocket::Accept(PSocket & socket)
{
  if (!PApplicationSocket::Accept(socket))
    return FALSE;

  return WriteResponse(220, GetLocalHostName() +
                     " Sendmail v1.61/WinSMTPSrv ready at " +
                      PTime(PTime::MediumDateTime).AsString());
}


BOOL PSMTPSocket::Close()
{
  BOOL ok = TRUE;
  if (IsOpen() && haveHello) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "") == '2';
  }
  return PApplicationSocket::Close() && ok;
}


BOOL PSMTPSocket::BeginMessage(const PString & from,
                               const PString & to,
                               BOOL useEightBitMIME)
{
  fromName = from;
  toNames.RemoveAll();
  toNames.AppendString(to);
  eightBitMIME = useEightBitMIME;
  return _BeginMessage();
}


BOOL PSMTPSocket::BeginMessage(const PString & from,
                               const PStringList & toList,
                               BOOL useEightBitMIME)
{
  fromName = from;
  toNames = toList;
  eightBitMIME = useEightBitMIME;
  return _BeginMessage();
}


BOOL PSMTPSocket::_BeginMessage()
{
  if (!haveHello) {
    if (ExecuteCommand(EHLO, GetLocalHostName()) == '2')
      haveHello = extendedHello = TRUE;
  }

  if (!haveHello) {
    extendedHello = FALSE;
    if (eightBitMIME)
      return FALSE;
    if (ExecuteCommand(HELO, GetLocalHostName()) != '2')
      return FALSE;
    haveHello = TRUE;
  }

  if (fromName[0] != '"' && fromName.Find(' ') != P_MAX_INDEX)
    fromName = '"' + fromName + '"';
  if (fromName.Find('@') == P_MAX_INDEX)
    fromName += '@' + GetLocalHostName();
  if (ExecuteCommand(MAIL, "FROM:<" + fromName + '>') != '2')
    return FALSE;

  for (PINDEX i = 0; i < toNames.GetSize(); i++) {
    if (toNames[i].Find('@') == P_MAX_INDEX)
      toNames[i] += '@' + GetPeerHostName();
    if (ExecuteCommand(RCPT, "TO:<" + toNames[i] + '>') != '2')
      return FALSE;
  }

  if (ExecuteCommand(DATA, PString()) != '3')
    return FALSE;

  stuffingState = StuffIdle;
  return TRUE;
}


BOOL PSMTPSocket::EndMessage()
{
  flush();
  stuffingState = DontStuff;
  if (!WriteString(CRLFdotCRLF))
    return FALSE;
  return ReadResponse() && lastResponseCode[0] == '2';
}


BOOL PSMTPSocket::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return FALSE;

  switch (num) {
    case HELO :
      OnHELO(args);
      break;
    case EHLO :
      OnEHLO(args);
      break;
    case QUIT :
      OnQUIT();
      return FALSE;
    case NOOP :
      OnNOOP();
      break;
    case TURN :
      OnTURN();
      break;
    case RSET :
      OnRSET();
      break;
    case VRFY :
      OnVRFY(args);
      break;
    case EXPN :
      OnEXPN(args);
      break;
    case RCPT :
      OnRCPT(args);
      break;
    case MAIL :
      OnMAIL(args);
      break;
    case SEND :
      OnSEND(args);
      break;
    case SAML :
      OnSAML(args);
      break;
    case SOML :
      OnSOML(args);
      break;
    case DATA :
      OnDATA();
      break;
    default :
      return OnUnknown(args);
  }

  return TRUE;
}


void PSMTPSocket::OnHELO(const PCaselessString & remoteHost)
{
  extendedHello = FALSE;
  ServerReset();

  PCaselessString peer = GetPeerHostName();

  PString response = GetLocalHostName() + " Hello " + peer + ", ";

  if (remoteHost == peer)
    response += "pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response);
}


void PSMTPSocket::OnEHLO(const PCaselessString & remoteHost)
{
  extendedHello = TRUE;
  ServerReset();

  PCaselessString peer = GetPeerHostName();

  PString response = GetLocalHostName() + " Hello " + peer + ", ";

  if (remoteHost == peer)
    response += ", pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response +
               "\nHELP\nVERB\nONEX\nMULT\nEXPN\nTICK\n8BITMIME\n");
}


void PSMTPSocket::OnQUIT()
{
  WriteResponse(221, GetLocalHostName() + " closing connection, goodbye.");
  Close();
}


void PSMTPSocket::OnHELP()
{
  WriteResponse(214, "No help here.");
}


void PSMTPSocket::OnNOOP()
{
  WriteResponse(250, "Ok");
}


void PSMTPSocket::OnTURN()
{
  WriteResponse(502, "I don't do that yet. Sorry.");
}


void PSMTPSocket::OnRSET()
{
  ServerReset();
  WriteResponse(250, "Reset state.");
}


void PSMTPSocket::OnVRFY(const PCaselessString & name)
{
  PString expandedName;
  switch (LookUpName(name, expandedName)) {
    case AmbiguousUser :
      WriteResponse(553, "User \"" + name + "\" ambiguous.");
      break;

    case ValidUser :
      WriteResponse(250, expandedName);
      break;

    case UnknownUser :
      WriteResponse(550, "Name \"" + name + "\" does not match anything.");
      break;

    default :
      WriteResponse(550, "Error verifying user \"" + name + "\".");
  }
}


void PSMTPSocket::OnEXPN(const PCaselessString &)
{
  WriteResponse(502, "I don't do that. Sorry.");
}


static PINDEX ExtractName(const PCaselessString & args,
                          const PCaselessString & subCmd,
                          PString & name, PString & host)
{
  PINDEX colon = args.Find(':');
  if (colon == P_MAX_INDEX)
    return 0;

  PCaselessString word = args.Left(colon).Trim();
  if (subCmd != word)
    return 0;

  PINDEX leftAngle = args.Find('<', colon);
  if (leftAngle == P_MAX_INDEX)
    return 0;

  PINDEX finishQuote = P_MAX_INDEX;
  PINDEX startQuote = args.Find('"', leftAngle);
  if (startQuote == P_MAX_INDEX)
    finishQuote = startQuote = leftAngle+1;
  else {
    finishQuote = args.Find('"', startQuote+1);
    if (finishQuote == P_MAX_INDEX)
      finishQuote = startQuote;
  }

  PINDEX at = args.Find('@', finishQuote);
  if (at == P_MAX_INDEX)
    at = finishQuote;

  PINDEX rightAngle = args.Find('>', at);
  if (rightAngle == P_MAX_INDEX)
    return 0;

  if (at == finishQuote)
    at = rightAngle-1;

  if (startQuote == finishQuote)
    finishQuote = rightAngle;

  name = args(startQuote, finishQuote-1);
  host = args(at, rightAngle-1);

  return rightAngle+1;
}


void PSMTPSocket::OnRCPT(const PCaselessString & recipient)
{
  PCaselessString toName;
  PCaselessString toHost;
  if (ExtractName(recipient, "to", toName, toHost) == 0)
    WriteResponse(501, "Syntax error.");
  else if (toName.Find(':') != P_MAX_INDEX)
    WriteResponse(550, "Cannot do forwarding.");
  else {
    PString expandedName;
    switch (LookUpName(toName, expandedName)) {
      case ValidUser :
        WriteResponse(250, "Recipient " + toName + " Ok");
        toNames.AppendString(toName);
        toHosts.AppendString(toHost);
        break;

      case AmbiguousUser :
        WriteResponse(553, "User ambiguous.");
        break;

      case UnknownUser :
        WriteResponse(550, "User unknown.");
        break;

      default :
        WriteResponse(550, "Error verifying user.");
    }
  }
}


void PSMTPSocket::OnMAIL(const PCaselessString & sender)
{
  sendCommand = WasMAIL;
  OnSendMail(sender);
}


void PSMTPSocket::OnSEND(const PCaselessString & sender)
{
  sendCommand = WasSEND;
  OnSendMail(sender);
}


void PSMTPSocket::OnSAML(const PCaselessString & sender)
{
  sendCommand = WasSAML;
  OnSendMail(sender);
}


void PSMTPSocket::OnSOML(const PCaselessString & sender)
{
  sendCommand = WasSOML;
  OnSendMail(sender);
}


void PSMTPSocket::OnSendMail(const PCaselessString & sender)
{
  if (!fromName.IsEmpty()) {
    WriteResponse(503, "Sender already specified.");
    return;
  }

  PINDEX extendedArgPos = ExtractName(sender, "from", fromName, fromHost);
  if (extendedArgPos == 0) {
    WriteResponse(501, "Syntax error.");
    return;
  }

  if (extendedHello) {
    PINDEX equalPos = sender.Find('=', extendedArgPos);
    PCaselessString body = sender(extendedArgPos, equalPos).Trim();
    PCaselessString mime = sender.Mid(equalPos+1).Trim();
    eightBitMIME = (body == "BODY" && mime == "8BITMIME");
  }

  PString response = "Sender " + fromName;
  if (eightBitMIME)
    response += " and 8BITMIME";
  WriteResponse(250, response + " Ok");
}


void PSMTPSocket::OnDATA()
{
  if (fromName.IsEmpty()) {
    WriteResponse(503, "Need a valid MAIL command.");
    return;
  }

  if (toNames.GetSize() == 0) {
    WriteResponse(503, "Need a valid RCPT command.");
    return;
  }

  // Ok, everything is ready to start the message.
  BOOL ok = TRUE;
  PCharArray buffer;
  if (eightBitMIME) {
    WriteResponse(354,
                "Enter 8BITMIME message, terminate with '<CR><LF>.<CR><LF>'.");
    endMIMEDetectState = StuffIdle;
    while (ok && OnMIMEData(buffer))
      ok = HandleMessage(buffer, FALSE);
  }
  else {
    WriteResponse(354, "Enter mail, terminate with '.' alone on a line.");
    while (ok && OnTextData(buffer))
      ok = HandleMessage(buffer, FALSE);
  }

  if (ok && HandleMessage(buffer, TRUE))
    WriteResponse(250, "Message received Ok.");
  else
    WriteResponse(554, "Message storage failed.");
}


BOOL PSMTPSocket::OnUnknown(const PCaselessString & command)
{
  WriteResponse(500, "Command \"" + command + "\" unrecognised.");
  return TRUE;
}


BOOL PSMTPSocket::OnTextData(PCharArray & buffer)
{
  PString line;
  while (ReadLine(line, TRUE)) {
    line += '\n';
    PINDEX size = buffer.GetSize();
    PINDEX len = line.GetLength();
    memcpy(buffer.GetPointer(size + len) + size, (const char *)line, len);
    if (size + len > messageBufferSize)
      return TRUE;
  }
  return FALSE;
}


BOOL PSMTPSocket::OnMIMEData(PCharArray & buffer)
{
  int count = 0;
  int c;
  while ((c = ReadChar()) >= 0) {
    if (count >= buffer.GetSize())
      buffer.SetSize(count + 100);
    switch (endMIMEDetectState) {
      case StuffIdle :
        buffer[count++] = (char)c;
        break;

      case StuffCR :
        endMIMEDetectState = c != '\n' ? StuffIdle : StuffCRLF;
        buffer[count++] = (char)c;
        break;

      case StuffCRLF :
        if (c == '.')
          endMIMEDetectState = StuffCRLFdot;
        else {
          endMIMEDetectState = StuffIdle;
          buffer[count++] = (char)c;
        }
        break;

      case StuffCRLFdot :
        switch (c) {
          case '\r' :
            endMIMEDetectState = StuffCRLFdotCR;
            break;

          case '.' :
            endMIMEDetectState = StuffIdle;
            buffer[count++] = (char)c;
            break;

          default :
            endMIMEDetectState = StuffIdle;
            buffer[count++] = '.';
            buffer[count++] = (char)c;
        }
        break;

      case StuffCRLFdotCR :
        if (c == '\n')
          return FALSE;
        buffer[count++] = '.';
        buffer[count++] = '\r';
        buffer[count++] = (char)c;
        endMIMEDetectState = StuffIdle;

      default :
        PAssertAlways("Illegal SMTP state");
    }
    if (count > messageBufferSize) {
      buffer.SetSize(messageBufferSize);
      return TRUE;
    }
  }

  return FALSE;
}


PSMTPSocket::LookUpResult PSMTPSocket::LookUpName(
                               const PCaselessString &, PString & expandedName)
{
  expandedName = PString();
  return LookUpError;
}


BOOL PSMTPSocket::HandleMessage(PCharArray &, BOOL)
{
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PPOP3Socket

static char const * POP3Commands[PPOP3Socket::NumCommands] = {
  "USER", "PASS", "QUIT", "RSET", "NOOP", "STAT",
  "LIST", "RETR", "DELE", "APOP", "TOP",  "UIDL"
};


PString PPOP3Socket::okResponse = "+OK";
PString PPOP3Socket::errResponse = "-ERR";


PPOP3Socket::PPOP3Socket()
  : PApplicationSocket(NumCommands, POP3Commands, "pop3 110")
{
  Construct();
}


PPOP3Socket::PPOP3Socket(const PString & address)
  : PApplicationSocket(NumCommands, POP3Commands, "pop3 110")
{
  Construct();
  Connect(address);
}


PPOP3Socket::PPOP3Socket(PSocket & socket)
  : PApplicationSocket(NumCommands, POP3Commands)
{
  Construct();
  Accept(socket);
}


PPOP3Socket::~PPOP3Socket()
{
  Close();
}

void PPOP3Socket::Construct()
{
  loggedIn = FALSE;
}


BOOL PPOP3Socket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (ReadResponse() && lastResponseCode[0] == '+')
    return TRUE;

  Close();
  return FALSE;
}


BOOL PPOP3Socket::Accept(PSocket & socket)
{
  if (!PApplicationSocket::Accept(socket))
    return FALSE;

  return WriteResponse(okResponse, GetLocalHostName() +
                     " POP3 server ready at " +
                      PTime(PTime::MediumDateTime).AsString());
}


BOOL PPOP3Socket::Close()
{
  BOOL ok = TRUE;
  if (IsOpen() && loggedIn) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "") == '+';
  }
  return PApplicationSocket::Close() && ok;
}


BOOL PPOP3Socket::LogIn(const PString & username, const PString & password)
{
  if (ExecuteCommand(USER, username) != '+')
    return FALSE;

  if (ExecuteCommand(PASS, password) != '+')
    return FALSE;

  loggedIn = TRUE;
  return TRUE;
}


int PPOP3Socket::GetMessageCount()
{
  if (ExecuteCommand(STAT, "") != '+')
    return -1;

  return (int)lastResponseInfo.AsInteger();
}


PUnsignedArray PPOP3Socket::GetMessageSizes()
{
  PUnsignedArray sizes;

  if (ExecuteCommand(LIST, "") == '+') {
    PString msgInfo;
    while (ReadLine(msgInfo, TRUE))
      sizes.SetAt((PINDEX)msgInfo.AsInteger()-1,
                  (unsigned)msgInfo.Mid(msgInfo.Find(' ')).AsInteger());
  }

  return sizes;
}


PStringArray PPOP3Socket::GetMessageHeaders()
{
  PStringArray headers;

  int count = GetMessageCount();
  for (int msgNum = 1; msgNum <= count; msgNum++) {
    if (ExecuteCommand(TOP, PString(PString::Unsigned, msgNum) + " 0") == '+'){
      PString headerLine;
      while (ReadLine(headerLine, TRUE))
        headers[msgNum-1] += headerLine;
    }
  }
  return headers;
}


BOOL PPOP3Socket::BeginMessage(PINDEX messageNumber)
{
  return ExecuteCommand(RETR, PString(PString::Unsigned,messageNumber)) == '+';
}


BOOL PPOP3Socket::DeleteMessage(PINDEX messageNumber)
{
  return ExecuteCommand(DELE, PString(PString::Unsigned,messageNumber)) == '+';
}


BOOL PPOP3Socket::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return FALSE;

  switch (num) {
    case USER :
      OnUSER(args);
      break;
    case PASS :
      OnPASS(args);
      break;
    case QUIT :
      OnQUIT();
      return FALSE;
    case RSET :
      OnRSET();
      break;
    case NOOP :
      OnNOOP();
      break;
    case STAT :
      OnSTAT();
      break;
    case LIST :
      OnLIST((PINDEX)args.AsInteger());
      break;
    case RETR :
      OnRETR((PINDEX)args.AsInteger());
      break;
    case DELE :
      OnDELE((PINDEX)args.AsInteger());
      break;
    case TOP :
      if (args.Find(' ') == P_MAX_INDEX)
        WriteResponse(errResponse, "Syntax error");
      else
        OnTOP((PINDEX)args.AsInteger(),
              (PINDEX)args.Mid(args.Find(' ')).AsInteger());
      break;
    case UIDL :
      OnUIDL((PINDEX)args.AsInteger());
      break;
    default :
      return OnUnknown(args);
  }

  return TRUE;
}


void PPOP3Socket::OnUSER(const PString & name)
{
  messageSizes.SetSize(0);
  messageIDs.SetSize(0);
  username = name;
  WriteResponse(okResponse, "User name accepted.");
}


void PPOP3Socket::OnPASS(const PString & password)
{
  if (username.IsEmpty())
    WriteResponse(errResponse, "No user name specified.");
  else if (HandleOpenMailbox(username, password))
    WriteResponse(okResponse, username + " mail is available.");
  else
    WriteResponse(errResponse, "No access to " + username + " mail.");
  messageDeletions.SetSize(messageIDs.GetSize());
}


void PPOP3Socket::OnQUIT()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    if (messageDeletions[i])
      HandleDeleteMessage(i+1, messageIDs[i]);

  WriteResponse(okResponse, GetLocalHostName() +
                     " POP3 server signing off at " +
                      PTime(PTime::MediumDateTime).AsString());

  Close();
}


void PPOP3Socket::OnRSET()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    messageDeletions[i] = FALSE;
  WriteResponse(okResponse, "Resetting state.");
}


void PPOP3Socket::OnNOOP()
{
  WriteResponse(okResponse, "Doing nothing.");
}


void PPOP3Socket::OnSTAT()
{
  DWORD total = 0;
  for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
    total += messageSizes[i];
  WriteResponse(okResponse, psprintf("%u %u", messageSizes.GetSize(), total));
}


void PPOP3Socket::OnLIST(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse,
            PString(PString::Unsigned, messageSizes.GetSize()) + " messages.");
    for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
      WriteLine(psprintf("%u %u", i, messageSizes[i-1]));
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    WriteResponse(okResponse, psprintf("%u %u", msg, messageSizes[msg-1]));
}


void PPOP3Socket::OnRETR(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else {
    WriteResponse(okResponse,
                 PString(PString::Unsigned, messageSizes[msg-1]) + " octets.");
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], P_MAX_INDEX);
    stuffingState = DontStuff;
    WriteString(CRLFdotCRLF);
  }
}


void PPOP3Socket::OnDELE(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    messageDeletions[msg-1] = TRUE;
}


void PPOP3Socket::OnTOP(PINDEX msg, PINDEX count)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else {
    WriteResponse(okResponse, PString());
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], count);
    stuffingState = DontStuff;
    WriteString(CRLFdotCRLF);
  }
}


void PPOP3Socket::OnUIDL(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse,
              PString(PString::Unsigned, messageIDs.GetSize()) + " messages.");
    for (PINDEX i = 1; i <= messageIDs.GetSize(); i++)
      WriteLine(PString(PString::Unsigned, i) & messageIDs[i-1]);
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    WriteLine(PString(PString::Unsigned, msg) & messageIDs[msg-1]);
}


BOOL PPOP3Socket::OnUnknown(const PCaselessString & command)
{
  WriteResponse(errResponse, "Command \"" + command + "\" unrecognised.");
  return TRUE;
}


BOOL PPOP3Socket::HandleOpenMailbox(const PString &, const PString &)
{
  return FALSE;
}


void PPOP3Socket::HandleSendMessage(PINDEX, const PString &, PINDEX)
{
}


void PPOP3Socket::HandleDeleteMessage(PINDEX, const PString &)
{
}


// End Of File ///////////////////////////////////////////////////////////////
