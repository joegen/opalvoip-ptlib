/*
 * notifier_ext.h
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
 */

#ifndef PTLIB_NOTIFIER_EXT_H
#define PTLIB_NOTIFIER_EXT_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <list>


///////////////////////////////////////////////////////////////////////

typedef unsigned long PNotifierIdentifer;

/** Validated PNotifier class.
    This uses a mechanism to assure the caller that the target class still exists
    rather than simply calling through a pointer as the base PNotifierFuntion class
    will. Thus the call simply will not happen rather than crash.

    A notification target class must derive from PValidatedNotifierTarget, usually
    as a multiple inheritance, which will associate a unique ID with the
    specific class instance. This is saved in a global table. When the object
    is destoyed the ID is removed from that table so the caller knows if the
    target still exists or not.

    As well as deriving from PValidatedNotifierTarget class, the notifier functions
    must be declared using the PDECLARE_VALIDATED_NOTIFIER rather than the
    usual PDECLARE_NOTIFIER macro.
  */
class PValidatedNotifierTarget
{
  public:
    PValidatedNotifierTarget();
    virtual ~PValidatedNotifierTarget();

    // Do not copy the id across
    PValidatedNotifierTarget(const PValidatedNotifierTarget &);
    void operator=(const PValidatedNotifierTarget &) { }

    static bool Exists(PNotifierIdentifer id);

  private:
    PNotifierIdentifer m_validatedNotifierId;

  template <typename ParmType> friend class PValidatedNotifierFunction;
};


/**
   This is an abstract class for which a descendent is declared for every
   function that may be called. The <code>PDECLARE_VALIDATED_NOTIFIER</code>
   macro makes this declaration.

   See PNotifierFunctionTemplate for more information.
  */
template <typename ParamType>
class PValidatedNotifierFunction : public PNotifierFunctionTemplate<ParamType>
{
  typedef PNotifierFunctionTemplate<ParamType> Parent;
  PCLASSINFO(PValidatedNotifierFunction, Parent);

  public:
    PValidatedNotifierFunction(void * voidTarget, PValidatedNotifierTarget * notifierTarget)
      : Parent(voidTarget)
      , m_targetID(notifierTarget->m_validatedNotifierId)
    { }

    virtual void * GetTarget() const
    {
      return PValidatedNotifierTarget::Exists(this->m_targetID) ? this->m_target : NULL;
    }

  protected:
    PNotifierIdentifer m_targetID;
};


/** Declare a validated notifier object class.
    See PDECLARE_NOTIFIER2 for more information.
  */
#define PDECLARE_VALIDATED_NOTIFIER_EXT(notifierType, notifierArg, notifiee, func, ParamType, ParamArg) \
            PDECLARE_NOTIFIER_COMMON(notifierType, notifierArg, notifiee, func, ParamType, ParamArg, PValidatedNotifierFunction<ParamType>)

/** Declare a validated notifier object class.
    See PDECLARE_NOTIFIER2 for more information.
  */
#define PDECLARE_VALIDATED_NOTIFIER2(notifierType,   notifiee, func, ParamType  ) \
     PDECLARE_VALIDATED_NOTIFIER_EXT(notifierType, , notifiee, func, ParamType, )

/// Declare validated PNotifier derived class with P_INT_PTR parameter. Uses PDECLARE_VALIDATED_NOTIFIER2 macro.
#define PDECLARE_VALIDATED_NOTIFIER(notifierType, notifiee, func) \
       PDECLARE_VALIDATED_NOTIFIER2(notifierType, notifiee, func, P_INT_PTR)


///////////////////////////////////////////////////////////////////////

struct PAsyncNotifierCallback
{
  virtual ~PAsyncNotifierCallback() { }
  virtual void Call() = 0;

  static void Queue(PNotifierIdentifer id, PAsyncNotifierCallback * callback);
};


/** Asynchronous PNotifier class.
    This is a notification mechanism disconnects the caller from the target
    via a queue. The primary use would be to assure that the target class
    instance is only accesses via a specific thread.

    A notification target class must derive from PAsyncNotifierTarget, usually
    as a multiple inheritance, which will associate a a queue with the
    specific class instance. 

    As well as deriving from PAsyncNotifierTarget class, the notifier functions
    must be declared using the PDECLARE_ASYNC_NOTIFIER rather than the
    usual PDECLARE_NOTIFIER macro.
  */
class PAsyncNotifierTarget
{
  public:
    PAsyncNotifierTarget();
    virtual ~PAsyncNotifierTarget();

    /**Execute any notifications queued.
       The target must call this function for the asynchronous notifications
       to occur.

       @return true if a notification was executed.
      */
    bool AsyncNotifierExecute(
      const PTimeInterval & wait = 0  ///< Time to wait for a notification
    );

    /**Signal the target that there are notifications pending.
       The infrastructure will call this virtual from a random thread to
       indicate that a notification has been queued. What happens is
       applicaation dependent, but typically this would indicate to a main
       thread that it is time to call AsyncNotifierExecute(). For example for
       a GUI, this would post a message to the windowing system. That message
       is captured in teh GUI thread and AsyncNotifierExecute() called.
      */
    virtual void AsyncNotifierSignal();

  private:
    PNotifierIdentifer m_asyncNotifierId;

  template <typename ParmType> friend class PAsyncNotifierFunction;
};


/**
   This is an abstract class for which a descendent is declared for every
   function that may be called. The <code>PDECLARE_ASYNC_NOTIFIER</code>
   macro makes this declaration.

   See PNotifierFunctionTemplate for more information.

   Note: the user must be very careful with the type of ParamType. The lifetime
   must be guaranteed until the PAsyncNotifierTarget::AsyncNotifierExecute()
   function is called. It is usually easier to simply make sure the data is
   passed by value and never pointer or reference.
  */
template <typename ParamType>
class PAsyncNotifierFunction : public PNotifierFunctionTemplate<ParamType>
{
  typedef PNotifierFunctionTemplate<ParamType> Parent;
  PCLASSINFO(PAsyncNotifierFunction, Parent);

  public:
    PAsyncNotifierFunction(void * voidTarget, PAsyncNotifierTarget * notifierTarget)
      : Parent(voidTarget)
      , m_targetID(notifierTarget->m_asyncNotifierId)
    { }

  protected:
    PNotifierIdentifer m_targetID;

    template <class Target>
    class TypedCallback : public PAsyncNotifierCallback
    {
      private:
        const Target  & m_target;
        PObject       & m_notifier;
        ParamType       m_extra;

      public:
        TypedCallback(const Target & target, PObject & notifier, ParamType extra)
          : m_target(target)
          , m_notifier(notifier)
          , m_extra(extra)
        { }

        virtual void Call()
        {
          m_target.AsyncCall(m_notifier, m_extra);
        }
    };
};


/** Declare an asynchronous notifier object class.
    See PDECLARE_NOTIFIER_EXT for more information.
  */
#define PDECLARE_ASYNC_NOTIFIER_EXT(notifierType, notifierArg, notifiee, func, ParamType, ParamArg) \
       PDECLARE_NOTIFIER_COMMON1(notifierType, notifiee, func, ParamType, PAsyncNotifierFunction<ParamType>) \
       { PAsyncNotifierCallback::Queue(m_targetID, new TypedCallback<func##_PNotifier>(*this, note, extra)); } \
       void AsyncCall(PObject & note, ParamType extra) const \
       PDECLARE_NOTIFIER_COMMON2(notifierType, notifierArg, notifiee, func, ParamType, ParamArg, PAsyncNotifierFunction<ParamType>)

/** Declare an asynchronous notifier object class.
    See PDECLARE_NOTIFIER_EXT for more information.
  */
#define PDECLARE_ASYNC_NOTIFIER2(notifierType, notifiee, func, ParamType) \
       PDECLARE_ASYNC_NOTIFIER_EXT(notifierType, , notifiee, func, ParamType, )

/// Declare an asynchronous PNotifier derived class with P_INT_PTR parameter. Uses PDECLARE_NOTIFIER_EXT macro.
#define PDECLARE_ASYNC_NOTIFIER(notifierType, notifiee, func) \
       PDECLARE_ASYNC_NOTIFIER2(notifierType, notifiee, func, P_INT_PTR)


///////////////////////////////////////////////////////////////////////

/**Maintain a list of notifiers to be called all at once.
  */
template <typename ParamType>
class PNotifierListTemplate : public PObject
{
  PCLASSINFO(PNotifierListTemplate, PObject);
  private:
    typedef PNotifierTemplate<ParamType> Notifier;
    typedef std::list<Notifier> List;
    List m_list;

  public:
    /// Indicate the number of notifiers in list.
    PINDEX GetSize() const { return this->m_list.size(); }

    /// Add a notifier to the list
    bool Add(const Notifier & handler)
    {
        if (std::find(this->m_list.begin(), this->m_list.end(), handler) != this->m_list.end())
            return false;
        this->m_list.push_back(handler);
        return true;
    }

    /// Remove notifier from teh list
    bool Remove(const Notifier & handler)
    {
      typename List::iterator it = std::find(this->m_list.begin(), this->m_list.end(), handler);
      if (it == this->m_list.end())
          return false;
      this->m_list.erase(it);
      return true;
    }

    struct IsTarget : public std::unary_function<PObject, bool> 
    {
      PObject * m_obj;
      IsTarget(PObject * obj) : m_obj(obj) { }
      bool operator()(Notifier & test) const { return m_obj == test.GetTarget(); }
    };

    /// Remove all notifiers that use the specified target object.
    void RemoveTarget(PObject * obj)
    {
      this->m_list.remove_if(IsTarget(obj));
    }

    /// Execute all notifiers in the list.
    bool operator()(PObject & obj, ParamType param)
    {
      if (this->m_list.empty())
        return false;
      for (typename List::iterator it = this->m_list.begin(); it != this->m_list.end() ; ++it)
        (*it)(obj, param);
      return true;
    }
};

typedef PNotifierListTemplate<P_INT_PTR> PNotifierList;


#endif  // PTLIB_NOTIFIER_EXT_H


// End of File ///////////////////////////////////////////////////////////////
