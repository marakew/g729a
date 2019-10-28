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
 File : BITS.C
 Used for the floating point version of both
 G.729 main body and G.729A
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/

#include "typedef.h"
#include "version.h"
#ifdef VER_G729A
 #include "ld8a.h"
 #include "tab_ld8a.h"
#else
 #include "ld8k.h"
 #include "tab_ld8k.h"
#endif

/* prototypes for local functions */

static void   int2bin(int  value, int  no_of_bits, INT16 *bitstream);
static int    bin2int(int  no_of_bits, INT16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters for 8000 bits/sec are:
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *----------------------------------------------------------------------------
 */

void prm2bits_ld8k(
 int  prm[],            /* input : encoded parameters  */
 unsigned char *bits           /* output: serial bits         */
)
{
   bits[0] = (unsigned char)(prm[0] & 0xff);

   bits[1] = (unsigned char)((prm[1] & 0x03fc) >> 2);

   bits[2] = (unsigned char)((prm[1] & 0x0003) << 6);
   bits[2] |= (unsigned char)((prm[2] & 0x00fc) >> 2);

   bits[3] = (unsigned char)((prm[2] & 0x0003) << 6);
   bits[3] |= (unsigned char)((prm[3] & 0x0001) << 5);
   bits[3] |= (unsigned char)((prm[4] & 0x1f00) >> 8);

   bits[4] = (unsigned char)(prm[4] & 0x00ff);

   bits[5] = (unsigned char)((prm[5] & 0x000f) << 4);
   bits[5] |= (unsigned char)((prm[6] & 0x0078) >> 3);

   bits[6] = (unsigned char)((prm[6] & 0x0007) << 5);
   bits[6] |= (unsigned char)(prm[7] & 0x001f);

   bits[7] = (unsigned char)((prm[8] & 0x1fe0) >> 5);

   bits[8] = (unsigned char)((prm[8] & 0x001f) << 3);
   bits[8] |= (unsigned char)((prm[9] & 0x000e) >> 1);

   bits[9] = (unsigned char)((prm[9] & 0x0001) << 7);
   bits[9] |= (unsigned char)(prm[10] & 0x007f);

   return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 int  value,             /* input : decimal value */
 int  no_of_bits,        /* input : number of bits to use */
 INT16 *bitstream        /* output: bitstream  */
)
{
   INT16 *pt_bitstream;
   int    i, bit;

   pt_bitstream = bitstream + no_of_bits;

   for (i = 0; i < no_of_bits; i++)
   {
     bit = value & 0x0001;      /* get lsb */
     if (bit == 0)
         *--pt_bitstream = BIT_0;
     else
         *--pt_bitstream = BIT_1;
     value >>= 1;
   }
   return;
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k(
 unsigned char *bits,      /* input : serial bits        */
 int prm[]                 /* output: decoded parameters */
)
{
   prm[0] = (int)(bits[0]);
   prm[1] = ((int)bits[1]) << 2;
   prm[1] |= (int)(bits[2] >> 6);
   prm[2] = ((int)(bits[2] & 0x3f)) << 2;
   prm[2] |= (int)(bits[3] >> 6);
   prm[3] = (int)((bits[3] & 0x20) >> 5);
   prm[4] = (int)(bits[3] & 0x1f) << 8;
   prm[4] |= (int)bits[4];
   prm[5] = (int)(bits[5] >> 4);
   prm[6] = (int)(bits[5] & 0x0f) << 3;
   prm[6] |= (int)(bits[6] >> 5);
   prm[7] = (int)(bits[6] & 0x1f);
   prm[8] = (int)bits[7] << 5;
   prm[8] |= (int)(bits[8] >> 3);
   prm[9] = ((int)bits[8] & 0x07) << 1;
   prm[9] |= (int)(bits[9] >> 7);
   prm[10] = (int)bits[9] & 0x7f;

   return;
}


/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static int  bin2int(            /* output: decimal value of bit pattern */
 int  no_of_bits,        /* input : number of bits to read       */
 INT16 *bitstream        /* input : array containing bits        */
)
{
   int    value, i;
   int  bit;

   value = 0;
   for (i = 0; i < no_of_bits; i++)
   {
     value <<= 1;
     bit = *bitstream++;
     if (bit == BIT_1)  value += 1;
   }
   return(value);
}
