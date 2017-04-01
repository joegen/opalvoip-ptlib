/*
 * spooldir.cxx
 *
 * Spool Directory class
 *
 * Portable Windows Library
 *
 * Copyright (C) 2011 Post Increment
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
 */

#ifdef __GNUC__
#pragma implementation "spooldir.h"
#endif

#include <ptlib.h>
#include <ptclib/spooldir.h>


PSpoolDirectory::PSpoolDirectory()
  : m_thread(NULL)
  , m_threadRunning(false)
  , m_timeoutIfNoDir(10000)
  , m_scanTimeout(10000)
{
}

bool PSpoolDirectory::Open(const PDirectory & dir, const PString & type)
{
  PWaitAndSignal m(m_mutex);

  Close();

  m_threadRunning = true;

  PTRACE(3, "PSpoolDirectory\tThread started " << m_threadRunning);
  m_thread = new PThreadObj<PSpoolDirectory>(*this, &PSpoolDirectory::ThreadMain);

  m_directory = dir;
  m_fileType  = type;

  return true;
}

void PSpoolDirectory::Close()
{
  PTRACE(3, "PSpoolDirectory\tClosed");
  PWaitAndSignal m(m_mutex);

  if (m_thread != NULL) {
    m_threadRunning = false;
    m_thread->WaitForTermination();
    delete m_thread;
    m_thread = NULL;
  }
}


PString PSpoolDirectory::GetLockExtension() const
{
  return ".LCK";
}


void PSpoolDirectory::SetNotifier(const PNotifier & func) 
{ 
  PWaitAndSignal m(m_mutex);
  m_callback = func; 
}


void PSpoolDirectory::ThreadMain()
{
  PTRACE(3, "PSpoolDirectory\tThread started " << m_threadRunning);

  while (m_threadRunning) {

    {
      PWaitAndSignal m(m_mutex);
      m_scanner = m_directory;
    }

    // attempt to open the directory
    if (!m_scanner.Open()) {
      PTRACE(3, "PSpoolDirectory\tUnable to open directory '" << m_scanner << "' - sleeping for " << m_timeoutIfNoDir << " ms");
      PThread::Sleep(m_timeoutIfNoDir);
    }
    else {
      do {
        ProcessEntry();
      } while (m_scanner.Next());
      PTRACE(3, "PSpoolDirectory\tFinished scan - sleeping for " << m_scanTimeout << " ms");
      PThread::Sleep(m_scanTimeout);
    }
  }

  PTRACE(3, "PSpoolDirectory\tThread ended");
}


void PSpoolDirectory::ProcessEntry()
{
  // get the name of a file
  PString entry = m_scanner.GetEntryName();
  PFilePath fn  = m_scanner + entry;

  // get file information
  PFileInfo info;
  if (!m_scanner.GetInfo(info))
    return;

  // ignore locks
  if (((info.type & PFileInfo::SubDirectory) != 0) && (fn.GetType() == GetLockExtension()))
    return;

  // see if file type matches
  if (!m_fileType.IsEmpty() && (fn.GetType() != m_fileType))
    return;

  // see if lock file exists for this entry
  PFilePath lockDirName = fn + GetLockExtension();
  if (PFile::Exists(lockDirName) && PFile::GetInfo(lockDirName, info) && ((info.type & PFileInfo::SubDirectory) != 0))
    return;

  // process the entry
  if (!m_callback.IsNULL()) {
    m_callback(*this, (P_INT_PTR)&entry);
  }
  else if (!OnProcess(entry)) {
    PTRACE(3, "PSpoolDirectory\tEntry '" << entry << "' skipped processing");
  }
  else {
    PTRACE(3, "PSpoolDirectory\tEntry '" << entry << "' finished processing");
    if (!OnCleanup(entry)) {
      PTRACE(3, "PSpoolDirectory\tEntry '" << entry << "' cleaned up");
    }
    else if (PFile::Remove(fn, true)) {
      PTRACE(3, "PSpoolDirectory\tEntry '" << entry << "' removed");
    }
    else {
      PTRACE(1, "PSpoolDirectory\tEntry '" << entry << "' could not be removed");
    }
  }
}


bool PSpoolDirectory::OnProcess(const PString & entry)
{
  PTRACE(3, "PSpoolDirectory\tProcessing file '" << entry << "'");
  return true;
}


bool PSpoolDirectory::OnCleanup(const PString & entry)
{
  PTRACE(3, "PSpoolDirectory\tCleaning up file '" << entry << "'");
  return true;
}


PString PSpoolDirectory::CreateUniqueName() const
{
  return PGloballyUniqueID().AsString();
}


bool PSpoolDirectory::CreateLockFile(const PString & filename)
{
  PFilePath lockFile = m_directory + (filename + GetLockExtension());
  return PDirectory::Create(lockFile);
}


bool PSpoolDirectory::DestroyLockFile(const PString & filename)
{
  PFilePath lockFile = m_directory + (filename + GetLockExtension());
  return PDirectory::Remove(lockFile);
}
