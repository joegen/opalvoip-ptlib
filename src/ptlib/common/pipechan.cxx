/*
 * pipechan.cxx
 *
 * Sub-process communicating with pipe I/O channel class
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pipechan.cxx,v $
 * Revision 1.4  1998/10/26 09:11:06  robertj
 * Added ability to separate out stdout from stderr on pipe channels.
 *
 * Revision 1.3  1998/09/23 06:22:31  robertj
 * Added open source copyright license.
 *
 * Revision 1.2  1996/05/09 12:18:41  robertj
 * Fixed syntax error found by Mac platform.
 *
 * Revision 1.1  1996/04/14 02:54:14  robertj
 * Initial revision
 *
 */

#include <ptlib.h>

#include <ctype.h>


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

#if defined(_PPIPECHANNEL)

PBASEARRAY(PConstCharStarArray, const char *);

PPipeChannel::PPipeChannel(const PString & subProgram,
                           const PStringArray & arguments,
                           OpenMode mode,
                           BOOL searchPath,
                           BOOL stderrSeparate)
{
  PConstCharStarArray args(arguments.GetSize()+1);
  PINDEX i;
  for (i = 0; i < arguments.GetSize(); i++)
    args[i] = arguments[i];
  args[i] = NULL;
  Construct(subProgram, args, mode, searchPath, stderrSeparate);
}


PPipeChannel::PPipeChannel(const PString & subProgram,
                           OpenMode mode,
                           BOOL searchPath,
                           BOOL stderrSeparate)
{
  Construct(subProgram, NULL, mode, searchPath, stderrSeparate);
}


PPipeChannel::PPipeChannel(const PString & subProgram,
                           const char * const * arguments,
                           OpenMode mode,
                           BOOL searchPath,
                           BOOL stderrSeparate)
{
  Construct(subProgram, arguments, mode, searchPath, stderrSeparate);
}


PObject::Comparison PPipeChannel::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PPipeChannel::Class()), PInvalidCast);
  return subProgName.Compare(((const PPipeChannel &)obj).subProgName);
}


PString PPipeChannel::GetName() const
{
  return subProgName;
}


#endif


// End Of File ///////////////////////////////////////////////////////////////
