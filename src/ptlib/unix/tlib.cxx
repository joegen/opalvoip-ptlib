/*
 * tlib.cxx
 *
 * Miscelaneous class implementation
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
 * $Log: tlib.cxx,v $
 * Revision 1.34  1998/10/31 14:14:21  robertj
 * Changed syncptack.h to syncthrd.h for more thread synchronisation objects.
 *
 * Revision 1.33  1998/10/19 00:29:57  robertj
 * Moved error stream to common.
 *
 * Revision 1.32  1998/09/24 04:12:22  robertj
 * Added open software license.
 *
 * Revision 1.31  1998/05/30 14:58:56  robertj
 * Fixed shutdown deadlock (and other failure modes) in cooperative threads.
 *
 * Revision 1.30  1998/04/17 15:13:08  craigs
 * Added lazy writes to Config cache
 *
 * Revision 1.29  1998/03/29 10:42:16  craigs
 * Changed for new initialisation scheme
 *
 * Revision 1.28  1998/03/26 05:01:12  robertj
 * Added PMutex and PSyncPoint classes.
 *
 * Revision 1.27  1998/01/04 08:09:23  craigs
 * Added support for PThreads through use of reentrant system calls
 *
 * Revision 1.26  1998/01/03 22:46:44  craigs
 * Added PThread support
 *
 * Revision 1.25  1997/05/10 08:04:15  craigs
 * Added new routines for access to PErrorStream
 *
 * Revision 1.24  1997/04/22 10:57:53  craigs
 * Removed DLL functions and added call the FreeStack
 *
 * Revision 1.23  1997/02/23 03:06:00  craigs
 * Changed for PProcess::Current reference
 *
 * Revision 1.22  1997/02/14 09:18:36  craigs
 * Changed for PProcess::Current being a reference rather that a ptr
 *
 * Revision 1.21  1996/12/30 03:21:46  robertj
 * Added timer to block on wait for child process.
 *
 * Revision 1.20  1996/12/29 13:25:02  robertj
 * Fixed GCC warnings.
 *
 * Revision 1.19  1996/11/16 11:11:46  craigs
 * Fixed problem with timeout on blocked IO channels
 *
 * Revision 1.18  1996/11/03 04:35:58  craigs
 * Added hack to avoid log timeouts, which shouldn't happen!
 *
 * Revision 1.17  1996/09/21 05:40:10  craigs
 * Changed signal hcnalding
 *
 * Revision 1.16  1996/09/03 11:55:19  craigs
 * Removed some potential problems with return values from system calls
 *
 * Revision 1.15  1996/06/29 01:43:11  craigs
 * Moved AllocateStack to switch.cxx to keep platform dependent routines in one place
 *
 * Revision 1.14  1996/06/10 12:46:53  craigs
 * Changed process.h include
 *
 * Revision 1.13  1996/05/25 06:06:33  craigs
 * Sun4 fixes and updated for gcc 2.7.2
 *
 * Revision 1.12  1996/05/09 10:55:59  craigs
 * More SunOS fixes
 *
 * Revision 1.11  1996/05/03 13:15:27  craigs
 * More Sun4 & Solaris fixes
 *
 * Revision 1.10  1996/05/03 13:11:35  craigs
 * More Sun4 fixes
 *
 * Revision 1.9  1996/05/02 12:11:54  craigs
 * Sun4 fixes
 *
 * Revision 1.8  1996/04/18 11:43:38  craigs
 * Changed GetHomeDir to use effective UID for uid, and changed to
 * look at passwd file info *before* $HOME variable
 *
 * Revision 1.7  1996/04/15 10:49:11  craigs
 * Last build prior to release of MibMaster v1.0
 *
 * Revision 1.6  1996/01/26 11:09:42  craigs
 * Added signal handlers
 *
 */

#define _OSUTIL_CXX

#pragma implementation "args.h"
#pragma implementation "pprocess.h"
#pragma implementation "thread.h"
#pragma implementation "semaphor.h"
#pragma implementation "mutex.h"
#pragma implementation "syncpoint.h"
#pragma implementation "syncthrd.h"

#include "ptlib.h"

#include <sys/time.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#if defined(P_LINUX)
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif

#if defined(P_LINUX) || defined(P_SUN4) || defined(P_SOLARIS)
#include <sys/utsname.h>
#define  HAS_UNAME
#endif

#include "uerror.h"

#if defined(P_HPUX9)
#define	SELECT(p1,p2,p3,p4,p5)		select(p1,(int *)(p2),(int *)(p3),(int *)(p4),p5)
#else
#define	SELECT(p1,p2,p3,p4,p5)		select(p1,p2,p3,p4,p5)
#endif

#if defined(P_SUN4)
extern "C" void bzero(void *, int);
extern "C" int select(int width,
			fd_set *readfds,
			fd_set *writefds,
			fd_set *exceptfds,
			struct timeval *timeout);
#endif

PString PProcess::GetOSClass()
{
  return PString("Unix");
}

PString PProcess::GetOSName()
{
#if defined(P_LINUX)
  return PString("Linux");
#elif defined(P_HPUX9)
  return PString("HP/UX");
#elif defined(P_SUN4)
  return PString("SunOS");
#elif defined(P_SOLARIS)
  return PString("Solaris");
#else
  return PString("Unknown");
#endif
}

PString PProcess::GetOSVersion()
{
#if defined(HAS_UNAME)
  struct utsname info;
  uname(&info);
  return PString(info.release);
#else
#warning No GetOSVersion specified
  return PString("unknown");
#endif
}

PDirectory PProcess::PXGetHomeDir ()

{
  PString dest;
  char *ptr;
  struct passwd *pw = NULL;

#ifdef P_PTHREADS
  struct passwd pwd;
  char buffer[1024];
  pw = ::getpwuid_r(geteuid(), &pwd, buffer, 1024);
#else
  pw = ::getpwuid(geteuid());
#endif

  if (pw != NULL && pw->pw_dir != NULL) 
    dest = pw->pw_dir;
  else if ((ptr = getenv ("HOME")) != NULL) 
    dest = ptr;
  else 
    dest = ".";

  if (dest.GetLength() > 0 && dest[dest.GetLength()-1] != '/')
    dest += "/";

  return dest;
}

///////////////////////////////////////////////////////////////////////////////
//
// PProcess
//
// Return the effective user name of the process, eg "root" etc.

PString PProcess::GetUserName() const

{
#ifdef P_PTHREADS
  struct passwd pwd;
  char buffer[1024];
  struct passwd * pw = ::getpwuid_r(getuid(), &pwd, buffer, 1024);
#else
  struct passwd * pw = ::getpwuid(getuid());
#endif

  char * ptr;
  if (pw != NULL && pw->pw_name != NULL)
    return PString(pw->pw_name);
  else if ((ptr = getenv("USER")) != NULL)
    return PString(ptr);
  else
    return PString("user");
}

void PProcess::PXShowSystemWarning(PINDEX num)
{
  PXShowSystemWarning(num, "");
}

void PProcess::PXShowSystemWarning(PINDEX num, const PString & str)
{
  PProcess::Current()._PXShowSystemWarning(num, str);
}

void PProcess::_PXShowSystemWarning(PINDEX code, const PString & str)
{
  PError << "PWLib/Unix error #"
         << code
         << "-"
         << str
         << endl;
}

void PXSignalHandler(int sig)
{
  PProcess & process = PProcess::Current();
  process.pxSignals |= 1 << sig;
  process.PXOnAsyncSignal(sig);
  signal(sig, PXSignalHandler);
}

void PProcess::PXCheckSignals()
{
  if (pxSignals == 0)
    return;

  for (int sig = 0; sig < 32; sig++) {
    int bit = 1 << sig;
    if ((pxSignals&bit) != 0) {
      pxSignals &= ~bit;
      PXOnSignal(sig);
    }
  }
}

#define HANDLER(h)  (h!=NULL?h:SIG_IGN)

void SetSignals(void (*handler)(int))
{
#ifdef SIGHUP
  signal(SIGHUP, HANDLER(handler));
#endif
#ifdef SIGINT
  signal(SIGINT, HANDLER(handler));
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, HANDLER(handler));
#endif
#ifdef SIGUSR1
  signal(SIGUSR1, HANDLER(handler));
#endif
#ifdef SIGUSR2
  signal(SIGUSR2, HANDLER(handler));
#endif
#ifdef SIGPIPE
  signal(SIGPIPE, HANDLER(handler));
#endif
#ifdef SIGTERM
  signal(SIGTERM, HANDLER(handler));
#endif
//#ifdef SIGCHLD		
//  signal(SIGCHLD, HANDLER(handler));
//#endif
}

#if 0
int PProcess::_main (int parmArgc, char *parmArgv[], char *parmEnvp[])
{
  // save the environment
  envp = parmEnvp;
  argc = parmArgc;
  argv = parmArgv;

  // perform process initialisation
  PXSetupProcess();

  // perform PWLib initialisation
  PreInitialise(argc, argv);

  // call the main program
  Main();

  SetSignals(NULL);

  return terminationValue;
}
#endif

void PProcess::PXOnAsyncSignal(int sig)
{
  switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGHUP:
    case SIGQUIT:
      raise(SIGKILL);
      break;
    default:
      return;
  }
}

void PProcess::PXOnSignal(int /*sig*/)
{
}

void PProcess::CommonConstruct()
{
  // Setup signal handlers
  pxSignals = 0;

  SetSignals(&PXSignalHandler);

  // initialise the timezone information
  tzset();

  CreateConfigFilesDictionary();
}

void PProcess::CommonDestruct()
{
  delete configFiles;
  SetSignals(NULL);
}

//////////////////////////////////////////////////////////////////
//
//  Non-PTHREAD based routines
//

#ifndef P_PTHREADS

#include "tlibcoop.cxx"

#else

#include "tlibthrd.cxx"

#endif
