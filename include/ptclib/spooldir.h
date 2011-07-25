/*
 * spooldir.h
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_SPOOLDIR_H
#define PTLIB_SPOOLDIR_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


////////////////////////////////////////////////////////

#include <ptlib.h>

#include <ptlib/pdirect.h>
#include <ptclib/guid.h>

class PSpoolDirectory : PObject
{
  public:
    PSpoolDirectory();

    bool Open(const PDirectory & dir, const PString & type = PString::Empty());
    void Close();
    void ThreadMain();

    PDirectory operator=(const PSpoolDirectory & dir) const
    { 
      return dir.GetDirectory(); 
    }

    PDirectory GetDirectory() const { return m_directory; }

    PString CreateLockName(const PString & filename) const;

    virtual void ProcessEntry();
    virtual bool OnProcess(const PString & entry);
    virtual bool OnCleanup(const PString & entry);

    virtual PString CreateUniqueName() const;
    virtual bool CreateLockFile(const PString & filename);
    virtual bool DestroyLockFile(const PString & filename);

    virtual PString GetLockExtension() const;

    virtual void SetNotifier(const PNotifier & func);

  protected:
    PMutex m_mutex;
    PThread * m_thread;

    PDirectory m_directory;

    PString m_fileType;

    bool m_threadRunning;
    PDirectory m_scanner;

    int m_timeoutIfNoDir;
    int m_scanTimeout;

    PNotifier m_callback;
};


#endif // PTLIB_SPOOLDIR_H

