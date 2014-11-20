/*
 * pprocess.h
 *
 * Operating System process (running program) class.
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

PDICTIONARY(PXFdDict, POrdinalKey, PThread);

///////////////////////////////////////////////////////////////////////////////
// PProcess

  public:
    friend class PApplication;
    friend class PServiceProcess;
    friend void PXSignalHandler(int);

    ~PProcess();

    friend void PXSigHandler(int);
    virtual void PXOnSignal(int);
    virtual void PXOnAsyncSignal(int);
    void         PXCheckSignals();

    static void PXShowSystemWarning(PINDEX code);
    static void PXShowSystemWarning(PINDEX code, const PString & str);

  protected:
    void         CommonConstruct();
    void         CommonDestruct();

    virtual void _PXShowSystemWarning(PINDEX code, const PString & str);
    uint32_t m_pxSignals;

#if P_HAS_BACKTRACE && PTRACING
  public:
    enum { WalkStackSignal = SIGTRAP };
    void InternalWalkStackSignaled();
#endif


// End Of File ////////////////////////////////////////////////////////////////
