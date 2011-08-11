/*
 * notifier_ext.cxx
 *
 * Smart Notifiers and Notifier Lists
 *
 * Portable Windows Library
 *
 * Copyright (c) 2004 Reitek S.p.A.
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "notifier_ext.h"
#endif

#include <ptlib.h>
#include <ptlib/notifier_ext.h>
#include <ptlib/semaphor.h>

#include <set>
#include <map>


//////////////////////////////////////////////////////////////////////////////

static PNotifierIdentifer s_ValidationId = 0;
static std::set<PNotifierIdentifer> s_TargetValid;
static PMutex s_Mutex;


PValidatedNotifierTarget::PValidatedNotifierTarget()
{
  s_Mutex.Wait();
  do {
    m_validatedNotifierId = ++s_ValidationId;
  } while (!s_TargetValid.insert(m_validatedNotifierId).second);
  s_Mutex.Signal();
}


PValidatedNotifierTarget::~PValidatedNotifierTarget()
{
  s_Mutex.Wait();
  s_TargetValid.erase(m_validatedNotifierId);
  s_Mutex.Signal();
}


bool PValidatedNotifierTargetExists(PNotifierIdentifer id)
{
  PWaitAndSignal mutex(s_Mutex);
  return s_TargetValid.find(id) != s_TargetValid.end();
}


//////////////////////////////////////////////////////////////////////////////

struct PAsyncNotifierQueue : std::queue<PAsyncNotifierCallback *>
{
  PSemaphore             m_count;
  PAsyncNotifierTarget * m_target;

  PAsyncNotifierQueue(PAsyncNotifierTarget * target)
    : m_count(0, INT_MAX)
    , m_target(target)
  { }

  void Queue(PAsyncNotifierCallback * callback)
  {
    push(callback);
    m_count.Signal();
    m_target->AsyncNotifierSignal();
  }

  bool Execute(PAsyncNotifierTarget * target, const PTimeInterval & wait)
  {
    if (!PAssert(target == m_target, "PAsyncNotifier mismatch"))
      return false;

    if (!m_count.Wait(wait))
      return false;
    
    if (!PAssert(!empty(), "PAsyncNotifier queue empty"))
      return false;

    PAsyncNotifierCallback * callback = front();
    pop();
    if (!PAssert(callback != NULL, "PAsyncNotifier callback NULL"))
      return false;

    s_Mutex.Signal();
    callback->Call();
    s_Mutex.Wait();

    return true;
  }
};

typedef std::map<PNotifierIdentifer, PAsyncNotifierQueue> PAsyncNotifierQueueMap;

static PAsyncNotifierQueueMap s_TargetQueues;


PAsyncNotifierTarget::PAsyncNotifierTarget()
{
  s_Mutex.Wait();
  do {
    m_asyncNotifierId = ++s_ValidationId;
  } while (!s_TargetQueues.insert(PAsyncNotifierQueueMap::value_type(m_asyncNotifierId, this)).second);
  s_Mutex.Signal();
}


PAsyncNotifierTarget::~PAsyncNotifierTarget()
{
  s_Mutex.Wait();
  s_TargetQueues.erase(m_asyncNotifierId);
  s_Mutex.Signal();
}


void PAsyncNotifierCallback::Queue(PNotifierIdentifer id, PAsyncNotifierCallback * callback)
{
  s_Mutex.Wait();

  PAsyncNotifierQueueMap::iterator it = s_TargetQueues.find(id);
  if (it == s_TargetQueues.end())
    delete callback;
  else
    it->second.Queue(callback);

  s_Mutex.Signal();
}


bool PAsyncNotifierTarget::AsyncNotifierExecute(const PTimeInterval & wait)
{
  bool doneOne = false;

  s_Mutex.Wait();

  PAsyncNotifierQueueMap::iterator it = s_TargetQueues.find(m_asyncNotifierId);
  if (PAssert(it != s_TargetQueues.end(), "PAsyncNotifier missing"))
    doneOne = it->second.Execute(this, wait);

  s_Mutex.Signal();

  return doneOne;
}


void PAsyncNotifierTarget::AsyncNotifierSignal()
{
}


// End of File ///////////////////////////////////////////////////////////////
