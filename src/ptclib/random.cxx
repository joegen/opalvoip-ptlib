/*
 * random.cxx
 *
 * ISAAC random number generator by Bob Jenkins.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2001 Equivalence Pty. Ltd.
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
 * Based on code originally by Bob Jenkins.
 *
 * $Revision$
 * $Author$
 * $Date$
 */


#ifdef __GNUC__
#pragma implementation "random.h"
#endif

#include <ptlib.h>
#include <ptclib/random.h>



///////////////////////////////////////////////////////////////////////////////
// PRandom

PRandom::PRandom()
{
  SetSeed((DWORD)PTimer::Tick().GetMilliSeconds());
}


PRandom::PRandom(uint32_t seed)
{
  SetSeed(seed);
}


#define mix(a,b,c,d,e,f,g,h) \
{ \
   a^=b<<11; d+=a; b+=c; \
   b^=c>>2;  e+=b; c+=d; \
   c^=d<<8;  f+=c; d+=e; \
   d^=e>>16; g+=d; e+=f; \
   e^=f<<10; h+=e; f+=g; \
   f^=g>>4;  a+=f; g+=h; \
   g^=h<<8;  b+=g; h+=a; \
   h^=a>>9;  c+=h; a+=b; \
}


void PRandom::SetSeed(uint32_t seed)
{
   int i;
   uint32_t a,b,c,d,e,f,g,h;
   uint32_t *m,*r;
   randa = randb = randc = 0;
   m=randmem;
   r=randrsl;

   for (i=0; i<RandSize; i++)
     r[i] = seed++;

   a=b=c=d=e=f=g=h=0x9e3779b9;  /* the golden ratio */

   for (i=0; i<4; ++i)          /* scramble it */
   {
     mix(a,b,c,d,e,f,g,h);
   }

   /* initialize using the the seed */
   for (i=0; i<RandSize; i+=8)
   {
     a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
     e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
     mix(a,b,c,d,e,f,g,h);
     m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
     m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
   }

   /* do a second pass to make all of the seed affect all of m */
   for (i=0; i<RandSize; i+=8)
   {
     a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
     e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
     mix(a,b,c,d,e,f,g,h);
     m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
     m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
   }

   randcnt=0;
   Generate();            /* fill in the first set of results */
   randcnt=RandSize;  /* prepare to use the first set of results */
}


#define ind(mm,x)  (*(uint32_t *)((BYTE *)(mm) + ((x) & ((RandSize-1)<<2))))

#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (a^(mix)) + *(m2++); \
  *(m++) = y = ind(mm,x) + a + b; \
  *(r++) = b = ind(mm,y>>RandBits) + x; \
}


static uint32_t redistribute(uint32_t value, uint32_t minimum, uint32_t maximum)
{
  if (minimum >= maximum)
    return maximum;
  uint32_t range = maximum - minimum + 1;
  while (value >= range)
    value = (value/range) ^ (value%range);
  return value + minimum;
}


uint32_t PRandom::Generate()
{
  if (randcnt-- == 0) {
    uint32_t a,b,x,y,*m,*mm,*m2,*r,*mend;
    mm=randmem; r=randrsl;
    a = randa; b = randb + (++randc);
    for (m = mm, mend = m2 = m+(RandSize/2); m<mend; )
    {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
    }
    for (m2 = mm; m2<mend; )
    {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
    }
    randb = b; randa = a;

    randcnt = RandSize-1;
  }

  return randrsl[randcnt];
}


uint32_t PRandom::Generate(uint32_t maximum)
{
  return redistribute(Generate(), 0, maximum);
}


uint32_t PRandom::Generate(uint32_t minimum, uint32_t maximum)
{
  return redistribute(Generate(), minimum, maximum);
}


uint32_t PRandom::Number()
{
  static PMutex mutex;
  PWaitAndSignal wait(mutex);

  static PRandom rand;
  return rand;
}

uint32_t PRandom::Number(uint32_t maximum)
{
  return redistribute(Number(), 0, maximum);
}


uint32_t PRandom::Number(uint32_t minimum, uint32_t maximum)
{
  return redistribute(Number(), minimum, maximum);
}


PBYTEArray PRandom::Octets(PINDEX size)
{
  PBYTEArray octets(size);
  Octets(octets.GetPointer(), octets.GetSize());
  return octets;
}


void PRandom::Octets(PBYTEArray & octets, PINDEX size)
{
  if (size != 0)
    Octets(octets.GetPointer(size), size);
  else
    Octets(octets.GetPointer(), octets.GetSize());
}


void PRandom::Octets(BYTE * octets, PINDEX size)
{
  if (octets == NULL || size == 0)
    return;

  static PMutex mutex;
  PWaitAndSignal wait(mutex);

  static PRandom rand;

  uint32_t * uintPtr = (uint32_t *)octets;

  PINDEX i;
  for (i = sizeof(uint32_t); i <= size; i += sizeof(uint32_t))
    *uintPtr++ = rand.Generate();

  for (i -= sizeof(uint32_t); i < size; ++i)
    octets[i] = (BYTE)rand.Generate(0, 255);
}


// End Of File ///////////////////////////////////////////////////////////////
