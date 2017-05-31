/*
 * delaychan.cxx
 *
 * Class for controlling the timing of data passing through it.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 */

#ifdef __GNUC__
#pragma implementation "delaychan.h"
#endif

#include <ptlib.h>
#include <ptclib/delaychan.h>

/////////////////////////////////////////////////////////

#define PTraceModule() "AdaptDelay"

PAdaptiveDelay::PAdaptiveDelay(const PTimeInterval & maximumSlip, const PTimeInterval & minimumDelay)
  : m_maximumSlip(-maximumSlip)
  , m_minimumDelay(minimumDelay)
  , m_targetTime(0)
  , m_firstTime(true)
#if PTRACING
  , m_traceLevel(3)
#endif
{
}

void PAdaptiveDelay::Restart()
{
  m_firstTime = true;
}

PAdaptiveDelay::DelayResult PAdaptiveDelay::DelayInterval(const PTimeInterval & delta)
{
  if (m_firstTime) {
    m_firstTime = false;
    m_targetTime.SetCurrentTime();   // targetTime is the time we want to delay to
  }

  if (delta <= 0) {
    m_actualDelay = 0;
    return eBadDelta;
  }

  PTime now;

  // System time changed backward
  if (m_targetTime > (now + delta)) {
    m_targetTime.SetCurrentTime();
    PTRACE(m_traceLevel, "Resyncronised due to backward system time change.");
  }

  // Set the new target
  m_targetTime += delta;

  // Calculate the sleep time so we delay until the target time
  PTimeInterval delay = m_targetTime - now;

  // Catch up if we are too late and the featue is enabled
  if (m_maximumSlip < 0 && delay < m_maximumSlip) {
    PTRACE(m_traceLevel, "Resyncronised due to max slip reached, skipped " << (-delay/delta) << " delta intervals of " << delta);
    m_targetTime.SetCurrentTime();
    m_actualDelay = 0;
    return eSlipped;
  }

  // Else sleep only if necessary
  if (delay < m_minimumDelay)
    m_actualDelay = 0;
  else {
    PThread::Sleep(delay);
    m_actualDelay = now.GetElapsed();
    if (m_actualDelay > delay+delta*2) {
      PTRACE(m_traceLevel, "Over slept: expected=" << delay << " actual=" << m_actualDelay);
      return eOverSlept;
    }
  }

  return delay <= -delta ? eLate : eOnTime;
}


/////////////////////////////////////////////////////////

#undef  PTraceModule
#define PTraceModule() "DelayChan"

PDelayChannel::PDelayChannel(Mode m,
                             unsigned delay,
                             PINDEX size,
                             unsigned max,
                             unsigned min)
{
  mode = m;
  frameDelay = delay;
  frameSize = size;
  maximumSlip = -PTimeInterval(max);
  minimumDelay = min;
}

PDelayChannel::PDelayChannel(PChannel &channel,
                             Mode m,
                             unsigned delay,
                             PINDEX size,
                             unsigned max,
                             unsigned min) :
   mode(m), 
   frameDelay(delay),
   frameSize(size),
   minimumDelay(min)
{
  maximumSlip = -PTimeInterval(max);
  if(Open(channel) == false){
    PTRACE(1, "PDelayChannel cannot open channel");
  }
  PTRACE(5, "delay = " << frameDelay << ", size = " << frameSize);
}

PBoolean PDelayChannel::Read(void * buf, PINDEX count)
{
  if (!PIndirectChannel::Read(buf, count))
    return false;

  if (mode != DelayWritesOnly)
    Wait(GetLastReadCount(), nextReadTick);

  return true;
}


PBoolean PDelayChannel::Write(const void * buf, PINDEX count)
{
  if (!PIndirectChannel::Write(buf, count))
    return false;

  if (mode != DelayReadsOnly)
    Wait(GetLastWriteCount(), nextWriteTick);

  return true;
}


void PDelayChannel::Wait(PINDEX count, PTimeInterval & nextTick)
{
  PTimeInterval thisTick = PTimer::Tick();

  if (nextTick == 0)
    nextTick = thisTick;

  PTimeInterval delay = nextTick - thisTick;
  if (delay > maximumSlip)
    PTRACE(6, delay);
  else {
    PTRACE(6, delay << " ignored, too large");
    nextTick = thisTick;
    delay = 0;
  }

  if (frameSize > 0)
    nextTick += count*frameDelay/frameSize;
  else
    nextTick += frameDelay;

  if (delay > minimumDelay)
    PThread::Sleep(delay);
}


// End of File ///////////////////////////////////////////////////////////////
