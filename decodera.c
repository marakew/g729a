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

/*-----------------------------------------------------------------*
 * Main program of the G.729a 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include "typedef.h"
#include "ld8a.h"

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/
void g729a_init_decoder(decoder_state *state)
{  
	int i;
	for (i=0; i<M; i++) state->synth_buf[i] = (F)0.0;
	state->synth = state->synth_buf + M;

	init_decod_ld8a(state);
	init_post_filter(&state->post_filter);
	init_post_process(&state->post_process);
}

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 * parm buffer length 11                                           *
 * synth_short buffer space length (>=L_FRAME sizeof(short) bytes) *                             *
 * bad frame indicator (bfi)							           *
 *-----------------------------------------------------------------*/
int g729a_decoder(decoder_state *state, unsigned char * bitstream, short *synth_short, int frame_size)
{
	int  i; 
	FLOAT temp;
	FLOAT  Az_dec[MP1*2];            /* Decoded Az for post-filter */
	int T2[2];                       /* Decoded Pitch              */
	int parm[PRM_SIZE+1];            /* Synthesis parameters */

	if (frame_size != 2 && frame_size != 10)
		return -1;

	bits2prm_ld8k(bitstream, &parm[0]);	
	parm[3] = check_parity_pitch(parm[2], parm[3]);	/* get parity check result */

	decod_ld8a(state, parm, state->synth, Az_dec, T2); /* decoder */

	post_filter(&state->post_filter, state->synth, Az_dec, T2); /* Post-filter */

	post_process(&state->post_process, state->synth, L_FRAME); /* Highpass filter */

	/*---------------------------------------------------------------*
	 * writes a FLOAT array as a Short to a output buf    *
	 *---------------------------------------------------------------*/
	for(i=0; i < L_FRAME; i++)
	{
		/* round and convert to int  */
	        temp = state->synth[i];
	        if (temp >= (F)0.0)
			temp += (F)0.5;
	        else  temp -= (F)0.5;
	        if (temp >  (F)32767.0 ) temp =  (F)32767.0;
	        if (temp < (F)-32768.0 ) temp = (F)-32768.0;
	        synth_short[i] = (INT16) temp;
	}

	return 0;
}

