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

/*-----------------------------------------------------------------*
 *   Functions coder_ld8a and init_coder_ld8a                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  init_coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  coder_ld8a(Short ana[]);                                       *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

#include <math.h>
#include "typedef.h"
#include "ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_ld8a.h"

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

/*--------------------------------------------------------*
 *         Static memory allocation.                      *
 *--------------------------------------------------------*/

        /* LSP Line spectral frequencies */

 static FLOAT lsp_old[M] =
     { (F)0.9595,  (F)0.8413,  (F)0.6549,  (F)0.4154,  (F)0.1423,
      (F)-0.1423, (F)-0.4154, (F)-0.6549, (F)-0.8413, (F)-0.9595};

 static FLOAT past_qua_en[4]={(F)-14.0,(F)-14.0,(F)-14.0,(F)-14.0};

/*-----------------------------------------------------------------*
 *   Function  init_coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  init_coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *-----------------------------------------------------------------*/

void init_coder_ld8a(encoder_state *state)
{

  /*----------------------------------------------------------------------*
  *      Initialize pointers to speech vector.                            *
  *                                                                       *
  *                                                                       *
  *   |--------------------|-------------|-------------|------------|     *
  *     previous speech           sf1           sf2         L_NEXT        *
  *                                                                       *
  *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
  *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
  *   |                   <-- present frame (L_FRAME) -->                 *
  * old_speech            |              <-- new speech (L_FRAME) -->     *
  * p_window              |              |                                *
  *                     speech           |                                *
  *                             new_speech                                *
  *-----------------------------------------------------------------------*/

   state->new_speech = state->old_speech + L_TOTAL - L_FRAME;         /* New speech     */
   state->speech     = state->new_speech - L_NEXT;                    /* Present frame  */
   state->p_window   = state->old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */

   /* Initialize static pointers */

   state->wsp    = state->old_wsp + PIT_MAX;
   state->exc    = state->old_exc + PIT_MAX + L_INTERPOL;

   /* Static vectors to zero */

   set_zero(state->old_speech, L_TOTAL);
   set_zero(state->old_exc, PIT_MAX+L_INTERPOL);
   set_zero(state->old_wsp, PIT_MAX);
   set_zero(state->mem_w,   M);
   set_zero(state->mem_w0,  M);
   set_zero(state->mem_zero, M);
   state->sharp = SHARPMIN;

   copy(lsp_old, state->lsp_old, M);
   copy(state->lsp_old, state->lsp_old_q, M);

   lsp_encw_reset(&state->lsp_state);
   init_exc_err(state->cng_state.exc_err);

   /* For G.729B */
   /* Initialize VAD/DTX parameters */
   state->seed = INIT_SEED;
   state->pastVad = 1;
   state->ppastVad = 1;
   vad_init(&state->vad_state);
   init_lsfq_noise(state->cng_state.noise_fg);

   copy(past_qua_en, state->past_qua_en, 4);
   return;
}

/*-----------------------------------------------------------------*
 *   Function coder_ld8a                                           *
 *            ~~~~~~~~~~                                           *
 *  coder_ld8a(int    ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void coder_ld8a(encoder_state *state,
 int ana[],                   /* output: analysis parameters */
 int frame,                   /* input : frame counter */
 int dtx_enable               /* input : DTX enable flag */
)
{
   /* LPC coefficients */

   FLOAT Aq_t[(MP1)*2];         /* A(z) quantized for the 2 subframes   */
   FLOAT Ap_t[(MP1)*2];         /* A(z) with spectral expansion         */
   FLOAT *Aq, *Ap;              /* Pointer on Aq_t  and Ap_t            */

   /* Other vectors */

   FLOAT h1[L_SUBFR];           /* Impulse response h1[]              */
   FLOAT xn[L_SUBFR];           /* Target vector for pitch search     */
   FLOAT xn2[L_SUBFR];          /* Target vector for codebook search  */
   FLOAT code[L_SUBFR];         /* Fixed codebook excitation          */
   FLOAT y1[L_SUBFR];           /* Filtered adaptive excitation       */
   FLOAT y2[L_SUBFR];           /* Filtered fixed codebook excitation */
   FLOAT g_coeff[5];            /* Correlations between xn, y1, & y2:
                                   <y1,y1>, <xn,y1>, <y2,y2>, <xn,y2>,<y1,y2>*/

   /* Scalars */

   int i, j, i_subfr;
   int T_op, T0, T0_min, T0_max, T0_frac;
   int index;
   FLOAT   gain_pit, gain_code;
   int     taming;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
   {
      /* Temporary vectors */
     FLOAT r[MP1];                    /* Autocorrelations       */
     FLOAT rc[M];                     /* Reflexion coefficients */
     FLOAT lsp_new[M];                /* lsp coefficients       */
     FLOAT lsp_new_q[M];              /* Quantized lsp coeff.   */


     /* For G.729B */
     FLOAT r_nbe[MP1];
     FLOAT lsf_new[M];
     FLOAT lsfq_mem[MA_NP][M];
     int Vad;
     FLOAT Energy_db;

     /* LP analysis */

     autocorr(state->p_window, M, r);             /* Autocorrelations */
     copy(r_nbe, r, MP1);
     lag_window(M, r);                     /* Lag windowing    */
     levinson(r, Ap_t, rc);                /* Levinson Durbin  */
     az_lsp(Ap_t, lsp_new, state->lsp_old);       /* Convert A(z) to lsp */

     if (dtx_enable == 1)
     {
	lsp_lsf(lsp_new, lsf_new, M);
	vad(&state->vad_state, rc[1], lsf_new, r, state->p_window, frame, state->pastVad, state->ppastVad, &Vad, &Energy_db);
	update_cng(&state->cng_state, r_nbe, Vad);
     } else Vad = 1;

    /* ---------------------- */
    /* Case of Inactive frame */
    /* ---------------------- */
    if (Vad == 0)
    {
        //get_freq_prev(&state->lsp_state, &lsfq_mem[i][0], MA_NP);
        for (i=0; i<MA_NP; i++) copy(&state->lsp_state.freq_prev[i][0], &lsfq_mem[i][0], M);
        cod_cng(&state->cng_state, state->exc, state->pastVad, state->lsp_old_q, Aq_t,
                        ana, lsfq_mem, &state->seed);

        //update_freq_prev(&state->lsp_state, &lsfq_mem[i][0], MA_NP);
        for (i=0; i<MA_NP; i++) copy(&lsfq_mem[i][0], &state->lsp_state.freq_prev[i][0], M);
        state->ppastVad = state->pastVad;
        state->pastVad = Vad;

        /* Update wsp, mem_w and mem_w0 */
        Aq = Aq_t;
        for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
          residu(Aq, &state->speech[i_subfr], xn, L_SUBFR);
          weight_az(Aq, GAMMA1, M, Ap_t);

          /* Compute wsp and mem_w */
          Ap = Ap_t + MP1;
          Ap[0] = 4096;
          for (i = 1; i <= M; i++)
             Ap[i] = Ap_t[i] - (F)0.7 * Ap_t[i-1];
          syn_filt(Ap, xn, &state->wsp[i_subfr], L_SUBFR, state->mem_w, 1);

          /* Compute mem_w0 */
          for (i = 0; i < L_SUBFR; i++)
             xn[i] = xn[i] - state->exc[i_subfr+i];  /* residu[] - exc[] */
          syn_filt(Ap_t, xn, xn, L_SUBFR, state->mem_w0, 1);
          Aq += MP1;
        }

        state->sharp = SHARPMIN;

        /* Update memories for next frames */
        copy(&state->old_speech[L_FRAME], &state->old_speech[0], L_TOTAL-L_FRAME);
        copy(&state->old_wsp[L_FRAME], &state->old_wsp[0], PIT_MAX);
        copy(&state->old_exc[L_FRAME], &state->old_exc[0], PIT_MAX+L_INTERPOL);
        return;
    } /* End of inactive frame case */

    /* -------------------- */
    /* Case of Active frame */
    /* -------------------- */

    /* Case of active frame */
    *ana++ = 1;
    state->seed = INIT_SEED;
    state->ppastVad = state->pastVad;
    state->pastVad = Vad;

     /* LSP quantization */

     qua_lsp(&state->lsp_state, lsp_new, lsp_new_q, ana);
     ana += 2;                        /* Advance analysis parameters pointer */

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all subframes                  *
     * The interpolated parameters are in array Aq_t[].                   *
     *--------------------------------------------------------------------*/

    int_qlpc(state->lsp_old_q, lsp_new_q, Aq_t);

    /* Compute A(z/gamma) */

    weight_az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
    weight_az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);

    /* update the LSPs for the next frame */

    copy(lsp_new,   state->lsp_old,   M);
    copy(lsp_new_q, state->lsp_old_q, M);
  }

   /*----------------------------------------------------------------------*
    * - Find the weighted input speech w_sp[] for the whole speech frame   *
    * - Find the open-loop pitch delay for the whole speech frame          *
    * - Set the range for searching closed-loop pitch in 1st subframe      *
    *----------------------------------------------------------------------*/

   residu(&Aq_t[0],   &state->speech[0],       &state->exc[0],       L_SUBFR);
   residu(&Aq_t[MP1], &state->speech[L_SUBFR], &state->exc[L_SUBFR], L_SUBFR);

  {
     FLOAT Ap1[MP1];

     Ap = Ap_t;
     Ap1[0] = (F)1.0;
     for(i=1; i<=M; i++)
       Ap1[i] = Ap[i] - (F)0.7 * Ap[i-1];
     syn_filt(Ap1, &state->exc[0], &state->wsp[0], L_SUBFR, state->mem_w, 1);

     Ap += MP1;
     for(i=1; i<=M; i++)
       Ap1[i] = Ap[i] - (F)0.7 * Ap[i-1];
     syn_filt(Ap1, &state->exc[L_SUBFR], &state->wsp[L_SUBFR], L_SUBFR, state->mem_w, 1);
   }


   /* Find open loop pitch lag for whole speech frame */

   T_op = pitch_ol_fast(state->wsp, L_FRAME);

   /* Range for closed loop pitch search in 1st subframe */

   T0_min = T_op - 3;
   if (T0_min < PIT_MIN) T0_min = PIT_MIN;
   T0_max = T0_min + 6;
   if (T0_max > PIT_MAX)
   {
      T0_max = PIT_MAX;
      T0_min = T0_max - 6;
   }

   /*------------------------------------------------------------------------*
    *          Loop for every subframe in the analysis frame                 *
    *------------------------------------------------------------------------*
    *  To find the pitch and innovation parameters. The subframe size is     *
    *  L_SUBFR and the loop is repeated L_FRAME/L_SUBFR times.               *
    *     - find the weighted LPC coefficients                               *
    *     - find the LPC residual signal                                     *
    *     - compute the target signal for pitch search                       *
    *     - compute impulse response of weighted synthesis filter (h1[])     *
    *     - find the closed-loop pitch parameters                            *
    *     - encode the pitch delay                                           *
    *     - find target vector for codebook search                           *
    *     - codebook search                                                  *
    *     - VQ of pitch and codebook gains                                   *
    *     - update states of weighting filter                                *
    *------------------------------------------------------------------------*/

   Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
   Ap = Ap_t;    /* pointer to weighted LPC coefficients             */

   for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      /*---------------------------------------------------------------*
       * Compute impulse response, h1[], of weighted synthesis filter  *
       *---------------------------------------------------------------*/

      h1[0] = (F)1.0;
      set_zero(&h1[1], L_SUBFR-1);
      syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);

      /*-----------------------------------------------*
       * Find the target vector for pitch search:      *
       *----------------------------------------------*/

      syn_filt(Ap, &state->exc[i_subfr], xn, L_SUBFR, state->mem_w0, 0);

      /*-----------------------------------------------------------------*
       *    Closed-loop fractional pitch search                          *
       *-----------------------------------------------------------------*/

      T0 = pitch_fr3_fast(&state->exc[i_subfr], xn, h1, L_SUBFR, T0_min, T0_max,
                    i_subfr, &T0_frac);

      index = enc_lag3(T0, T0_frac, &T0_min, &T0_max, PIT_MIN, PIT_MAX,
                            i_subfr);

      *ana++ = index;

      if (i_subfr == 0)
        *ana++ = parity_pitch(index);

      /*-----------------------------------------------------------------*
       *   - find filtered pitch exc                                     *
       *   - compute pitch gain and limit between 0 and 1.2              *
       *   - update target vector for codebook search                    *
       *   - find LTP residual.                                          *
       *-----------------------------------------------------------------*/

      syn_filt(Ap, &state->exc[i_subfr], y1, L_SUBFR, state->mem_zero, 0);

      gain_pit = g_pitch(xn, y1, g_coeff, L_SUBFR);

      /* clip pitch gain if taming is necessary */

      taming = test_err(state->cng_state.exc_err, T0, T0_frac);

      if( taming == 1){
        if (gain_pit > GPCLIP) {
          gain_pit = GPCLIP;
      }
    }

    for (i = 0; i < L_SUBFR; i++)
        xn2[i] = xn[i] - y1[i]*gain_pit;

      /*-----------------------------------------------------*
       * - Innovative codebook search.                       *
       *-----------------------------------------------------*/

      index = ACELP_code_A(xn2, h1, T0, state->sharp, code, y2, &i);

      *ana++ = index;           /* Positions index */
      *ana++ = i;               /* Signs index     */

      /*------------------------------------------------------*
       *  - Compute the correlations <y2,y2>, <xn,y2>, <y1,y2>*
       *  - Vector quantize gains.                            *
       *------------------------------------------------------*/

      corr_xy2(xn, y1, y2, g_coeff);
       *ana++ =qua_gain(state->past_qua_en, code, g_coeff, L_SUBFR, &gain_pit, &gain_code,
                                    taming);

      /*------------------------------------------------------------*
       * - Update pitch sharpening "sharp" with quantized gain_pit  *
       *------------------------------------------------------------*/

      state->sharp = gain_pit;
      if (state->sharp > SHARPMAX) state->sharp = SHARPMAX;
      if (state->sharp < SHARPMIN) state->sharp = SHARPMIN;

      /*------------------------------------------------------*
       * - Find the total excitation                          *
       * - update filters' memories for finding the target    *
       *   vector in the next subframe  (mem_w0[])            *
       *------------------------------------------------------*/

      for (i = 0; i < L_SUBFR;  i++)
        state->exc[i+i_subfr] = gain_pit*state->exc[i+i_subfr] + gain_code*code[i];

      update_exc_err(state->cng_state.exc_err, gain_pit, T0);

      for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
        state->mem_w0[j]  = xn[i] - gain_pit*y1[i] - gain_code*y2[i];

      Aq += MP1;           /* interpolated LPC parameters for next subframe */
      Ap += MP1;
   }

   /*--------------------------------------------------*
    * Update signal for next frame.                    *
    * -> shift to the left by L_FRAME:                 *
    *     speech[], wsp[] and  exc[]                   *
    *--------------------------------------------------*/

   copy(&state->old_speech[L_FRAME], &state->old_speech[0], L_TOTAL-L_FRAME);
   copy(&state->old_wsp[L_FRAME], &state->old_wsp[0], PIT_MAX);
   copy(&state->old_exc[L_FRAME], &state->old_exc[0], PIT_MAX+L_INTERPOL);

   return;
}
