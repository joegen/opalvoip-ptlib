/*
 * $Id: pprocess.h,v 1.7 1996/10/31 10:28:38 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.7  1996/10/31 10:28:38  craigs
 * Removed PXOnSigxxx routines
 *
 * Revision 1.6  1996/09/21 05:42:12  craigs
 * Changes for new common files, PConfig changes and signal handling
 *
 * Revision 1.5  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.4  1996/06/10 11:03:23  craigs
 * Changed include name
 *
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

#include "../../common/ptlib/pprocess.h"
  public:
    friend class PApplication;
    friend class PServiceProcess;
    friend void PXSignalHandler(int);

    PString GetHomeDir ();
    char ** GetEnvp() const;
    char ** GetArgv() const;
    int     GetArgc() const;
    void    PXSetupProcess();

    friend void PXSigHandler(int);
    virtual void PXOnSignal(int);
    virtual void PXOnAsyncSignal(int);

    void PXAbortIOBlock(int fd);

    static void PXShowSystemWarning(PINDEX code);
    static void PXShowSystemWarning(PINDEX code, const PString & str);

  protected:
    void         PXCheckSignals();
    virtual void _PXShowSystemWarning(PINDEX code, const PString & str);
    PXFdDict     ioBlocks[3];

    int pxSignals;

  private:
    char **envp;
    char **argv;
    int  argc;
};

extern PProcess * PProcessInstance;

#endif
