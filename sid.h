/*
  ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
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

#define         TRUE 1
#define         FALSE 0
#define         sqr(a)  ((a)*(a))
#define         R_LSFQ 10

void init_lsfq_noise(FLOAT noise_fg[MODE][MA_NP][M]);
void lsfq_noise(FLOAT noise_fg[MODE][MA_NP][M], FLOAT *lsp_new, FLOAT *lspq,
                FLOAT freq_prev[MA_NP][M], int *idx);
void sid_lsfq_decode(FLOAT noise_fg[MODE][MA_NP][M], int *index, FLOAT *lspq,
                     FLOAT freq_prev[MA_NP][M]);





