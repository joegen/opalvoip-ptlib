#include <ptlib.h>

static PThread * localThis;

void PThread::SwitchContext(PThread * from)
{
  /////////////////////////////////////////////////
  //
  //  no need to switch to ourselves
  //
  /////////////////////////////////////////////////
  if (this == from)
    return;

  /////////////////////////////////////////////////
  //
  //  save context for old thread
  //
  /////////////////////////////////////////////////
//#if defined (P_SUN4)
//  __asm__ ("ta 3");
//#endif
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  /////////////////////////////////////////////////
  //
  //  if starting the current thread, create a context, give it a new stack
  //  and then switch to it.
  //  if we have just switched into a new thread, change status to Running
  //  and execute the Main and Terminate functions
  //
  /////////////////////////////////////////////////
  if (status == Starting) {
    localThis = this;
    if (setjmp(context) != 0) 
      localThis->BeginThread();

  /////////////////////////////////////////////////
  //
  // Change the stack pointer in the jmp_buf to point
  // to the new stack
  //
  /////////////////////////////////////////////////


#if defined(P_LINUX)
    context[0].__sp = (__ptr_t)stackTop-16;
#elif defined (P_SUN4)
    context[2] = (int)stackTop-1024;
    context[2] &= ~7;
#elif defined (P_SOLARIS)
    context[1] = (int)stackTop-1024;
    context[1] &= ~7;
#elif defined (P_ULTRIX)
    context[JB_SP] = (int)(stackTop-16);
#else
#warning No lightweight thread context switch mechanism defined
#endif
  }

  /////////////////////////////////////////////////
  //
  //  switch to the new thread
  //
  /////////////////////////////////////////////////
  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}
