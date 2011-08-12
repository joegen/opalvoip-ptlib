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
#include <queue>


//////////////////////////////////////////////////////////////////////////////

class PValidatedNotifierSet : std::set<PNotifierIdentifer>
{
  unsigned m_state; // 0 = pre-constructor, 1 = active, 2 = destroyed
  unsigned m_nextId;
  PMutex   m_mutex;

public:
  PValidatedNotifierSet()
    : m_state(1)
  {
  }

  ~PValidatedNotifierSet()
  {
    m_state = 2;
  }

  PNotifierIdentifer Add()
  {
    if (m_state != 1)
      return 0;

    PNotifierIdentifer id;

    m_mutex.Wait();

    do {
      id = ++m_nextId;
    } while (!insert(value_type(id)).second);

    m_mutex.Signal();

    return id;
  }


  void Remove(PNotifierIdentifer id)
  {
    if (m_state != 1)
      return;

    m_mutex.Wait();

    erase(id);

    m_mutex.Signal();
  }


  bool Exists(PNotifierIdentifer id)
  {
    if (m_state != 1)
      return false;

    PWaitAndSignal mutex(m_mutex);
    return find(id) != end();
  }
};

static PValidatedNotifierSet s_ValidatedTargets;


PValidatedNotifierTarget::PValidatedNotifierTarget()
{
  m_validatedNotifierId = s_ValidatedTargets.Add();
}


PValidatedNotifierTarget::~PValidatedNotifierTarget()
{
  s_ValidatedTargets.Remove(m_validatedNotifierId);
}


bool PValidatedNotifierTargetExists(PNotifierIdentifer id)
{
  return s_ValidatedTargets.Exists(id);
}


//////////////////////////////////////////////////////////////////////////////

class PAsyncNotifierQueue : std::queue<PAsyncNotifierCallback *>
{
  PSemaphore             m_count;
  PAsyncNotifierTarget * m_target;

public:
  PAsyncNotifierQueue(PAsyncNotifierTarget * target)
    : m_count(0, INT_MAX)
    , m_target(target)
  {
  }


  ~PAsyncNotifierQueue()
  {
    m_target = NULL;
  }


  void Queue(PAsyncNotifierCallback * callback)
  {
    push(callback);
    m_count.Signal();
    m_target->AsyncNotifierSignal();
  }


  PAsyncNotifierCallback * GetCallback(PAsyncNotifierTarget * target, const PTimeInterval & wait)
  {
    if (!PAssert(target == m_target, "PAsyncNotifier mismatch"))
      return NULL;

    if (!m_count.Wait(wait))
      return NULL;
    
    if (!PAssert(!empty(), "PAsyncNotifier queue empty"))
      return NULL;

    PAsyncNotifierCallback * callback = front();
    pop();

    if (!PAssert(callback != NULL, "PAsyncNotifier callback NULL"))
      return NULL;

    return callback;
  }
};


class PAsyncNotifierQueueMap : std::map<PNotifierIdentifer, PAsyncNotifierQueue>
{
  unsigned m_state; // 0 = pre-constructor, 1 = active, 2 = destroyed
  unsigned m_nextId;
  PMutex   m_mutex;

public:
  PAsyncNotifierQueueMap()
    : m_state(1)
    , m_nextId(1)
  {
  }


  ~PAsyncNotifierQueueMap()
  {
    m_state = 2;
  }


  PNotifierIdentifer Add(PAsyncNotifierTarget * target)
  {
    if (m_state != 1)
      return 0;

    PNotifierIdentifer id;

    m_mutex.Wait();

    do {
      id = ++m_nextId;
    } while (!insert(value_type(id, target)).second);

    m_mutex.Signal();

    return id;
  }


  void Remove(PNotifierIdentifer id)
  {
    if (m_state != 1)
      return;

    m_mutex.Wait();

    erase(id);

    m_mutex.Signal();
  }


  void Queue(PNotifierIdentifer id, PAsyncNotifierCallback * callback)
  {
    if (m_state != 1)
      return;

    m_mutex.Wait();

    iterator it = find(id);
    if (it == end())
      delete callback;
    else
      it->second.Queue(callback);

    m_mutex.Signal();
  }


  bool Execute(PNotifierIdentifer id, PAsyncNotifierTarget * target, const PTimeInterval & wait)
  {
    if (m_state != 1)
      return false;

    PAsyncNotifierCallback * callback = NULL;

    m_mutex.Wait();

    iterator it = find(id);
    if (PAssert(it != end(), "PAsyncNotifier missing"))
      callback = it->second.GetCallback(target, wait);

    m_mutex.Signal();

    if (callback == NULL)
      return false;

    callback->Call();
    return true;
  }
};

static PAsyncNotifierQueueMap s_AsyncTargetQueues;


PAsyncNotifierTarget::PAsyncNotifierTarget()
{
  m_asyncNotifierId = s_AsyncTargetQueues.Add(this);
}


PAsyncNotifierTarget::~PAsyncNotifierTarget()
{
  s_AsyncTargetQueues.Remove(m_asyncNotifierId);
}


void PAsyncNotifierCallback::Queue(PNotifierIdentifer id, PAsyncNotifierCallback * callback)
{
  s_AsyncTargetQueues.Queue(id, callback);
}


bool PAsyncNotifierTarget::AsyncNotifierExecute(const PTimeInterval & wait)
{
  return s_AsyncTargetQueues.Execute(m_asyncNotifierId, this, wait);
}


void PAsyncNotifierTarget::AsyncNotifierSignal()
{
}


// End of File ///////////////////////////////////////////////////////////////
