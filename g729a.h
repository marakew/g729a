
#ifndef _g729a_h_
#define _g729a_h_

#ifdef __cplusplus
extern "C" {
#endif

void g729a_encoder_init(encoder_state *state, int dtx_enable);
int g729a_encoder(encoder_state *state, short *speech, unsigned char *bitstream, int *frame_size);

void g729a_decoder_init(decoder_state *state);
int g729a_decoder(decoder_state *state, unsigned char *bitstream, short *synth_short, int frame_size);

#ifdef __cplusplus
extern "C" {
#endif

#endif
