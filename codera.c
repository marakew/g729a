/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729 Annex A
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
 File : CODERA.C
 Used for the floating point version of G.729A only
 (not for G.729 main body)
*/

#include "typedef.h"
#include "ld8a.h"
extern FLOAT *new_speech;           /* Pointer to new speech data   */
static int prm[PRM_SIZE];           /* Transmitted parameters        */

/*-------------------------------------------------*
 * Initialization of the coder.                    *
 *-------------------------------------------------*/
void va_g729a_init_encoder()
{
   init_pre_process();
   init_coder_ld8a();           /* Initialize the coder             */
}

/*---------------------------------------------------------------------*
 * L_FRAME data are read. (L_FRAME = number of speech data per frame)  *  
 * output PRM_SIZE int encoded data                                    *
 *---------------------------------------------------------------------*/
void va_g729a_encoder(short *speech, unsigned char *bitstream)
{
	INT16  i;
    for (i = 0; i < L_FRAME; i++)  new_speech[i] = (FLOAT) speech[i];

    pre_process( new_speech, L_FRAME);

    coder_ld8a(prm);
    prm2bits_ld8k(prm, bitstream);
}

