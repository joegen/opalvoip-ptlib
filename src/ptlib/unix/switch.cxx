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
    free(stackBase);
#endif
}

#endif  // P_PTHREADS

