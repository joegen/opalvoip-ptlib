#include <ptlib.h>

void PThread::SwitchContext(PThread * from)
{
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  if (status == Starting) {

    if (setjmp(context) != 0) {
      status = Running;
      Main();
      Terminate(); // Never returns from here
    }
#if defined(P_LINUX)
    context[0].__sp = (__ptr_t)stackTop-16;  // Change the stack pointer in jmp_buf
#elif defined (P_SUN4)
    context[2] = (int)(stackTop-1024);  // Change the stack pointer in jmp_buf
    context[2] &= 7;
#elif defined (P_SOLARIS)
    context[1] = (int)(stackTop-1024);  // Change the stack pointer in jmp_buf
    context[1] &= 7;
#else
#warning No lightweight thread context switch mechanism defined
#endif
  }
  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}
