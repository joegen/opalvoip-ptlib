/*
 * pchannel.cxx
 *
 * Operating System utilities.
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
 * $Log: pchannel.cxx,v $
 * Revision 1.2  1999/01/31 00:57:18  robertj
 * Fixed bug when opening an already open file, should close it!
 *
 * Revision 1.1  1998/11/30 12:46:19  robertj
 * Initial revision
 *
 */

#include <ptlib.h>

#include <ctype.h>


///////////////////////////////////////////////////////////////////////////////
// PChannel

PChannelStreamBuffer::PChannelStreamBuffer(PChannel * chan)
  : channel(PAssertNULL(chan))
{
}


int PChannelStreamBuffer::overflow(int c)
{
  if (pbase() == NULL) {
    if (eback() == 0)
      setp(buffer, &buffer[sizeof(buffer)]);
    else {
      char * halfway = &buffer[sizeof(buffer)/2];
      setp(buffer, halfway);
      setg(halfway, &buffer[sizeof(buffer)], &buffer[sizeof(buffer)]);
    }
  }

  int bufSize = pptr() - pbase();
  if (bufSize > 0) {
    setp(pbase(), epptr());
    if (!channel->Write(pbase(), bufSize))
      return EOF;
  }

  if (c != EOF) {
    char ch = (char)c;
    if (!channel->Write(&ch, 1))
      return EOF;
  }

  return 0;
}


int PChannelStreamBuffer::underflow()
{
  if (eback() == NULL) {
    if (pbase() == 0)
      setg(buffer, &buffer[sizeof(buffer)], &buffer[sizeof(buffer)]);
    else {
      char * halfway = &buffer[sizeof(buffer)/2];
      setp(buffer, halfway);
      setg(halfway, &buffer[sizeof(buffer)], &buffer[sizeof(buffer)]);
    }
  }

  if (gptr() != egptr())
    return (BYTE)*gptr();

  if (!channel->Read(eback(), egptr() - eback()) ||
                                  channel->GetErrorCode() != PChannel::NoError)
    return EOF;

  PINDEX count = channel->GetLastReadCount();
  char * p = egptr() - count;
  memmove(p, eback(), count);
  setg(eback(), p, egptr());
  return (BYTE)*p;
}


int PChannelStreamBuffer::sync()
{
  int inAvail = egptr() - gptr();
  if (inAvail > 0) {
    setg(eback(), egptr(), egptr());
    if (channel->IsDescendant(PFile::Class()))
      ((PFile *)channel)->SetPosition(-inAvail, PFile::Current);
  }

  if (pptr() > pbase())
    return overflow();

  return 0;
}


streampos PChannelStreamBuffer::seekoff(streamoff off,
#ifdef __MWERKS__
                                        ios::seekdir dir, ios::openmode)
#else
                                        ios::seek_dir dir, int)
#endif
{
  sync();
  if (!channel->IsDescendant(PFile::Class()))
    return -1;
  ((PFile *)channel)->SetPosition(off, (PFile::FilePositionOrigin)dir);
  return ((PFile *)channel)->GetPosition();
}


PChannel::PChannel()
  : readTimeout(PMaxTimeInterval), writeTimeout(PMaxTimeInterval)
{
  os_handle = -1;
  osError = 0;
  lastError = NoError;
  lastReadCount = lastWriteCount = 0;
  init(new PChannelStreamBuffer(this));
  Construct();
}


PChannel::~PChannel()
{
  flush();
  Close();
  delete (PChannelStreamBuffer *)rdbuf();
  init(NULL);
}


BOOL PChannel::IsOpen() const
{
  return os_handle >= 0;
}


int PChannel::ReadChar()
{
  BYTE c;
  BOOL retVal = Read(&c, 1);
  return (retVal && lastReadCount == 1) ? c : -1;
}


int PChannel::ReadCharWithTimeout(PTimeInterval & timeout)
{
  SetReadTimeout(timeout);
  PTimeInterval startTick = PTimer::Tick();
  int c;
  if ((c = ReadChar()) < 0) // Timeout or aborted
    return -1;
  timeout -= PTimer::Tick() - startTick;
  return c;
}


BOOL PChannel::ReadBlock(void * buf, PINDEX len)
{
  char * ptr = (char *)buf;
  PINDEX numRead = 0;

  while (numRead < len && Read(ptr+numRead, len - numRead))
    numRead += lastReadCount;

  lastReadCount = numRead;

  return lastReadCount == len;
}


PString PChannel::ReadString(PINDEX len)
{
  PString str;
  ReadBlock(str.GetPointer(len+1), len);
  str.SetSize(lastReadCount+1);
  return str;
}


BOOL PChannel::WriteString(const PString & str)
{
  const char * ptr = str;
  PINDEX len = 0, slen = str.GetLength();

  while (len < slen && Write(ptr+len, slen - len))
    len += lastWriteCount;

  return len == slen;
}


BOOL PChannel::ReadAsync(void * buf, PINDEX len)
{
  BOOL retVal = Read(buf, len);
  OnReadComplete(buf, lastReadCount);
  return retVal;
}


void PChannel::OnReadComplete(void *, PINDEX)
{
}


BOOL PChannel::WriteChar(int c)
{
  PAssert(c >= 0 && c < 256, PInvalidParameter);
  char buf = (char)c;
  return Write(&buf, 1);
}


BOOL PChannel::WriteAsync(const void * buf, PINDEX len)
{
  BOOL retVal = Write(buf, len);
  OnWriteComplete(buf, lastWriteCount);
  return retVal;
}


void PChannel::OnWriteComplete(const void *, PINDEX)
{
}


enum {
  NextCharEndOfString = -1,
  NextCharDelay = -2,
  NextCharSend = -3,
  NextCharWait = -4
};


static int HexDigit(char c)
{
  if (!isxdigit(c))
    return 0;

  int hex = c - '0';
  if (hex < 10)
    return hex;

  hex -= 'A' - '9' - 1;
  if (hex < 16)
    return hex;

  return hex - ('a' - 'A');
}


static int GetNextChar(const PString & command,
                                    PINDEX & pos, PTimeInterval * time = NULL)
{
  int temp;

  if (command[pos] == '\0')
    return NextCharEndOfString;

  if (command[pos] != '\\')
    return command[pos++];

  switch (command[++pos]) {
    case '\0' :
      return NextCharEndOfString;

    case 'a' : // alert (ascii value 7)
      pos++;
      return 7;

    case 'b' : // backspace (ascii value 8)
      pos++;
      return 8;

    case 'f' : // formfeed (ascii value 12)
      pos++;
      return 12;

    case 'n' : // newline (ascii value 10)
      pos++;
      return 10;

    case 'r' : // return (ascii value 13)
      pos++;
      return 13;

    case 't' : // horizontal tab (ascii value 9)
      pos++;
      return 9;

    case 'v' : // vertical tab (ascii value 11)
      pos++;
      return 11;

    case 'x' : // followed by hh  where nn is hex number (ascii value 0xhh)
      if (isxdigit(command[++pos])) {
        temp = HexDigit(command[pos++]);
        if (isxdigit(command[pos]))
          temp += HexDigit(command[pos++]);
        return temp;
      }
      return command[pos];

    case 's' :
      pos++;
      return NextCharSend;

    case 'd' : // ns  delay for n seconds/milliseconds
    case 'w' :
      temp = command[pos] == 'd' ? NextCharDelay : NextCharWait;
      long milliseconds = 0;
      while (isdigit(command[++pos]))
        milliseconds = milliseconds*10 + command[pos] - '0';
      if (milliseconds <= 0)
        milliseconds = 1;
      if (command[pos] == 'm')
        pos++;
      else {
        milliseconds *= 1000;
        if (command[pos] == 's')
          pos++;
      }
      if (time != NULL)
        *time = milliseconds;
      return temp;
  }

  if (command[pos] < '0' || command[pos] > '7')
    return command[pos++];

  // octal number
  temp = command[pos++] - '0';
  if (command[pos] < '0' || command[pos] > '7')
    return temp;

  temp += command[pos++] - '0';
  if (command[pos] < '0' || command[pos] > '7')
    return temp;

  temp += command[pos++] - '0';
  return temp;
}


BOOL PChannel::ReceiveCommandString(int nextChar,
                            const PString & reply, PINDEX & pos, PINDEX start)
{
  if (nextChar != GetNextChar(reply, pos)) {
    pos = start;
    return FALSE;
  }

  PINDEX dummyPos = pos;
  return GetNextChar(reply, dummyPos) < 0;
}


BOOL PChannel::SendCommandString(const PString & command)
{
  abortCommandString = FALSE;

  int nextChar;
  PINDEX sendPosition = 0;
  PTimeInterval timeout;
  SetWriteTimeout(10000);

  while (!abortCommandString) { // not aborted
    nextChar = GetNextChar(command, sendPosition, &timeout);
    switch (nextChar) {
      default :
        if (!WriteChar(nextChar))
          return FALSE;
        break;

      case NextCharEndOfString :
        return TRUE;  // Success!!

      case NextCharSend :
        break;

      case NextCharDelay : // Delay in send
        PThread::Current()->Sleep(timeout);
        break;

      case NextCharWait : // Wait for reply
        PINDEX receivePosition = sendPosition;
        if (GetNextChar(command, receivePosition) < 0) {
          SetReadTimeout(timeout);
          while (ReadChar() >= 0)
            if (abortCommandString) // aborted
              return FALSE;
        }
        else {
          receivePosition = sendPosition;
          do {
            if (abortCommandString) // aborted
              return FALSE;
            if ((nextChar = ReadCharWithTimeout(timeout)) < 0)
              return FALSE;
          } while (!ReceiveCommandString(nextChar,
                                     command, receivePosition, sendPosition));
//          nextChar = GetNextChar(command, receivePosition);
          sendPosition = receivePosition;
        }
    }
  }

  return FALSE;
}


BOOL PChannel::Shutdown(ShutdownValue)
{
  return FALSE;
}


PChannel * PChannel::GetBaseReadChannel() const
{
  return (PChannel *)this;
}


PChannel * PChannel::GetBaseWriteChannel() const
{
  return (PChannel *)this;
}


///////////////////////////////////////////////////////////////////////////////
// PIndirectChannel

PIndirectChannel::PIndirectChannel()
{
  readChannel = writeChannel = NULL;
  writeAutoDelete = readAutoDelete = FALSE;
}


PObject::Comparison PIndirectChannel::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PIndirectChannel::Class()), PInvalidCast);
  const PIndirectChannel & other = (const PIndirectChannel &)obj;
  return readChannel == other.readChannel &&
         writeChannel == other.writeChannel ? EqualTo : GreaterThan;
}


PString PIndirectChannel::GetName() const
{
  if (readChannel != NULL && readChannel == writeChannel)
    return readChannel->GetName();

  PString name = "R<";
  if (readChannel != NULL)
    name += readChannel->GetName();
  name += "> T<";
  if (writeChannel != NULL)
    name += writeChannel->GetName();
  name += ">";
  return name;
}


BOOL PIndirectChannel::Close()
{
  BOOL retval = TRUE;

  flush();

  PChannel * r = readChannel;
  PChannel * w = writeChannel;

  readChannel = NULL;
  writeChannel = NULL;

  if (r != NULL) {
    retval = r->Close();
    if (readAutoDelete)
      delete r;
  }

  if (r != w && w != NULL) {
    retval = w->Close() && retval;
    if (writeAutoDelete)
      delete w;
  }

  return retval;
}


BOOL PIndirectChannel::IsOpen() const
{
  if (readChannel != NULL && readChannel == writeChannel)
    return readChannel->IsOpen();

  BOOL readOk = readChannel != NULL ? readChannel->IsOpen() : FALSE;
  BOOL writeOk = writeChannel != NULL ? writeChannel->IsOpen() : FALSE;
  return readOk && writeOk;
}


BOOL PIndirectChannel::Read(void * buf, PINDEX len)
{
  PAssert(readChannel != NULL, "Indirect read though NULL channel");
  flush();
  readChannel->SetReadTimeout(readTimeout);
  BOOL ret = readChannel->Read(buf, len);
  if (readChannel == NULL) {
    lastError     = Interrupted;
    osError       = EINTR;
    lastReadCount = 0;
  } else {
    lastError = readChannel->GetErrorCode();
    osError = readChannel->GetErrorNumber();
    lastReadCount = readChannel->GetLastReadCount();
  }
  return ret;
}


BOOL PIndirectChannel::Write(const void * buf, PINDEX len)
{
  PAssert(writeChannel != NULL, "Indirect write though NULL channel");
  flush();
  writeChannel->SetWriteTimeout(writeTimeout);
  BOOL ret = writeChannel->Write(buf, len);
  if (writeChannel == NULL) {
    lastError      = Interrupted;
    osError        = EINTR;
    lastWriteCount = 0;
  } else {
    lastError = writeChannel->GetErrorCode();
    osError = writeChannel->GetErrorNumber();
    lastWriteCount = writeChannel->GetLastWriteCount();
  }
  return ret;
}


BOOL PIndirectChannel::Shutdown(ShutdownValue value)
{
  if (readChannel != NULL && readChannel == writeChannel)
    return readChannel->Shutdown(value);

  BOOL readOk = readChannel != NULL ? readChannel->Shutdown(value) : FALSE;
  BOOL writeOk = writeChannel != NULL ? writeChannel->Shutdown(value) : FALSE;
  return readOk && writeOk;
}


BOOL PIndirectChannel::Open(PChannel & channel)
{
  return Open(&channel, FALSE);
}


BOOL PIndirectChannel::Open(PChannel * channel, BOOL autoDelete)
{
  return Open(channel, channel, autoDelete, autoDelete);
}


BOOL PIndirectChannel::Open(PChannel * readChan,
                            PChannel * writeChan,
                            BOOL autoDeleteRead,
                            BOOL autoDeleteWrite)
{
  if (readAutoDelete)
    delete readChannel;

  if (writeAutoDelete && readChannel != writeChannel)
    delete writeChannel;

  readChannel = readChan;
  readAutoDelete = autoDeleteRead;
  writeChannel = writeChan;
  writeAutoDelete = autoDeleteWrite;

  return IsOpen() && OnOpen();
}


BOOL PIndirectChannel::OnOpen()
{
  return TRUE;
}


BOOL PIndirectChannel::SetReadChannel(PChannel * channel, BOOL autoDelete)
{
  if (readAutoDelete)
    delete readChannel;

  readChannel = channel;
  readAutoDelete = autoDelete;
  return IsOpen();
}


BOOL PIndirectChannel::SetWriteChannel(PChannel * channel, BOOL autoDelete)
{
  if (writeAutoDelete)
    delete writeChannel;

  writeChannel = channel;
  writeAutoDelete = autoDelete;
  return IsOpen();
}


PChannel * PIndirectChannel::GetBaseReadChannel() const
{
  return readChannel != NULL ? readChannel->GetBaseReadChannel() : 0;
}


PChannel * PIndirectChannel::GetBaseWriteChannel() const
{
  return writeChannel != NULL ? writeChannel->GetBaseWriteChannel() : 0;
}


///////////////////////////////////////////////////////////////////////////////
// PFile

PFile::~PFile()
{
  Close();
}


PObject::Comparison PFile::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PFile::Class()), PInvalidCast);
  return path.Compare(((const PFile &)obj).path);
}


BOOL PFile::Rename(const PString & newname, BOOL force)
{
  Close();

  if (!ConvertOSError(Rename(path, newname, force) ? 0 : -1))
    return FALSE;

  path = path.GetDirectory() + newname;
  return TRUE;
}


BOOL PFile::Close()
{
  if (os_handle < 0) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  flush();

  BOOL ok = ConvertOSError(_close(os_handle));

  os_handle = -1;

  if (removeOnClose)
    Remove();

  return ok;
}


BOOL PFile::Read(void * buffer, PINDEX amount)
{
  flush();
  lastReadCount = _read(GetHandle(), buffer, amount);
  return ConvertOSError(lastReadCount) && lastReadCount > 0;
}


BOOL PFile::Write(const void * buffer, PINDEX amount)
{
  flush();
  lastWriteCount = _write(GetHandle(), buffer, amount);
  return ConvertOSError(lastWriteCount) && lastWriteCount >= amount;
}


BOOL PFile::Open(const PFilePath & name, OpenMode  mode, int opts)
{
  Close();
  SetFilePath(name);
  return Open(mode, opts);
}


off_t PFile::GetLength() const
{
  off_t pos = _lseek(GetHandle(), 0, SEEK_CUR);
  off_t len = _lseek(GetHandle(), 0, SEEK_END);
  PAssertOS(_lseek(GetHandle(), pos, SEEK_SET) == pos);
  return len;
}


BOOL PFile::IsEndOfFile() const
{
  ((PFile *)this)->flush();
  return GetPosition() >= GetLength();
}


BOOL PFile::SetPosition(off_t pos, FilePositionOrigin origin)
{
  return _lseek(GetHandle(), pos, origin) == pos;
}


BOOL PFile::Copy(const PFilePath & oldname, const PFilePath & newname, BOOL force)
{
  PFile oldfile(oldname, ReadOnly);
  if (!oldfile.IsOpen())
    return FALSE;

  PFile newfile(newname,
                   WriteOnly, Create|Truncate|(force ? MustExist : Exclusive));
  if (!newfile.IsOpen())
    return FALSE;

  PCharArray buffer(10000);

  off_t amount = oldfile.GetLength();
  while (amount > 10000) {
    if (!oldfile.Read(buffer.GetPointer(), 10000))
      return FALSE;
    if (!newfile.Write((const char *)buffer, 10000))
      return FALSE;
    amount -= 10000;
  }

  if (!oldfile.Read(buffer.GetPointer(), (int)amount))
    return FALSE;
  if (!oldfile.Write((const char *)buffer, (int)amount))
    return FALSE;

  return newfile.Close();
}


// End Of File ///////////////////////////////////////////////////////////////
