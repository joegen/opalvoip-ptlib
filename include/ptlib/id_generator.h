/*
 * handlegen.h
 *
 * Class that generates unique numbers in a thread safe manner.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2013 Vox Lucida
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

#ifndef PTLIB_ID_GENERATOR_H
#define PTLIB_ID_GENERATOR_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <set>
#include <ptlib/mutex.h>



/**This class generates unique numerical "handles".
   This makes efforts to be global static constrictor/destructor and thread
   safe in it's operations.
 */
class PIdGenerator
{
public:
  typedef unsigned int Handle;
  static const Handle Invalid; // Always zero

private:
  Handle             m_nextId;
  std::set<Handle>   m_inUse;
  PCriticalSection * m_mutex;

public:
  PIdGenerator();
  ~PIdGenerator();

  Handle Create();
  void Release(Handle id);
  bool IsValid(Handle id) const;
};


#endif // PTLIB_ID_GENERATOR_H


// End Of File ///////////////////////////////////////////////////////////////
