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

/* VAD constants */
extern FLOAT lbf_corr[NP+1];

/* SID LSF quantization */
extern FLOAT noise_fg_sum[MODE][M];
extern FLOAT noise_fg_sum_inv[MODE][M];
extern int PtrTab_1[32];
extern int PtrTab_2[2][16];
extern FLOAT Mp[MODE];

/* SID gain quantization */
extern FLOAT fact[NB_GAIN+1];
extern FLOAT tab_Sidgain[32];



