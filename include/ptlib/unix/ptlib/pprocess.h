/*
 * $Id: pprocess.h,v 1.3 1996/04/15 10:50:48 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.3  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.2  1996/01/26 11:06:31  craigs
 * Added signal handlers
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PPROCESS

#pragma interface

PDICTIONARY(PXFdDict,    POrdinalKey, PThread);

///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "../../common/process.h"
  public:
    friend class PApplication;
    friend class PServiceProcess;

    PString GetHomeDir ();
    char ** GetEnvp() const;
    char ** GetArgv() const;
    int     GetArgc() const;

    friend void PXSigHandler(int);
    void PXSetupProcess();
    virtual void PXOnSigHup();
    virtual void PXOnSigInt();
    virtual void PXOnSigQuit();
    virtual void PXOnSigUsr1();
    virtual void PXOnSigUsr2();
    virtual void PXOnSigPipe();
    virtual void PXOnSigTerm();
    virtual void PXOnSigChld();

    void PXAbortIOBlock(int fd);

    static void PXShowSystemWarning(PINDEX code);
    static void PXShowSystemWarning(PINDEX code, const PString & str);

  protected:
    virtual void _PXShowSystemWarning(PINDEX code, const PString & str);
    PXFdDict     ioBlocks[3];

  private:
    char **envp;
    char **argv;
    int  argc;
};

extern PProcess * PProcessInstance;

#endif
