
#include "typedef.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

void g729a_init_decoder(decoder_state *state)
{  
	int i;
	for (i=0; i<M; i++) state->synth_buf[i] = (F)0.0;
	state->synth = state->synth_buf + M;

	init_decod_ld8a(state);
	init_post_filter(&state->post_filter_state);
	init_post_process(&state->post_process);
}

int g729a_decoder(decoder_state *state, unsigned char * bitstream, short *synth_short, int frame_size)
{
	int  i; 
	FLOAT temp;
	FLOAT  Az_dec[MP1*2];
	int T2[2];
	int parm[PRM_SIZE+1];
        int Vad;

	if (frame_size != 2 && frame_size != 10)
		return -1;

	bits2prm_ld8k(bitstream, &parm[0], frame_size);	
	parm[3] = check_parity_pitch(parm[2], parm[3]);

	decod_ld8a(state, parm, state->synth, Az_dec, T2, &Vad);

	post_filter(&state->post_filter_state, state->synth, Az_dec, T2, Vad);

	post_process(&state->post_process, state->synth, L_FRAME);

	for(i=0; i < L_FRAME; i++)
	{
	        temp = state->synth[i];
	        if (temp >= (F)0.0)
                     temp += (F)0.5;
	        else temp -= (F)0.5;

	        if (temp >  (F)32767.0 ) temp =  (F)32767.0;
	        if (temp < (F)-32768.0 ) temp = (F)-32768.0;
	        synth_short[i] = (INT16) temp;
	}

	return 0;
}
