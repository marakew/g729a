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

/*-------------------------------------------------*
 * Initialization of the coder.                    *
 *-------------------------------------------------*/
void va_g729a_init_encoder(encoder_state *state)
{
   init_pre_process(&state->pre_process);
   init_coder_ld8a(state);           /* Initialize the coder             */
}

/*---------------------------------------------------------------------*
 * L_FRAME data are read. (L_FRAME = number of speech data per frame)  *  
 * output PRM_SIZE int encoded data                                    *
 *---------------------------------------------------------------------*/
void va_g729a_encoder(encoder_state *state, short *speech, unsigned char *bitstream)
{
	INT16  i;
  int prm[PRM_SIZE];           /* Transmitted parameters        */

    for (i = 0; i < L_FRAME; i++)  state->new_speech[i] = (FLOAT) speech[i];

    pre_process(&state->pre_process, state->new_speech, L_FRAME);

    coder_ld8a(state, prm);
    prm2bits_ld8k(prm, bitstream);
}

