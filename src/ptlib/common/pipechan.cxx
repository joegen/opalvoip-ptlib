/*
 * $Id: pipechan.cxx,v 1.1 1996/04/14 02:54:14 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.cxx,v $
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
                const PStringArray & arguments, OpenMode mode, BOOL searchPath)
{
  PConstCharStarArray args(arguments.GetSize()+1);
  for (PINDEX i = 0; i < arguments.GetSize(); i++)
    args[i] = arguments[i];
  args[i] = NULL;
  Construct(subProgram, args, mode, searchPath);
}


PPipeChannel::PPipeChannel(const PString & subProgram,
                                                OpenMode mode, BOOL searchPath)
{
  Construct(subProgram, NULL, mode, searchPath);
}


PPipeChannel::PPipeChannel(const PString & subProgram,
                const char * const * arguments, OpenMode mode, BOOL searchPath)
{
  Construct(subProgram, arguments, mode, searchPath);
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
