//
//  Routines for cooperative multi-tasking system
//

void PProcess::Construct()
{
  CommonConstruct();

  ioBlocks[0].AllowDeleteObjects(FALSE);
  ioBlocks[1].AllowDeleteObjects(FALSE);
  ioBlocks[2].AllowDeleteObjects(FALSE);
}


PProcess::~PProcess()
{
  CommonDestruct();
}

void PProcess::PXAbortIOBlock(int fd) 
{
  for (PINDEX i = 0; i < 3 ; i++) {
    POrdinalKey key(fd);
    PThread * thread = ioBlocks[i].GetAt(key);
    if (thread != NULL) {
      // ensure the thread is in the current list of active threads
      for (PThread * start = this; start != thread; start = start->link) 
        PAssert (start->link != this, "I/O abort for inactive thread");

      // check for terminated thread
      PAssert (thread->status != Terminated, "I/O abort for terminated thread");

      // zero out the thread entry
      ioBlocks[i].SetAt(key, NULL);
      thread->selectReturnVal = -1;
      thread->selectErrno     = EINTR;
      FD_ZERO(thread->read_fds);
      FD_ZERO(thread->write_fds);
      FD_ZERO(thread->exception_fds);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// PThread

PThread::PThread()
{
  hasIOTimer  = FALSE;
  handleWidth = 0;
  waitPid     = 0;
}

PThread::~PThread()
{
  if (this == (PThread *)&PProcess::Current())
    return;

  // can never destruct ourselves, unless we are the system process
  PAssert(this != PThread::Current(), "Thread attempted suicide!");

  // call the terminate function so overloads work properly
  if (!IsTerminated())
    Terminate();

  // now we can terminate
  FreeStack();
}

void PThread::ClearBlock()
{
  if (waitPid > 0)
    waitPid = 0;
  else
    handleWidth = 0;
}

static void Copy_Fd_Sets(fd_set & dr, fd_set & dw, fd_set & de,
                         fd_set & sr, fd_set & sw, fd_set & se, int width)
{
  unsigned long *srp = (unsigned long *)&sr;
  unsigned long *swp = (unsigned long *)&sw;
  unsigned long *sep = (unsigned long *)&se;

  unsigned long *drp = (unsigned long *)&dr;
  unsigned long *dwp = (unsigned long *)&dw;
  unsigned long *dep = (unsigned long *)&de;

  int orSize = (width + (8*sizeof (unsigned long)) - 1) / (8*sizeof (unsigned long));

  for (int i = 0; i < orSize; i++) {
    *drp++ = *srp++;
    *dwp++ = *swp++;
    *dep++ = *sep++;
  }
}

BOOL PThread::IsNoLongerBlocked()
{
  // check signals
  PProcess & process = PProcess::Current();
  process.PXCheckSignals();

  // if are blocked on a child process, see if that child is still going
  if (waitPid > 0) 
    return kill(waitPid, 0) != 0;

  // if the I/O block was terminated previously, perhaps by a close from
  // another thread, then return TRUE. Don't check for timeouts yet -
  // this means we return if data is available before we timeout because
  // it isn't there
  if (selectErrno != 0 || selectReturnVal != 0)
    return TRUE;

  // if we aren't blocked on a channel, assert
  PAssert (handleWidth > 0, "IsNoLongerBlocked called for thread not I/O blocked");

  // do a zero time select call to see if we would block if we did the read/write
  struct timeval timeout = {0, 0};

  fd_set rfds, wfds, efds;

  Copy_Fd_Sets(rfds, wfds, efds, *read_fds, *write_fds, *exception_fds, handleWidth);

  // check signals on the way in
  process.PXCheckSignals();

  // do the select
  selectReturnVal = SELECT (handleWidth,
                            read_fds, write_fds, exception_fds,
                            &timeout);

  // check signals on the way out
  process.PXCheckSignals();
  
  Copy_Fd_Sets(*read_fds, *write_fds, *exception_fds, rfds, wfds, efds, handleWidth);

  if (selectReturnVal != 0) {
    selectErrno  = errno;
    return TRUE;
  }

  // if the channel has timed out, return TRUE
  return hasIOTimer && (ioTimer == 0);
}


void PProcess::OperatingSystemYield()

{
  PThread * current = &PProcess::Current();

  // setup file descriptor tables for select call
  fd_set rfds, wfds, efds;
  FD_ZERO (&rfds); FD_ZERO (&wfds); FD_ZERO (&efds);
  int width     = 0;
  int waitCount = 0;

  // process the timer list BEFORE checking timers
  PTimeInterval delay = GetTimerList()->Process();

  // collect the handles across all threads
  PThread * thread = current;
  BOOL      threadUnblocked = FALSE;
  do {
    if (thread->status == BlockedIO) {
      if (thread->waitPid > 0)
        waitCount++;
      else if ((thread->selectReturnVal != 0) ||
               (thread->hasIOTimer && thread->ioTimer == 0)) {
        threadUnblocked = TRUE;
      } else if (thread->handleWidth > 0) {
        unsigned long *srp = (unsigned long *)thread->read_fds;
        unsigned long *swp = (unsigned long *)thread->write_fds;
        unsigned long *sep = (unsigned long *)thread->exception_fds;

        unsigned long *drp = (unsigned long *)&rfds;
        unsigned long *dwp = (unsigned long *)&wfds;
        unsigned long *dep = (unsigned long *)&efds;

        int orSize = (thread->handleWidth + (8*sizeof (unsigned long)) - 1) / (8*sizeof (unsigned long));

        for (int i = 0; i < orSize; i++) {
          *drp++ |= *srp++;
          *dwp++ |= *swp++;
          *dep++ |= *sep++;
        }

        width  =  PMAX(width, thread->handleWidth);
      }
    }
    thread = thread->link;
  } while (thread != current);

  //
  // if we found a thread that was IO blocked, but has already found
  // input, then return now
  //
  if (threadUnblocked)
    return;
  

  //
  // find timeout until next timer event
  //
  struct timeval * timeout = NULL;
  struct timeval   timeout_val;
  if (delay != PMaxTimeInterval) {
    if (delay.GetMilliSeconds() < 1000L*60L*60L*24L) {
      timeout_val.tv_usec = (delay.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = delay.GetSeconds();
      timeout             = &timeout_val;
    }
  }

  // wait for something to happen on an I/O channel, or for a timeout, or a SIGCLD
  int childPid;
  if ((width > 0) || (timeout != NULL)) {
    SELECT (width, &rfds, &wfds, &efds, timeout);
    childPid = wait3(NULL, WNOHANG|WUNTRACED, NULL);
  } else if (waitCount > 0)
    childPid = wait3(NULL, WUNTRACED, NULL);
  else {
    //PError << "OperatingSystemYield with no blocks! Sleeping....\n";
    ::sleep(1);
    childPid = 0;
  }

  // finish off any child processes that exited
  if (childPid >= 0) {
    int retVal;
    waitpid(childPid, &retVal, WNOHANG);
  }
 
  // process the timer list
  GetTimerList()->Process();
}

void PThread::PXSetOSHandleBlock(int fd, int type)
{
  POrdinalKey key(fd);
  PProcess & proc = PProcess::Current();
  if (type & 1) {
    PAssert(!proc.ioBlocks[0].Contains(key),
           "Attempt to read block on handle which already has a pending read operation");
    proc.ioBlocks[0].SetAt(key, this);
  }
  if (type & 2) {
    PAssert(!proc.ioBlocks[1].Contains(key),
           "Attempt to write block on handle which already has a pending write operation");
    proc.ioBlocks[1].SetAt(key, this);
  }
  if (type & 4) {
    PAssert(!proc.ioBlocks[2].Contains(key),
           "Attempt to exception block on handle which already has a pending except operation");
    proc.ioBlocks[2].SetAt(key, this);
  }
}

void PThread::PXClearOSHandleBlock(int fd, int type)
{
  POrdinalKey key(fd);
  PProcess & proc = PProcess::Current();
  if ((type & 1) && (proc.ioBlocks[0].GetAt(key) == this)) 
    proc.ioBlocks[0].SetAt(key, NULL);
  if ((type & 2) && (proc.ioBlocks[1].GetAt(key) == this)) 
    proc.ioBlocks[1].SetAt(key, NULL);
  if ((type & 4) && (proc.ioBlocks[2].GetAt(key) == this)) 
    proc.ioBlocks[2].SetAt(key, NULL);
}

////////////////////////////////////////////////////////////////////
//
// PThread::PXBlockOnIO
//   This function performs the specified IO block
//   The return value is the value that the select call returned
//   which caused the block to return. errno is also set to the 
//   value of errno at the time the select returned.
//

int PThread::PXBlockOnIO(int handle, int type, const PTimeInterval & timeout)
{
  // make sure this thread is running
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  // make sure we flush the buffer before doing a write
  fd_set tmp_rfd, tmp_wfd, tmp_efd;
  read_fds      = &tmp_rfd;
  write_fds     = &tmp_wfd;
  exception_fds = &tmp_efd;

  FD_ZERO (read_fds);
  FD_ZERO (write_fds);
  FD_ZERO (exception_fds);

  int blockType;
  switch (type) {
    case PChannel::PXReadBlock:
    case PChannel::PXAcceptBlock:
      FD_SET (handle, read_fds);
      blockType = 1;
      break;
    case PChannel::PXWriteBlock:
      FD_SET (handle, write_fds);
      blockType = 2;
      break;
    case PChannel::PXConnectBlock:
      FD_SET (handle, write_fds);
      FD_SET (handle, exception_fds);
      blockType = 2+4;
      break;
    default:
      PAssertAlways(PLogicError);
      return 0;
  }

  // make sure no other thread is blocked on this os_handle
  PXSetOSHandleBlock(handle, blockType);

  hasIOTimer  = timeout != PMaxTimeInterval;
  if (hasIOTimer)
    ioTimer     = timeout;
  selectErrno = selectReturnVal = 0;
  handleWidth = handle+1;
  waitPid     = 0;

  status      = BlockedIO;
  Yield();

  PXClearOSHandleBlock(handle, blockType);

  ioTimer.Stop();
  handleWidth = 0;

  errno = selectErrno;
  return selectReturnVal;
}

int PThread::PXBlockOnIO(int maxHandles,
                    fd_set & readBits,
                    fd_set & writeBits,
                    fd_set & exceptionBits,
       const PTimeInterval & timeout,
           const PIntArray & osHandles)
{
  PAssert(status == Running, "Attempt to I/O block a thread which is not running");

  for (PINDEX i = 0; i < osHandles.GetSize(); i+=2) 
    PXSetOSHandleBlock(osHandles[i], osHandles[i+1]);

  read_fds      = &readBits;
  write_fds     = &writeBits;
  exception_fds = &exceptionBits;

  handleWidth   = maxHandles;

  hasIOTimer    = timeout != PMaxTimeInterval;
  if (hasIOTimer)
    ioTimer       = timeout;
  selectErrno   = selectReturnVal = 0;
  waitPid     = 0;

  status        = BlockedIO;
  Yield();

  for (PINDEX i = 0; i < osHandles.GetSize(); i+=2)
    PXClearOSHandleBlock(osHandles[i], osHandles[i+1]);

  ioTimer.Stop();
  handleWidth = 0;

  errno = selectErrno;
  return selectReturnVal;
}

int PThread::PXBlockOnChildTerminate(int pid, const PTimeInterval & timeout)
{
  waitPid    = pid;
  hasIOTimer = timeout != PMaxTimeInterval;
  if (hasIOTimer)
    ioTimer  = timeout;
  status     = BlockedIO;
  Yield();
  return 0;
}

