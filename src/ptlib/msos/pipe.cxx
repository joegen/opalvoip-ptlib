/*
 * pipe.cxx
 *
 * Sub-process communicating with pipe I/O channel class
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
 */

#include <ptlib.h>
#include <ptlib/pipechan.h>

#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

PPipeChannel::PPipeChannel()
{
}


#ifdef _WIN32_WCE
PBoolean PPipeChannel::PlatformOpen(const PString &, const PStringArray &, OpenMode, PBoolean, PBoolean, const PStringToString *)
{
  return false;
}
#else
PBoolean PPipeChannel::PlatformOpen(const PString & subProgram,
                                const PStringArray & argumentList,
                                OpenMode mode,
                                PBoolean searchPath,
                                PBoolean stderrSeparate,
                                const PStringToString * environment)
{
  subProgName = subProgram;

  const char * prog = NULL;
  PStringStream cmdLine;
  if (searchPath)
    cmdLine << subProgram;
  else
    prog = subProgram;

  for (PINDEX i = 0; i < argumentList.GetSize(); i++) {
    cmdLine << ' ';
    if (argumentList[i].Find(' ') == P_MAX_INDEX)
      cmdLine << argumentList[i];
    else if (argumentList[i].Find('"') == P_MAX_INDEX)
      cmdLine << '"' << argumentList[i] << '"';
    else
      cmdLine << '\'' << argumentList[i] << '\'';
  }

  PCharArray envBuf;
  char * envStr = NULL;
  if (environment != NULL) {
    PINDEX size = 0;
    for (PStringToString::const_iterator it = environment->begin(); it != environment->end(); ++it) {
      PString str = it->first + '=' + it->second;
      PINDEX len = str.GetLength() + 1;
      envBuf.SetSize(size + len);
      memcpy(envBuf.GetPointer()+size, (const char *)str, len);
      size += len;
    }
    envStr = envBuf.GetPointer();
  }

  //
  // this code comes from http://msdn.microsoft.com/en-us/library/ms682499(VS.85).aspx
  //
  STARTUPINFO startup;
  memset(&startup, 0, sizeof(startup));
  startup.cb = sizeof(startup);
  startup.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
  startup.wShowWindow = SW_HIDE;
  startup.hStdInput = INVALID_HANDLE_VALUE;
  startup.hStdOutput = INVALID_HANDLE_VALUE;
  startup.hStdError = INVALID_HANDLE_VALUE;

  SECURITY_ATTRIBUTES security;
  security.nLength = sizeof(security);
  security.lpSecurityDescriptor = NULL;
  security.bInheritHandle = TRUE;

  // ReadOnly means child has no stdin
  // otherwise create a pipe for us to send data to child
  if (mode == ReadOnly)
    m_hToChild.Close();
  else {
    HANDLE writeEnd;
    PAssertOS(CreatePipe(&startup.hStdInput, &writeEnd, &security, 0));
    PAssertOS(SetHandleInformation(writeEnd, HANDLE_FLAG_INHERIT, 0));
    PAssertOS(m_hToChild.Duplicate(writeEnd, DUPLICATE_CLOSE_SOURCE|DUPLICATE_SAME_ACCESS));
  }

  // WriteOnly means child has no stdout
  // ReadWriteStd means child uses our stdout and stderr
  // otherwise, create a pipe to read stdout from child, and perhaps a seperate one for stderr too
  if (mode == WriteOnly)
    m_hFromChild.Close();
  else if (mode == ReadWriteStd) {
    m_hFromChild.Close();
    startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  }
  else {
    PAssertOS(CreatePipe(m_hFromChild.GetPointer(), &startup.hStdOutput, &security, 1));
    PAssertOS(SetHandleInformation(m_hFromChild, HANDLE_FLAG_INHERIT, 0));
    if (stderrSeparate) {
      PAssertOS(CreatePipe(m_hStandardError.GetPointer(), &startup.hStdError, &security, 1));
      PAssertOS(SetHandleInformation(m_hStandardError, HANDLE_FLAG_INHERIT, 0));
    }
    else {
      startup.hStdError = startup.hStdOutput;
      m_hStandardError.Close();
    }
  }

  if (ConvertOSError(CreateProcess(prog, (LPSTR)(const char *)cmdLine,
                                   NULL, NULL, true, 0, envStr,
                                   NULL, &startup, &info) ? 0 : -2))
    os_handle = info.dwProcessId;
  else {
    m_hToChild.Close();
    m_hFromChild.Close();
    m_hStandardError.Close();
  }

  if (startup.hStdInput != INVALID_HANDLE_VALUE)
    CloseHandle(startup.hStdInput);

  if (mode != ReadWriteStd) {
    if (startup.hStdOutput != INVALID_HANDLE_VALUE)
      CloseHandle(startup.hStdOutput);
    if (startup.hStdOutput != startup.hStdError)
      CloseHandle(startup.hStdError);
  }

  return IsOpen();
}
#endif // !_WIN32_WCE



PPipeChannel::~PPipeChannel()
{
  Close();
}


PBoolean PPipeChannel::IsOpen() const
{
  return os_handle != -1;
}


int PPipeChannel::GetReturnCode() const
{
  DWORD code;
  if (GetExitCodeProcess(info.hProcess, &code))
    return code != STILL_ACTIVE ? code : -2;

  ((PPipeChannel*)this)->ConvertOSError(-2);
  return -1;
}


PBoolean PPipeChannel::CanReadAndWrite()
{
  return true;
}

PBoolean PPipeChannel::IsRunning() const
{
  return GetReturnCode() == -2;
}


int PPipeChannel::WaitForTermination()
{
  if (WaitForSingleObject(info.hProcess, INFINITE) == WAIT_OBJECT_0)
    return GetReturnCode();

  ConvertOSError(-2);
  return -1;
}


int PPipeChannel::WaitForTermination(const PTimeInterval & timeout)
{
  if (WaitForSingleObject(info.hProcess, timeout.GetInterval()) == WAIT_OBJECT_0)
    return GetReturnCode();

  ConvertOSError(-2);
  return -1;
}


PBoolean PPipeChannel::Kill(int signal)
{
  return ConvertOSError(TerminateProcess(info.hProcess, signal) ? 0 : -2);
}


PBoolean PPipeChannel::Read(void * buffer, PINDEX len)
{
  SetLastReadCount(0);

  DWORD count = 0;

#ifndef _WIN32_WCE
  // Cannot use overlapped I/O with anonymous pipe.
  // So have all this hideous code. :-(

  if (readTimeout == PMaxTimeInterval) {
    if (!ConvertOSError(ReadFile(m_hFromChild, buffer, 1, &count, NULL) ? 0 : -2, LastReadError))
      return false;

    SetLastReadCount(1);
    if (len == 1)
      return true;

    if (!PeekNamedPipe(m_hFromChild, NULL, 0, NULL, &count, NULL))
      return ConvertOSError(-2, LastReadError);

    if (count == 0)
      return true;

    ++((BYTE * &)buffer);
    --len;
  }
  else {
    PSimpleTimer timeout(readTimeout);
    for (;;) {
      if (!PeekNamedPipe(m_hFromChild, NULL, 0, NULL, &count, NULL))
        return ConvertOSError(-2, LastReadError);

      if (count > 0)
        break;

      if (timeout.HasExpired()) {
        SetErrorValues(Timeout, ETIMEDOUT, LastReadError);
        return false;
      }

      Sleep(10);
    }
  }

  if (len > (PINDEX)count)
    len = count;
#endif

  if (!ConvertOSError(ReadFile(m_hFromChild, buffer, len, &count, NULL) ? 0 : -2, LastReadError))
    return false;

  return SetLastReadCount(GetLastReadCount() + count) > 0;
}
      

PBoolean PPipeChannel::Write(const void * buffer, PINDEX len)
{
  SetLastWriteCount(0);
  DWORD count;
  if (!ConvertOSError(WriteFile(m_hToChild, buffer, len, &count, NULL) ? 0 : -2, LastWriteError))
    return false;
  return SetLastWriteCount(count) >= len;
}


PBoolean PPipeChannel::Close()
{
  if (IsOpen()) {
    os_handle = -1;
    m_hToChild.Close();
    m_hFromChild.Close();
    m_hStandardError.Close();
    if (!TerminateProcess(info.hProcess, 1))
      return false;
  }
  return true;
}


PBoolean PPipeChannel::Execute()
{
  flush();
  clear();
  m_hToChild.Close();
  return IsRunning();
}


#ifdef _WIN32_WCE
PBoolean PPipeChannel::ReadStandardError(PString &, PBoolean)
{
  return false;
}
#else
PBoolean PPipeChannel::ReadStandardError(PString & errors, PBoolean wait)
{
  DWORD available, bytesRead;
  if (!PeekNamedPipe(m_hStandardError, NULL, 0, NULL, &available, NULL))
    return ConvertOSError(-2, LastReadError);

  if (available != 0)
    return ConvertOSError(ReadFile(m_hStandardError,
                          errors.GetPointerAndSetLength(available), available,
                          &bytesRead, NULL) ? 0 : -2, LastReadError);

  if (!wait)
    return false;

  char firstByte;
  if (!ReadFile(m_hStandardError, &firstByte, 1, &bytesRead, NULL))
    return ConvertOSError(-2, LastReadError);

  errors = firstByte;

  if (!PeekNamedPipe(m_hStandardError, NULL, 0, NULL, &available, NULL))
    return ConvertOSError(-2, LastReadError);

  if (available == 0)
    return true;

  return ConvertOSError(ReadFile(m_hStandardError,
                        errors.GetPointerAndSetLength(available+1)+1, available,
                        &bytesRead, NULL) ? 0 : -2, LastReadError);
}
#endif // !_WIN32_WCE


int PPipeChannel::Run(const PString & command, PString & output, bool includeStderr, const PTimeInterval & timeout)
{
    PPipeChannel pipe;
    pipe.SetReadTimeout(timeout);

    if (!pipe.Open(command, PPipeChannel::ReadOnly, true, !includeStderr) | !pipe.Execute())
        return -1;

    // Read until end of file, or timeout
    output = pipe.ReadString(P_MAX_INDEX);

    // Wait for completion, which should have already happened
    return pipe.WaitForTermination(1000);
}


// End Of File ///////////////////////////////////////////////////////////////
