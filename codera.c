
#include "typedef.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

void g729a_encoder_init(encoder_state *state, int dtx_enable)
{
    state->frame = 0;
    state->dtx_enable = dtx_enable;
    init_pre_process(&state->pre_process);
    init_coder_ld8a(state);
    if (dtx_enable)
	init_cod_cng(&state->cng_state);
}

int g729a_encoder(encoder_state *state, short *speech, unsigned char *bitstream, int *frame_size)
{
    INT16  i;
    int prm[PRM_SIZE+1];

    for (i = 0; i < L_FRAME; i++) state->new_speech[i] = (FLOAT) speech[i];

    pre_process(&state->pre_process, state->new_speech, L_FRAME);

    if (state->frame == 32767) state->frame = 256;
    else state->frame++;

    coder_ld8a(state, prm, state->frame, state->dtx_enable);

    if (prm[0])
       prm2bits_ld8k_frame(prm, bitstream, frame_size);
    else
      *frame_size = 0;
    return 0;
}

#ifdef TEST_ENCODER
int main(int argc, char *argv[])
{
	encoder_state state;

	FILE *f_speech;
	FILE *f_serial;

#ifdef TEST_CONTROL
	static INT16 serial[SERIAL_SIZE];
#else
	static unsigned char serial[SERIAL_SIZE];
#endif
	static INT16 sp16[L_FRAME];

	INT32   frame;
	INT32	frame_size;

	/* For G.729B */
	int dtx_enable;

	if (argc != 3)
	{
		printf("Usage :%s  speech_file  bitstream_file \n", argv[0]);
		printf("\n");
		printf("Format for speech_file:\n");
		printf("  Speech is read form a binary file of 16 bits data.\n");
		printf("\n");
		printf("Format for bitstream_file:\n");
#ifdef TEST_CONTROL
		printf("  One word (2-bytes) to indicate erasure.\n");
		printf("  One word (2 bytes) to indicate bit rate\n");
		printf("  80 words (2-bytes) containing 80 bits.\n");
#else
		printf("  10 bytes - g729a parameters\n");
#endif
		printf("DTX flag:\n");
		printf("  0 to disable the DTX\n");
		printf("  1 to enable the DTX\n");
		printf("\n");
		return 1;
	}

	if ( (f_speech = fopen(argv[1], "rb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", argv[0], argv[1]);
		exit(0);
	}

	printf(" Input speech file     :  %s\n", argv[1]);

	if ( (f_serial = fopen(argv[2], "wb")) == NULL) {
		printf("%s - Error opening file  %s !!\n", argv[0], argv[2]);
		exit(0);
	}

	printf(" Output bitstream file :  %s\n", argv[2]);

	dtx_enable = (int)atoi(argv[3]);
	if (dtx_enable == 1)
		printf(" DTX enabled\n");
	else
		printf(" DTX disabled\n");

	g729a_encoder_init(&state, dtx_enable);

	frame = 0;
	while(fread((void *)sp16, sizeof(INT16), L_FRAME, f_speech) == L_FRAME){
		frame++;
		printf("Frame: %d\r", frame);
#ifdef TEST_CONTROL
		INT16  i;
		int prm[PRM_SIZE+1];

		for (i = 0; i < L_FRAME; i++) state.new_speech[i] = (FLOAT) sp16[i];

		pre_process(&state.pre_process, state.new_speech, L_FRAME);

		if (state.frame == 32767) state.frame = 256;
		else state.frame++;

		coder_ld8a(&state, prm, state.frame, state.dtx_enable);

		prm2bits_ld8k(&prm[1], serial);
		fwrite((void *)serial, sizeof(INT16), SERIAL_SIZE, f_serial);
#else
		g729a_encoder(&state, sp16, serial, &frame_size);
		fwrite((void *)serial, sizeof(unsigned char), M, f_serial);
#endif
	}

	fclose(f_serial);
	fclose(f_speech);
	return 0;
}
#endif
