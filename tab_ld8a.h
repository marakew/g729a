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

extern FLOAT hamwindow[L_WINDOW];
#if 1
extern FLOAT lwindow[M+3];
#else
extern FLOAT lwindow[M+2];
#endif
extern FLOAT lspcb1[NC0][M];
extern FLOAT lspcb2[NC1][M];
extern FLOAT fg[2][MA_NP][M];
extern FLOAT fg_sum[2][M];
extern FLOAT fg_sum_inv[2][M];
extern FLOAT grid[GRID_POINTS+1];
extern FLOAT freq_prev_reset[M];
extern FLOAT inter_3l[FIR_SIZE_SYN];
extern FLOAT pred[4];
extern FLOAT gbk1[NCODE1][2];
extern FLOAT gbk2[NCODE2][2];
extern int map1[NCODE1];
extern int map2[NCODE2];
extern FLOAT coef[2][2];
extern FLOAT thr1[NCODE1-NCAN1];
extern FLOAT thr2[NCODE2-NCAN2];
extern int imap1[NCODE1];
extern int imap2[NCODE2];
extern FLOAT b100[3];
extern FLOAT a100[3];
extern FLOAT b140[3];
extern FLOAT a140[3];
extern int  bitsno[PRM_SIZE];
extern int  bitsno2[4];
extern FLOAT lsp_old[M];
extern FLOAT past_qua_en[4];
