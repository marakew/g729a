/*
  ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
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

/*****************************************************************************/
/* auxiliary functions                                                       */
/*****************************************************************************/

#include "typedef.h"
#include "ld8a.h"

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

/*****************************************************************************/
/* Functions used by VAD.C                                                   */
/*****************************************************************************/
void dvsub(FLOAT *in1, FLOAT *in2, FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in1++) - *(in2++);
}

FLOAT dvdot(FLOAT *in1, FLOAT *in2, INT16 npts)
{
    FLOAT accum;
    
    accum = (F)0.0;
    while (npts--)  accum += *(in1++) * *(in2++);
    return(accum);
}

void dvwadd(FLOAT *in1, FLOAT scalar1, FLOAT *in2, FLOAT scalar2,
                        FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in1++) * scalar1 + *(in2++) * scalar2;
}

void dvsmul(FLOAT *in, FLOAT scalar, FLOAT *out, INT16 npts)
{
    while (npts--)  *(out++) = *(in++) * scalar;
}
