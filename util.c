/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
                         Version 1.01 of 15.September.98
*/

/*
----------------------------------------------------------------------
                    COPYRIGHT NOTICE
----------------------------------------------------------------------
   ITU-T G.729 Annex C ANSI C source code
   Copyright (C) 1998, AT&T, France Telecom, NTT, University of
   Sherbrooke.  All rights reserved.

----------------------------------------------------------------------
*/

/*
 File : UTIL.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

/*****************************************************************************/
/* auxiliary functions                                                       */
/*****************************************************************************/

#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
#else
 #include "ld8k.h"
#endif

/*-------------------------------------------------------------------*
 * Function  set zero()                                              *
 *           ~~~~~~~~~~                                              *
 * Set vector x[] to zero                                            *
 *-------------------------------------------------------------------*/

void set_zero(
  FLOAT  x[],       /* (o)    : vector to clear     */
  int L             /* (i)    : length of vector    */
)
{
   int i;

   for (i = 0; i < L; i++)
     x[i] = (F)0.0;

   return;
}

/*-------------------------------------------------------------------*
 * Function  copy:                                                   *
 *           ~~~~~                                                   *
 * Copy vector x[] to y[]                                            *
 *-------------------------------------------------------------------*/

void copy(
  FLOAT  x[],      /* (i)   : input vector   */
  FLOAT  y[],      /* (o)   : output vector  */
  int L            /* (i)   : vector length  */
)
{
   int i;

   for (i = 0; i < L; i++)
     y[i] = x[i];

   return;
}

/* Random generator  */

INT16 random_g729(INT16 *seed)
{
  *seed = (INT16) (*seed * 31821L + 13849L);

  return(*seed);

}
