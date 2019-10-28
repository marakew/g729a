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

int bad_lsf;        /* bad LSF indicator   */
/*
   This variable should be always set to zero unless transmission errors
   in LSP indices are detected.
   This variable is useful if the channel coding designer decides to
   perform error checking on these important parameters. If an error is
   detected on the  LSP indices, the corresponding flag is
   set to 1 signalling to the decoder to perform parameter substitution.
   (The flags should be set back to 0 for correct transmission).
*/

static FLOAT  synth_buf[L_FRAME+M];     /* Synthesis                  */
FLOAT  *synth;
static FLOAT  Az_dec[MP1*2];            /* Decoded Az for post-filter */
static int T2[2];                       /* Decoded Pitch              */
static int parm[PRM_SIZE+1];            /* Synthesis parameters */

/*-----------------------------------------------------------------*
 *           Initialization of decoder                             *
 *-----------------------------------------------------------------*/
void va_g729a_init_decoder()
{  
	int i;
	for (i=0; i<M; i++) synth_buf[i] = (F)0.0;
	synth = synth_buf + M;

	bad_lsf = 0;          /* Initialize bad LSF indicator */
	init_decod_ld8a();
	init_post_filter();
	init_post_process();
}

/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 * parm buffer length 11                                           *
 * synth_short buffer space length (>=L_FRAME sizeof(short) bytes) *                             *
 * bad frame indicator (bfi)							           *
 *-----------------------------------------------------------------*/
void va_g729a_decoder(unsigned char * bitstream, short *synth_short, int bfi)
{
    int  i; 
    FLOAT temp;

    bits2prm_ld8k(bitstream, &parm[0]);	
	parm[3] = check_parity_pitch(parm[2], parm[3] ); /* get parity check result */

	decod_ld8a(parm, synth, Az_dec, T2, bfi);             /* decoder */

	post_filter(synth, Az_dec, T2);                  /* Post-filter */

	post_process(synth, L_FRAME);                    /* Highpass filter */

	/*---------------------------------------------------------------*
     * writes a FLOAT array as a Short to a output buf    *
     *---------------------------------------------------------------*/
	for(i=0; i < L_FRAME; i++)
    {
		/* round and convert to int  */
        temp = synth[i];
        if (temp >= (F)0.0)
			temp += (F)0.5;
        else  temp -= (F)0.5;
        if (temp >  (F)32767.0 ) temp =  (F)32767.0;
        if (temp < (F)-32768.0 ) temp = (F)-32768.0;
        synth_short[i] = (INT16) temp;
    }
}

