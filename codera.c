
#include "typedef.h"
#include "ld8a.h"
#include "dtx.h"
#include "octet.h"

void g729a_init_encoder(encoder_state *state, int dtx_enable)
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
    int prm[PRM_SIZE];

    for (i = 0; i < L_FRAME; i++) state->new_speech[i] = (FLOAT) speech[i];

    pre_process(&state->pre_process, state->new_speech, L_FRAME);

    state->frame = (state->frame != 32768) ? state->frame+1 : 256;
    coder_ld8a(state, prm, state->frame, state->dtx_enable);

    if (prm[0])
       prm2bits_ld8k(prm, bitstream, frame_size);
    else
      *frame_size = 0;
    return 0;
}

