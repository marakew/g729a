/*
   ITU-T G.729 Annex C - Reference C code for floating point
                         implementation of G.729
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

/*
 File : LSPDEC.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/
#include <math.h>
#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/* Prototype definitions of static functions */
static void lsp_iqua_cs(lsp_decw *l, int prm[], FLOAT lsp[], int erase);

/* static memory */
static FLOAT freq_prev_reset[M] = {  /* previous LSP vector(init) */
 (F)0.285599,  (F)0.571199,  (F)0.856798,  (F)1.142397,  (F)1.427997,
 (F)1.713596,  (F)1.999195,  (F)2.284795,  (F)2.570394,  (F)2.855993
};     /* PI*(float)(j+1)/(float)(M+1) */

/*----------------------------------------------------------------------------
 * Lsp_decw_reset -   set the previous LSP vectors
 *----------------------------------------------------------------------------
 */
void lsp_decw_reset(lsp_decw *l)
{
   int  i;

   for(i=0; i<MA_NP; i++)
     copy (freq_prev_reset, &l->freq_prev[i][0], M );

   l->prev_ma = 0;

   copy (freq_prev_reset, l->prev_lsp, M );

   return;
}


/*----------------------------------------------------------------------------
 * lsp_iqua_cs -  LSP main quantization routine
 *----------------------------------------------------------------------------
 */
static void lsp_iqua_cs(lsp_decw *l,
 int    prm[],          /* input : codes of the selected LSP */
 FLOAT  lsp_q[],        /* output: Quantized LSP parameters  */
 int    erase           /* input : frame erase information   */
)
{
   int  mode_index;
   int  code0;
   int  code1;
   int  code2;
   FLOAT buf[M];


   if(erase==0)                 /* Not frame erasure */
     {
        mode_index = (prm[0] >> NC0_B) & 1;
        code0 = prm[0] & (INT16)(NC0 - 1);
        code1 = (prm[1] >> NC1_B) & (INT16)(NC1 - 1);
        code2 = prm[1] & (INT16)(NC1 - 1);

        lsp_get_quant(lspcb1, lspcb2, code0, code1, code2, fg[mode_index],
              l->freq_prev, lsp_q, fg_sum[mode_index]);

        copy(lsp_q, l->prev_lsp, M );
        l->prev_ma = mode_index;
     }
   else                         /* Frame erased */
     {
       copy(l->prev_lsp, lsp_q, M );

        /* update freq_prev */
       lsp_prev_extract(l->prev_lsp, buf,
          fg[l->prev_ma], l->freq_prev, fg_sum_inv[l->prev_ma]);
       lsp_prev_update(buf, l->freq_prev);
     }
     return;
}
/*----------------------------------------------------------------------------
 * d_lsp - decode lsp parameters
 *----------------------------------------------------------------------------
 */
void d_lsp(lsp_decw *l,
    int     index[],    /* input : indexes                 */
    FLOAT   lsp_q[],    /* output: decoded lsp             */
    int     bfi         /* input : frame erase information */
)
{
   int i;

   lsp_iqua_cs(l, index, lsp_q,bfi); /* decode quantized information */

   /* Convert LSFs to LSPs */

   for (i=0; i<M; i++ )
     lsp_q[i] = (FLOAT)cos(lsp_q[i]);

   return;
}

