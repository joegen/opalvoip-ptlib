/*
 * thread.h
 *
 * Thread of execution control class.
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
 * $Revision$
 * $Author$
 * $Date$
 */


///////////////////////////////////////////////////////////////////////////////
// PThread

#define PTHREAD_ID_FMT ":%u"

  public:
    HANDLE GetHandle() const { return threadHandle; }
    void Win32AttachThreadInput();

    typedef DWORD LocalStorageKey;
    __inline static void   CreateLocalStorage(LocalStorageKey & key) { key = TlsAlloc(); }
    __inline static void   RemoveLocalStorage(const LocalStorageKey & key) { TlsFree(key);  }
    __inline static void * GetLocalStoragePtr(const LocalStorageKey & key) { return TlsGetValue(key); }
    __inline static void   SetLocalStoragePtr(const LocalStorageKey & key, void * ptr) { TlsSetValue(key, ptr); }

  protected:
    HANDLE threadHandle;

  private:
    void CleanUp();
    static UINT __stdcall MainFunction(void * thread);

  
// End Of File ///////////////////////////////////////////////////////////////
