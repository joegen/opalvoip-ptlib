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
 * Revision 1.19  2002/12/19 03:37:05  craigs
 * Simplified PChannel::WriteString
 *
 * Revision 1.18  2002/04/09 02:30:18  robertj
 * Removed GCC3 variable as __GNUC__ can be used instead, thanks jason Spence
 *
 * Revision 1.17  2002/01/26 23:57:45  craigs
 * Changed for GCC 3.0 compatibility, thanks to manty@manty.net
 *
 * Revision 1.16  2001/11/13 04:13:22  robertj
 * Added ability to adjust size of ios buffer on PChannels.
 *
 * Revision 1.15  2001/09/27 10:23:42  craigs
 * CHanged ReadString to allow read until end of input with P_MAX_INDEX arg
 *
 * Revision 1.14  2001/09/11 02:36:52  robertj
 * Fixed crash bug when ReadString() gets I/O error.
 *
 * Revision 1.13  2001/09/10 21:58:31  craigs
 * Fixed cut and paste problem that broke PIndirectChannel::Write
 *
 * Revision 1.12  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.11  2001/06/04 10:13:38  robertj
 * Added compare function to compare value of os_handle.
 * Added has function based on os_handle value.
 *
 * Revision 1.10  2001/01/02 06:07:07  robertj
 * Fixed race condition in reopening indirect channel, thanks Bertrand Croq.
 *
 * Revision 1.9  2000/11/14 08:25:58  robertj
 * Added function to propagate the error text through to indirect channel.
 *
 * Revision 1.8  2000/08/22 08:33:37  robertj
 * Removed PAssert() for write to unattached indirect channel, now sets
 *    return code so is similay to "unopened file" semantics.
 *
 * Revision 1.7  2000/06/26 11:17:20  robertj
 * Nucleus++ port (incomplete).
 *
 * Revision 1.6  1999/07/06 08:55:05  robertj
 * Fixed bug in PFile::Copy, does not write last chunk of data to new file.
 *
 * Revision 1.5  1999/06/17 14:44:42  robertj
 * Fixed incorrect comparison of open write channel
 *
 * Revision 1.4  1999/06/17 13:38:11  robertj
 * Fixed race condition on indirect channel close, mutex needed in PIndirectChannel.
 *
 * Revision 1.3  1999/02/22 10:10:12  robertj
 * Changed channel output flush to remove double Write() call.
 *
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


BOOL PChannelStreamBuffer::SetBufferSize(PINDEX newSize)
{
  return input.SetSize(newSize) && output.SetSize(newSize);
}


int PChannelStreamBuffer::overflow(int c)
{
  if (pbase() == NULL) {
    char * p = output.GetPointer(1024);
    setp(p, p+output.GetSize());
  }

  int bufSize = pptr() - pbase();
  if (bufSize > 0) {
    setp(pbase(), epptr());
    if (!channel->Write(pbase(), bufSize))
      return EOF;
  }

  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }

  return 0;
}


int PChannelStreamBuffer::underflow()
{
  if (eback() == NULL) {
    char * p = input.GetPointer(1024);
    char * e = p+input.GetSize();
    setg(p, e, e);
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
#if defined(__MWERKS__) || __GNUC__ >= 3
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
  : iostream(cout.rdbuf()),
    readTimeout(PMaxTimeInterval), writeTimeout(PMaxTimeInterval)
{
  os_handle = -1;
  memset(lastErrorCode, 0, sizeof(lastErrorCode));
  memset(lastErrorNumber, 0, sizeof(lastErrorNumber));
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


PObject::Comparison PChannel::Compare(const PObject & obj) const
{
  PAssert(IsDescendant(PChannel::Class()), PInvalidCast);
  int h1 = GetHandle();
  int h2 = ((const PChannel&)obj).GetHandle();
  if (h1 < h2)
    return LessThan;
  if (h1 > h2)
    return GreaterThan;
  return EqualTo;
}


PINDEX PChannel::HashFunction() const
{
  return GetHandle()%97;
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

  if (len == P_MAX_INDEX) {
    PINDEX l = 0;
    for (;;) {
      char * p = l + str.GetPointer(l+1000+1);
      if (!Read(p, 1000))
        break;
      l += lastReadCount;
    }
    str.SetSize(l+1);
  } else {
    if (!ReadBlock(str.GetPointer(len+1), len))
      return PString();
    str.SetSize(lastReadCount+1);
  }

  return str;
}


BOOL PChannel::WriteString(const PString & str)
{
  return Write((const char *)str, str.GetLength()); 
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


BOOL PChannel::SetBufferSize(PINDEX newSize)
{
  return ((PChannelStreamBuffer *)rdbuf())->SetBufferSize(newSize);
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


PString PChannel::GetErrorText(ErrorGroup group) const
{
  return GetErrorText(lastErrorCode[group], lastErrorNumber[group]);
}


BOOL PChannel::ConvertOSError(int status, ErrorGroup group)
{
  Errors lastError;
  int osError;
  BOOL ok = ConvertOSError(status, lastError, osError);
  SetErrorValues(lastError, osError, group);
  return ok;
}


BOOL PChannel::SetErrorValues(Errors errorCode, int errorNum, ErrorGroup group)
{
  lastErrorCode[NumErrorGroups] = lastErrorCode[group] = errorCode;
  lastErrorNumber[NumErrorGroups] = lastErrorNumber[group] = errorNum;
  return errorCode == NoError;
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
  ((PIndirectChannel*)this)->channelPointerMutex.StartRead();

  PString name;
  if (readChannel != NULL && readChannel == writeChannel)
    name = readChannel->GetName();
  else {
    name = "R<";
    if (readChannel != NULL)
      name += readChannel->GetName();
    name += "> T<";
    if (writeChannel != NULL)
      name += writeChannel->GetName();
    name += ">";
  }

  ((PIndirectChannel*)this)->channelPointerMutex.EndRead();

  return name;
}


BOOL PIndirectChannel::Close()
{
  BOOL retval = TRUE;

  flush();

  channelPointerMutex.StartRead();

  if (readChannel != NULL)
    retval = readChannel->Close();

  if (readChannel != writeChannel && writeChannel != NULL)
    retval = writeChannel->Close() && retval;

  channelPointerMutex.EndRead();

  channelPointerMutex.StartWrite();

  PChannel * r = readChannel;
  PChannel * w = writeChannel;

  readChannel = NULL;
  writeChannel = NULL;

  if (readAutoDelete)
    delete r;

  if (r != w && writeAutoDelete)
    delete w;

  channelPointerMutex.EndWrite();

  return retval;
}


BOOL PIndirectChannel::IsOpen() const
{
  ((PIndirectChannel*)this)->channelPointerMutex.StartRead();

  BOOL returnValue;
  if (readChannel != NULL && readChannel == writeChannel)
    returnValue = readChannel->IsOpen();
  else {
    returnValue = readChannel != NULL ? readChannel->IsOpen() : FALSE;
    if (writeChannel != NULL)
      returnValue = writeChannel->IsOpen() || returnValue;
  }

  ((PIndirectChannel*)this)->channelPointerMutex.EndRead();

  return returnValue;
}


BOOL PIndirectChannel::Read(void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  BOOL returnValue;
  if (readChannel == NULL) {
    SetErrorValues(NotOpen, EBADF, LastReadError);
    returnValue = FALSE;
  }
  else {
    readChannel->SetReadTimeout(readTimeout);

    returnValue = readChannel->Read(buf, len);

    SetErrorValues(readChannel->GetErrorCode(LastReadError),
                   readChannel->GetErrorNumber(LastReadError),
                   LastReadError);
    lastReadCount = readChannel->GetLastReadCount();
  }

  channelPointerMutex.EndRead();

  return returnValue;
}


BOOL PIndirectChannel::Write(const void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  BOOL returnValue;
  if (writeChannel == NULL) {
    SetErrorValues(NotOpen, EBADF, LastWriteError);
    returnValue = FALSE;
  }
  else {
    writeChannel->SetWriteTimeout(writeTimeout);

    returnValue = writeChannel->Write(buf, len);

    SetErrorValues(writeChannel->GetErrorCode(LastWriteError),
                   writeChannel->GetErrorNumber(LastWriteError),
                   LastWriteError);

    lastWriteCount = writeChannel->GetLastWriteCount();
  }

  channelPointerMutex.EndRead();

  return returnValue;
}


BOOL PIndirectChannel::Shutdown(ShutdownValue value)
{
  channelPointerMutex.StartRead();

  BOOL returnValue;
  if (readChannel != NULL && readChannel == writeChannel)
    returnValue = readChannel->Shutdown(value);
  else {
    returnValue = readChannel != NULL ? readChannel->Shutdown(value) : FALSE;
    if (writeChannel != NULL)
      returnValue = writeChannel->Shutdown(value) || returnValue;
  }

  channelPointerMutex.EndRead();

  return returnValue;
}


PString PIndirectChannel::GetErrorText(ErrorGroup group) const
{
  if (readChannel != NULL)
    return readChannel->GetErrorText(group);

  if (writeChannel != NULL)
    return writeChannel->GetErrorText(group);

  return PChannel::GetErrorText(group);
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
  flush();

  channelPointerMutex.StartWrite();

  if (readChannel != NULL)
    readChannel->Close();

  if (readChannel != writeChannel && writeChannel != NULL)
    writeChannel->Close();

  if (readAutoDelete)
    delete readChannel;

  if (readChannel != writeChannel && writeAutoDelete)
    delete writeChannel;

  readChannel = readChan;
  readAutoDelete = autoDeleteRead;

  writeChannel = writeChan;
  writeAutoDelete = autoDeleteWrite;

  channelPointerMutex.EndWrite();

  return IsOpen() && OnOpen();
}


BOOL PIndirectChannel::OnOpen()
{
  return TRUE;
}


BOOL PIndirectChannel::SetReadChannel(PChannel * channel, BOOL autoDelete)
{
  if (readChannel != NULL)
    return SetErrorValues(DeviceInUse, EEXIST);

  channelPointerMutex.StartWrite();

  readChannel = channel;
  readAutoDelete = autoDelete;

  channelPointerMutex.EndWrite();

  return IsOpen();
}


BOOL PIndirectChannel::SetWriteChannel(PChannel * channel, BOOL autoDelete)
{
  if (writeChannel != NULL)
    return SetErrorValues(DeviceInUse, EEXIST);

  channelPointerMutex.StartWrite();

  writeChannel = channel;
  writeAutoDelete = autoDelete;

  channelPointerMutex.EndWrite();

  return IsOpen();
}


PChannel * PIndirectChannel::GetBaseReadChannel() const
{
  ((PIndirectChannel*)this)->channelPointerMutex.StartRead();

  PChannel * returnValue = readChannel != NULL ? readChannel->GetBaseReadChannel() : 0;

  ((PIndirectChannel*)this)->channelPointerMutex.EndRead();

  return returnValue;
}


PChannel * PIndirectChannel::GetBaseWriteChannel() const
{
  ((PIndirectChannel*)this)->channelPointerMutex.StartRead();

  PChannel * returnValue = writeChannel != NULL ? writeChannel->GetBaseWriteChannel() : 0;

  ((PIndirectChannel*)this)->channelPointerMutex.EndRead();

  return returnValue;
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
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

  flush();

#ifdef WOT_NO_FILESYSTEM
  BOOL ok = TRUE;
#else
  BOOL ok = ConvertOSError(_close(os_handle));
#endif

  os_handle = -1;

  if (removeOnClose)
    Remove();

  return ok;
}


BOOL PFile::Read(void * buffer, PINDEX amount)
{
  flush();
#ifdef WOT_NO_FILESYSTEM
  lastReadCount = 0;
#else
  lastReadCount = _read(GetHandle(), buffer, amount);
#endif
  return ConvertOSError(lastReadCount, LastReadError) && lastReadCount > 0;
}


BOOL PFile::Write(const void * buffer, PINDEX amount)
{
  flush();
#ifdef WOT_NO_FILESYSTEM
  lastWriteCount = amount;
#else
  lastWriteCount = _write(GetHandle(), buffer, amount);
#endif
  return ConvertOSError(lastWriteCount, LastWriteError) && lastWriteCount >= amount;
}


BOOL PFile::Open(const PFilePath & name, OpenMode  mode, int opts)
{
  Close();
  SetFilePath(name);
  return Open(mode, opts);
}


off_t PFile::GetLength() const
{
#ifdef WOT_NO_FILESYSTEM
  return 0;
#else
  off_t pos = _lseek(GetHandle(), 0, SEEK_CUR);
  off_t len = _lseek(GetHandle(), 0, SEEK_END);
  PAssertOS(_lseek(GetHandle(), pos, SEEK_SET) == pos);
  return len;
#endif
}


BOOL PFile::IsEndOfFile() const
{
  ((PFile *)this)->flush();
  return GetPosition() >= GetLength();
}


BOOL PFile::SetPosition(off_t pos, FilePositionOrigin origin)
{
#ifdef WOT_NO_FILESYSTEM
  return TRUE;
#else
  return _lseek(GetHandle(), pos, origin) == pos;
#endif
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
  if (!newfile.Write((const char *)buffer, (int)amount))
    return FALSE;

  return newfile.Close();
}


// End Of File ///////////////////////////////////////////////////////////////
