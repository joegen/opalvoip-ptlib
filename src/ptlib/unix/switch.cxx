/*
 * switch.cxx
 *
 * Cooperative multi-threading stack switch function.
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
 * $Log: switch.cxx,v $
 * Revision 1.13  1998/11/05 09:04:16  craigs
 * Changed free to runtime_free
 *
 * Revision 1.12  1998/09/24 04:12:21  robertj
 * Added open software license.
 *
 */

#include <ptlib.h>

#ifndef P_PTHREADS

#ifdef P_LINUX
#ifdef JB_SP
#define	SET_STACK	context[0].__jmpbuf[JB_SP] = (int)stackTop-16;
#else
#define	SET_STACK	context[0].__sp = (__ptr_t)stackTop-16;
#endif
#define	SETJMP_PROLOG
#include <sys/mman.h>
#define	USE_MMAP	MAP_ANON | MAP_PRIVATE
#endif

#ifdef P_SUN4
#define	SETJMP_PROLOG	__asm__ ("ta 3"); 
#define SET_STACK	context[2] = ((int)stackTop-1024) & ~7;
#endif

#ifdef P_SOLARIS
#define	SETJMP_PROLOG	__asm__ ("ta 3"); 
#define SET_STACK	context[1] = ((int)stackTop-1024) & ~7;
#define	STACK_MULT	4
//#define	USE_MMAP	MAP_PRIVATE | MAP_NORESERVE
#include <sys/mman.h>
#endif

#ifdef P_HPUX
#define SET_STACK	context[1] = (int)(stackBase+64*2);
#define	SETJMP_PROLOG
#endif

#ifdef P_ULTRIX
#define SET_STACK	context[JB_SP] = (int)(stackTop-16);
#define	SETJMP_PROLOG
#endif

#ifndef SET_STACK
#warning No lightweight thread context switch mechanism defined
#endif

#ifndef STACK_MIN
#define	STACK_MIN	10240
#endif

#ifndef	STACK_MULT
#define	STACK_MULT	1
#endif

static PThread * localThis;

void PThread::SwitchContext(PThread * from)
{
  //
  //  no need to switch to ourselves
  //
  if (this == from)
    return;

  //
  //  save context for old thread
  //
  SETJMP_PROLOG
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  //
  //  if starting the current thread, create a context, give it a new stack
  //  and then switch to it.
  //  if we have just switched into a new thread, execute the BeginThread
  //  function
  //
  if (status == Starting) {
    localThis = this;
    SETJMP_PROLOG
    if (setjmp(context) != 0) { // Are being reactivated from previous yield
      localThis->BeginThread();
      PAssertAlways("Return from BeginThread not allowed");
    }
    SET_STACK
  }

  /////////////////////////////////////////////////
  //
  //  switch to the new thread
  //
  /////////////////////////////////////////////////
  longjmp(context, TRUE);
  PAssertAlways("Return from longjmp not allowed");
}


void PThread::AllocateStack(PINDEX stackProtoSize)
{
  int stackSize = STACK_MULT*PMAX(STACK_MIN, stackProtoSize);

#if defined(USE_MMAP)
  stackBase = (char *)mmap(0,
                           stackSize,
                           PROT_READ | PROT_WRITE,
			   USE_MMAP,
                           -1, 0);
  PAssert(stackBase != (char *)-1, "Cannot allocate virtual stack for thread");
#else
  stackBase = (char *)malloc(stackSize);
  PAssert(stackBase != NULL, "Cannot allocate stack for thread");
#endif
  stackTop  = stackBase + stackSize-1;
}

void PThread::FreeStack()
{
  if (stackBase != NULL)
#if defined(USE_MMAP)
    munmap(stackBase, stackTop-stackBase+1);
#else
    runtime_free(stackBase);
#endif
}

#endif  // P_PTHREADS


