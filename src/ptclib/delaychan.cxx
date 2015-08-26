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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "delaychan.h"
#endif

#include <ptlib.h>
#include <ptclib/delaychan.h>

/////////////////////////////////////////////////////////

PAdaptiveDelay::PAdaptiveDelay(const PTimeInterval & maximumSlip, const PTimeInterval & minimumDelay)
  : m_jitterLimit(-maximumSlip)
  , m_minimumDelay(minimumDelay)
  , m_targetTime(0)
  , m_firstTime(true)
{
}

void PAdaptiveDelay::Restart()
{
  m_firstTime = true;
}

PBoolean PAdaptiveDelay::DelayInterval(const PTimeInterval & delta)
{
  if (m_firstTime) {
    m_firstTime = false;
    m_targetTime.SetCurrentTime();   // targetTime is the time we want to delay to
  }

  if (delta <= 0)
    return true;

  // Set the new target
  m_targetTime += delta;

  // Calculate the sleep time so we delay until the target time
  PTimeInterval delay = m_targetTime - PTime();

  // Catch up if we are too late and the featue is enabled
  if (m_jitterLimit < 0 && delay < m_jitterLimit) {
    unsigned i = 0;
    while (delay < 0) { 
      m_targetTime += delta;
      delay += delta;
      i++;
    }
    PTRACE (4, "AdaptiveDelay\tResynchronise skipped " << i << " frames");
  }

  // Else sleep only if necessary
  if (delay > m_minimumDelay)
    PThread::Sleep(delay);

  return delay <= -delta;
}


/////////////////////////////////////////////////////////

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
    PTRACE(1,"Delay\tPDelayChannel cannot open channel");
  }
  PTRACE(5,"Delay\tdelay = " << frameDelay << ", size = " << frameSize);
}

PBoolean PDelayChannel::Read(void * buf, PINDEX count)
{
  if (!PIndirectChannel::Read(buf, count))
    return false;

  if (mode != DelayWritesOnly)
    Wait(lastReadCount, nextReadTick);

  return true;
}


PBoolean PDelayChannel::Write(const void * buf, PINDEX count)
{
  if (!PIndirectChannel::Write(buf, count))
    return false;

  if (mode != DelayReadsOnly)
    Wait(lastWriteCount, nextWriteTick);

  return true;
}


void PDelayChannel::Wait(PINDEX count, PTimeInterval & nextTick)
{
  PTimeInterval thisTick = PTimer::Tick();

  if (nextTick == 0)
    nextTick = thisTick;

  PTimeInterval delay = nextTick - thisTick;
  if (delay > maximumSlip)
    PTRACE(6, "Delay\t" << delay);
  else {
    PTRACE(6, "Delay\t" << delay << " ignored, too large");
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
