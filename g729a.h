
#ifndef _g729a_h_
#define _g729a_h_

extern "C" void g729a_init_encoder(encoder_state *state, int dtx_enable);
extern "C" int g729a_encoder(encoder_state *state, short *speech, unsigned char *bitstream, int *frame_size);

extern "C" void g729a_init_decoder(decoder_state *state);
extern "C" int g729a_decoder(decoder_state *state, unsigned char *bitstream, short *synth_short, int frame_size);

#endif
