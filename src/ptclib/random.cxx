/*
 * random.cxx
 *
 * Mersenne Twister random number generator.
 * From Makoto Matsumoto and Takuji Nishimura.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
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
 * Based on code originally under the following license:
 *
 * This library is free software; you can redistribute it and/or   
 * modify it under the terms of the GNU Library General Public     
 * License as published by the Free Software Foundation; either    
 * version 2 of the License, or (at your option) any later         
 * version.                                                        
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            
 * See the GNU Library General Public License for more details.    
 * You should have received a copy of the GNU Library General      
 * Public License along with this library; if not, write to the    
 * Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   
 * 02111-1307  USA                                                 
 * 
 * Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.       
 * When you use this, send an email to: matumoto@math.keio.ac.jp   
 * with an appropriate reference to your work.                     
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: random.cxx,v $
 * Revision 1.1  2000/02/17 12:05:02  robertj
 * Added better random number generator after finding major flaws in MSVCRT version.
 *
 */


#ifdef __GNUC__
#pragma implementation "random.h"
#endif

#include <ptlib.h>
#include <ptclib/random.h>


/* Period parameters */  
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)


///////////////////////////////////////////////////////////////////////////////
// PRandom

PRandom::PRandom()
{
  SetSeed((DWORD)(time(NULL)+clock()));
}


PRandom::PRandom(DWORD seed)
{
  SetSeed(seed);
}


void PRandom::SetSeed(DWORD seed)
{
    /* setting initial seeds to mt[N] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
    mt[0] = seed & 0xffffffff;
    for (mti = 1; mti < N; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}


unsigned PRandom::Generate()
{
  unsigned long y;
  static unsigned long mag01[2]={0x0, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N) { /* generate N words at one time */
    int kk;

    for (kk=0;kk<N-M;kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (;kk<N-1;kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

    mti = 0;
  }

  y = mt[mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);

  return y; 
}


unsigned PRandom::Number()
{
  static PMutex mutex;
  PWaitAndSignal wait(mutex);

  static PRandom rand;
  return rand;
}


// End Of File ///////////////////////////////////////////////////////////////
