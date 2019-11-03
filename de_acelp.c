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

#include "typedef.h"
#include "ld8a.h"

/*-----------------------------------------------------------*
 *  Function  decod_ACELP()                                  *
 *  ~~~~~~~~~~~~~~~~~~~~~~~                                  *
 *   Algebraic codebook decoder.                             *
 *----------------------------------------------------------*/

void decod_ACELP(
 int sign,              /* input : signs of 4 pulses     */
 int index,             /* input : positions of 4 pulses */
 FLOAT cod[]            /* output: innovative codevector */
)
{
   int pos[4];
   int i, j;

   /* decode the positions of 4 pulses */

   i = index & 7;
   pos[0] = i*5;

   index >>= 3;
   i = index & 7;
   pos[1] = i*5 + 1;

   index >>= 3;
   i = index & 7;
   pos[2] = i*5 + 2;

   index >>= 3;
   j = index & 1;
   index >>= 1;
   i = index & 7;
   pos[3] = i*5 + 3 + j;

   /* find the algebraic codeword */

   set_zero(cod, L_SUBFR);

   /* decode the signs of 4 pulses */

   for (j=0; j<4; j++)
   {

     i = sign & 1;
     sign >>= 1;

     if (i != 0) {
       cod[pos[j]] = (F)1.0;
     }
     else {
       cod[pos[j]] = (F)-1.0;
     }
   }

   return;
}
