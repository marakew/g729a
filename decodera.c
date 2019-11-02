
#include "typedef.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

void g729a_decoder_init(decoder_state *state)
{  
	int i;

	init_decod_ld8a(state);
	init_post_filter(&state->post_filter_state);
	init_post_process(&state->post_process);

	for (i=0; i<M; i++) state->synth_buf[i] = (F)0.0;
	state->synth = state->synth_buf + M;

	init_dec_cng(&state->cng_state);
}

int g729a_decoder(decoder_state *state, unsigned char * bitstream, short *synth_short, int frame_size)
{
	int  i; 
	FLOAT temp;
	FLOAT  Az_dec[MP1*2];
	int T2[2];
	int parm[PRM_SIZE+2];
        int Vad;

	if (frame_size != 2 && frame_size != 10)
		return -1;

	bits2prm_ld8k_frame(bitstream, &parm[0], frame_size);	

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

#ifdef TEST
int main(int argc, char *argv[])
{
	decoder_state state;

	FILE *f_serial;
	FILE *f_syn;

#ifdef TEST_CONTROL
	static INT16 serial[SERIAL_SIZE];
#else
	static unsigned char serial[M];
#endif
	static INT16 synth[L_FRAME];

	INT32   frame;
#ifndef TEST_CONTROL
	INT32	frame_size;
#endif

	if (argc != 3)
	{
		printf("Usage :%s bitstream_file  outputspeech_file\n", argv[0]);
		printf("\n");
		printf("Format for bitstream_file:\n");
#ifdef TEST_CONTROL
		printf("  One word (2-bytes) to indicate erasure.\n");
		printf("  One word (2 bytes) to indicate bit rate\n");
		printf("  80 words (2-bytes) containing 80 bits.\n");
#else
		printf("  10 bytes - g729a parameters\n");
#endif
		printf("Format for outputspeech_file:\n");
		printf("  Synthesis is written to a binary file of 16 bits PCM data.\n");
		printf("\n");
		return 1;
	}

	if ( (f_serial = fopen(argv[1], "rb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
		exit(0);
	}

	printf(" Input bitstream file  :  %s\n", argv[1]);

	if ( (f_syn = fopen(argv[2], "wb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
		exit(0);
	}

	printf(" Synthesis speech file :  %s\n", argv[2]);

	g729a_decoder_init(&state);

	frame = 0;
#ifdef TEST_CONTROL
	while(fread((void *)serial, sizeof(INT16), SERIAL_SIZE, f_serial) == SERIAL_SIZE)
#else
	while(fread((void *)serial, sizeof(unsigned char), M, f_serial) == M)
#endif
	{
		printf("Frame: %d\r", frame++);
#ifdef TEST_CONTROL
	     {
		int  i; 
		FLOAT temp;
		FLOAT  Az_dec[MP1*2];
		int T2[2];
		int parm[PRM_SIZE+2];
	        int Vad;

		bits2prm_ld8k(&serial[2], &parm[1]);

		if (serial[0] == SYNC_WORD) {
			parm[0] = 0;           /* No frame erasure */
		} else {
			parm[0] = 1;           /* frame erased     */
		}

		parm[4] = check_parity_pitch(parm[3], parm[4]);

		decod_ld8a(&state, parm, state.synth, Az_dec, T2, &Vad);

		post_filter(&state.post_filter_state, state.synth, Az_dec, T2, Vad);

		post_process(&state.post_process, state.synth, L_FRAME);

		for(i=0; i < L_FRAME; i++)
		{
		        temp = state.synth[i];
		        if (temp >= (F)0.0)
	                     temp += (F)0.5;
		        else temp -= (F)0.5;
	
		        if (temp >  (F)32767.0 ) temp =  (F)32767.0;
		        if (temp < (F)-32768.0 ) temp = (F)-32768.0;
		        synth[i] = (INT16) temp;
		}
	      }
#else
		frame_size = 10;
		g729a_decoder(&state, serial, synth, frame_size);
#endif
		fwrite((void *)synth, sizeof(INT16), L_FRAME, f_syn);
	}

	fclose(f_syn);
	fclose(f_serial);

	return 0;
}
#endif
