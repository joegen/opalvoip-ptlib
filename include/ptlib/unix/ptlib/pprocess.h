/*
 * $Id: pprocess.h,v 1.2 1996/01/26 11:06:31 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
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

  private:
    char **envp;
    char **argv;
    int  argc;
};

extern PProcess * PProcessInstance;

#endif
