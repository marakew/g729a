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

/* DTX and Comfort Noise Generator - Encoder part */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "sid.h"

/* Local functions */
static void calc_pastfilt(enc_cng_state *state, FLOAT *Coeff);
static void calc_RCoeff(FLOAT *Coeff, FLOAT *RCoeff);
static int cmp_filt(FLOAT *RCoeff, FLOAT *acf, FLOAT alpha, FLOAT Thresh);
static void calc_sum_acf(FLOAT *acf, FLOAT *sum, int nb);
static void update_sumAcf(enc_cng_state *state,);

/*-----------------------------------------------------------*
* procedure init_Cod_cng:                                   *
*           ~~~~~~~~~~~~                                    *
*   Initialize variables used for dtx at the encoder        *
*-----------------------------------------------------------*/
void init_cod_cng(enc_cng_state *state)
{
    int i;
    
    for(i=0; i<SIZ_SUMACF; i++) state->sumAcf[i] = (F)0.;
    
    for(i=0; i<SIZ_ACF; i++) state->Acf[i] = (F)0.;
    
    for(i=0; i<NB_GAIN; i++) state->ener[i] = (F)0.;
    
    state->cur_gain = 0;
    state->fr_cur = 0;
    state->flag_chang = 0;
    
    return;
}

/*-----------------------------------------------------------*
* procedure cod_cng:                                        *
*           ~~~~~~~~                                        *
*   computes DTX decision                                   *
*   encodes SID frames                                      *
*   computes CNG excitation for encoder update              *
*-----------------------------------------------------------*/
void cod_cng(
    enc_cng_state *state,
    FLOAT *exc,          /* (i/o) : excitation array                     */
    int pastVad,         /* (i)   : previous VAD decision                */
    FLOAT *lsp_old_q,    /* (i/o) : previous quantized lsp               */
    FLOAT *Aq,           /* (o)   : set of interpolated LPC coefficients */
    int *ana,            /* (o)   : coded SID parameters                 */
    FLOAT freq_prev[MA_NP][M], /* (i/o) : previous LPS for quantization        */
    INT16 *seed          /* (i/o) : random generator seed                */
)
{
    int i;
    
    FLOAT curAcf[MP1];
    FLOAT bid[MP1];
    FLOAT curCoeff[MP1];
    FLOAT lsp_new[M];
    FLOAT *lpcCoeff;
    int cur_igain;
    FLOAT energyq;
    
    /* Update Ener */
    for(i = NB_GAIN-1; i>=1; i--) {
        state->ener[i] = state->ener[i-1];
    }
    
    /* Compute current Acfs */
    calc_sum_acf(state->Acf, curAcf, NB_CURACF);
    
    /* Compute LPC coefficients and residual energy */
    if(curAcf[0] == (F)0.) {
        state->ener[0] = (F)0.;                /* should not happen */
    }
    else {
        state->ener[0] = levinson(curAcf, curCoeff, bid);
    }

    /* if first frame of silence => SID frame */
    if(pastVad != 0) {
        ana[0] = 1;
        state->count_fr0 = 0;
        state->nb_ener = 1;
        qua_Sidgain(state->ener, state->nb_ener, &energyq, &cur_igain);
    }
    else {
        state->nb_ener++;
        if(state->nb_ener > NB_GAIN) state->nb_ener = NB_GAIN;
        qua_Sidgain(state->ener, state->nb_ener, &energyq, &cur_igain);

        /* Compute stationarity of current filter   */
        /* versus reference filter                  */
        if(cmp_filt(state->RCoeff, curAcf, state->ener[0], THRESH1) != 0) {
            state->flag_chang = 1;
        }
        
        /* compare energy difference between current frame and last frame */
        if( (FLOAT)fabs(state->prev_energy - energyq) > (F)2.0) state->flag_chang = 1;
        
        state->count_fr0++;
        if(state->count_fr0 < FR_SID_MIN) {
            ana[0] = 0; /* no transmission */
        }
        else {
            if(state->flag_chang != 0) {
                ana[0] = 1;             /* transmit SID frame */
            }
            else{
                ana[0] = 0;
            }
            state->count_fr0 = FR_SID_MIN;   /* to avoid overflow */
        }
    }

    if(ana[0] == 1) {

        /* Reset frame count and change flag */
        state->count_fr0 = 0;
        state->flag_chang = 0;
        
        /* Compute past average filter */
        calc_pastfilt(state->pastCoeff, old_A, old_rc);
        calc_RCoeff(state->pastCoeff, state->RCoeff);
        
        /* Compute stationarity of current filter   */
        /* versus past average filter               */
        
        /* if stationary */
        /* transmit average filter => new ref. filter */
        if(cmp_filt(state->RCoeff, curAcf, state->ener[0], THRESH2) == 0) {
            lpcCoeff = state->pastCoeff;
        }
        
        /* else */
        /* transmit current filter => new ref. filter */
        else {
            lpcCoeff = curCoeff;
            calc_RCoeff(curCoeff, state->RCoeff);
        }
        
        /* Compute SID frame codes */
        az_lsp(lpcCoeff, lsp_new, lsp_old_q); /* From A(z) to lsp */
        
        /* LSP quantization */
        lsfq_noise(state->noise_fg, lsp_new, state->lspSid_q, freq_prev, &ana[1]);
        
        state->prev_energy = energyq;
        ana[4] = cur_igain;
        state->sid_gain = tab_Sidgain[cur_igain];
        
    } /* end of SID frame case */
    
    /* Compute new excitation */
    if(pastVad != 0) {
        state->cur_gain = state->sid_gain;
    }
    else {
        state->cur_gain *= A_GAIN0;
        state->cur_gain += A_GAIN1 * state->sid_gain;
    }
    
    calc_exc_rand(state->exc_err, state->cur_gain, exc, seed, FLAG_COD);
    
    int_qlpc(lsp_old_q, state->lspSid_q, Aq);
    for(i=0; i<M; i++) {
        lsp_old_q[i]  = state->lspSid_q[i];
    }
    
    /* Update sumAcf if fr_cur = 0 */
    if(state->fr_cur == 0) {
        update_sumAcf(state);
    }
    
    return;
}

/*-----------------------------------------------------------*
* procedure update_cng:                                     *
*           ~~~~~~~~~~                                      *
*   updates autocorrelation arrays                          *
*   used for DTX/CNG                                        *
*   If Vad=1 : updating of array sumAcf                     *
*-----------------------------------------------------------*/
void update_cng(
    enc_cng_state *state,
    FLOAT *r,         /* (i) :   frame autocorrelation               */
    int Vad           /* (i) :   current Vad decision                */
)
{
    int i;
    FLOAT *ptr1, *ptr2;
    
    /* Update Acf */
    ptr1 = state->Acf + SIZ_ACF - 1;
    ptr2 = ptr1 - MP1;
    for(i=0; i<(SIZ_ACF-MP1); i++) {
        *ptr1-- = *ptr2--;
    }
    
    /* Save current Acf */
    for(i=0; i<MP1; i++) {
        state->Acf[i] = r[i];
    }
    
    state->fr_cur++;
    if(state->fr_cur == NB_CURACF) {
        state->fr_cur = 0;
        if(Vad != 0) {
            update_sumAcf(state);
        }
    }
    
    return;
}


/*-----------------------------------------------------------*
*         Local procedures                                  *
*         ~~~~~~~~~~~~~~~~                                  *
*-----------------------------------------------------------*/

/* Compute autocorr of LPC coefficients used for Itakura distance */
/******************************************************************/
static void calc_RCoeff(FLOAT *Coeff, FLOAT *RCoeff)
{
    int i, j;
    FLOAT temp;
    
    /* RCoeff[0] = SUM(j=0->M) Coeff[j] ** 2 */
    for(j=0, temp = (F)0.; j <= M; j++) {
        temp += Coeff[j] * Coeff[j];
    }
    RCoeff[0] = temp;
    
    /* RCoeff[i] = SUM(j=0->M-i) Coeff[j] * Coeff[j+i] */
    for(i=1; i<=M; i++) {
        for(j=0, temp=(F)0.; j<=M-i; j++) {
            temp += Coeff[j] * Coeff[j+i];
        }
        RCoeff[i] = (F)2. * temp;
    }
    return;
}

/* Compute Itakura distance and compare to threshold */
/*****************************************************/
static int cmp_filt(FLOAT *RCoeff, FLOAT *acf, FLOAT alpha, FLOAT Thresh)
{
    FLOAT temp1, temp2;
    int i;
    int diff;
    
    temp1 = (F)0.;
    for(i=0; i <= M; i++) {
        temp1 += RCoeff[i] * acf[i];
    }
    
    temp2 = alpha * Thresh;
    if(temp1 > temp2) diff = 1;
    else diff = 0;
    
    return(diff);
}

/* Compute past average filter */
/*******************************/
static void calc_pastfilt(enc_cng_state *state, FLOAT *Coeff)
{
    int i;
    FLOAT s_sumAcf[MP1];
    FLOAT bid[M];

    calc_sum_acf(state->sumAcf, s_sumAcf, NB_SUMACF);
    
    if(s_sumAcf[0] == (F)0.) {
        Coeff[0] = (F)1.;
        for(i=1; i<=M; i++) Coeff[i] = (F)0.;
        return;
    }

    levinson(s_sumAcf, Coeff, bid);
    return;
}

/* Update sumAcf */
/*****************/
static void update_sumAcf(enc_cng_state *state)
{
    FLOAT *ptr1, *ptr2;
    int i;
    
    /*** Move sumAcf ***/
    ptr1 = state->sumAcf + SIZ_SUMACF - 1;
    ptr2 = ptr1 - MP1;
    for(i=0; i<(SIZ_SUMACF-MP1); i++) {
        *ptr1-- = *ptr2--;
    }
    
    /* Compute new sumAcf */
    calc_sum_acf(state->Acf, state->sumAcf, NB_CURACF);
    return;
}

/* Compute sum of acfs (curAcf, sumAcf or s_sumAcf) */
/****************************************************/
static void calc_sum_acf(FLOAT *acf, FLOAT *sum, int nb)
{
    
    FLOAT *ptr1;
    int i, j;
    
    for(j=0; j<MP1; j++) {
        sum[j] = (F)0.;
    }
    ptr1 = acf;
    for(i=0; i<nb; i++) {
        for(j=0; j<MP1; j++) {
            sum[j] += (*ptr1++);
        }
    }
    return;
}
