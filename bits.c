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

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/

#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"

#ifdef TEST_CONTROL
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
 INT16 bits[]           /* output: serial bits         */
)
{
   int  i;
   *bits++ = SYNC_WORD;     /* At receiver this bit indicates BFI */
   *bits++ = SIZE_WORD;     /* Number of bits in this frame       */

   for (i = 0; i < PRM_SIZE; i++)
   {
      int2bin(prm[i], bitsno[i], bits);
      bits += bitsno[i];
   }

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
 INT16 bits[],          /* input : serial bits        */
 int  prm[]             /* output: decoded parameters */
)
{
   int  i;
   for (i = 0; i < PRM_SIZE; i++)
   {
      prm[i] = bin2int(bitsno[i], bits);
      bits  += bitsno[i];
   }

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

#endif

/*----------------------------------------------------------------------------
 * prm2bits_ld8k_frame - converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k_frame - converts serial received bits to  encoder parameter vector
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
/*
struct g729frame
{
//
//0
	unsigned char cb1:8; //24
//1
	unsigned char cb2_1:8;
//2
	unsigned char cb2_2:2; //14
	unsigned char sub1_pp_1:6;
//3
	unsigned char sub1_pp_2:2; //6
	unsigned char parity:1; //5
	unsigned char sub1_ci1_1:5; //0
//
//4
	unsigned char sub1_ci1_2:8; //24
//5
	unsigned char sub1_ci2:4; //20
	unsigned char sub1_pcg_1:4;
//6
	unsigned char sub1_pcg_2:3; //13
	unsigned char sub2_pp:5; //8
//7
	unsigned char sub2_ci1_1:8; //0
//
//8
	unsigned char sub2_ci1_2:5; //
	unsigned char sub2_ci2_1:3;
//9
	unsigned char sub2_ci2_2:1; //7
	unsigned char sub2_pcg:7; //0
};
*/

void prm2bits_ld8k_frame(
 int  prm[],            /* input : encoded parameters  */
 unsigned char *bits,   /* output: serial bits         */
 int *frame_size
)
{
   char ftyp = prm[0];
   if (ftyp == 0)
   {
     *frame_size = 0;
   } else
   if (ftyp == 1) //voice frame
   {
     *frame_size = 10;
#if 1
     unsigned int d0 = (prm[1]<<24) | //8
                      ((prm[2]&0x3ff)<<14) | //10
                      ((prm[3]&0xff)<<6) | //8
                      ((prm[4]&1)<<5) | //1
                      ((prm[5]>>8)&0x1f); //5

     bits[0] = (d0>>24)&0xff;
     bits[1] = (d0>>16)&0xff;
     bits[2] = (d0>>8)&0xff;
     bits[3] = (d0>>0)&0xff;

     unsigned int d1 = (prm[5]<<24) | //8
                      ((prm[6]&0xf)<<20) | //4
                      ((prm[7]&0x7f)<<13) | //7
                      ((prm[8]&0x1f)<<8) | //5
                       (prm[9]>>5)&0xff; //8

     bits[4] = (d1>>24)&0xff;
     bits[5] = (d1>>16)&0xff;
     bits[6] = (d1>>8)&0xff;
     bits[7] = (d1>>0)&0xff;

     unsigned int d2 = ((prm[9]&0x7ff)<<11) | //
                      ((prm[10]&0xf)<<7) |
                       (prm[11]&0x7f);

     bits[8] = (d2>>8)&0xff;
     bits[9] = (d2>>0)&0xff;
#else
     /* MA + 1st stage */
     //7+1
     bits[0] = (unsigned char)(prm[1] & 0xff);

     /* 2nd stage */
     //5*2
     bits[1] = (unsigned char)((prm[2] & 0x03fc) >> 2);
     bits[2] = (unsigned char)((prm[2] & 0x0003) << 6);

     /* first subframe  */
     //8
     bits[2] |= (unsigned char)((prm[3] & 0x00fc) >> 2);
     bits[3] = (unsigned char)((prm[3] & 0x0003) << 6);
     //1
     bits[3] |= (unsigned char)((prm[4] & 0x0001) << 5);
     //13
     bits[3] |= (unsigned char)((prm[5] & 0x1f00) >> 8);
     bits[4] = (unsigned char)(prm[5] & 0x00ff);
     //4
     bits[5] = (unsigned char)((prm[6] & 0x000f) << 4);
     //4+3
     bits[5] |= (unsigned char)((prm[7] & 0x0078) >> 3);
     bits[6] = (unsigned char)((prm[7] & 0x0007) << 5);

     /* second subframe */
     //5
     bits[6] |= (unsigned char)(prm[8] & 0x001f);
     //13
     bits[7] = (unsigned char)((prm[9] & 0x1fe0) >> 5);
     bits[8] = (unsigned char)((prm[9] & 0x001f) << 3);
     //4
     bits[8] |= (unsigned char)((prm[10] & 0x000e) >> 1);
     bits[9] = (unsigned char)((prm[10] & 0x0001) << 7);
     //4+3
     bits[9] |= (unsigned char)(prm[11] & 0x007f);
#endif
   } else
   if (ftyp == 2) //noise frame
   {
     *frame_size = 2;
     bits[0] = (prm[4]);
     bits[1] = (prm[4] << 1) & 0x1f;
   } else
     *frame_size = 0;
   return;
}

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k_frame(
 unsigned char *bits,      /* input : serial bits        */
 int prm[],                /* output: decoded parameters */
 int frame_size
)
{
   prm[0] = 0; //bad frame indicator

   if (frame_size == 0)
   {
     prm[0] = 1;
   } else
   if (frame_size == 2)
   {
     prm[1] = 2; //noise frame

     unsigned int d = (bits[0] << 8) | (bits[1] << 0);
     prm[2] = (int)(d >> 15);
     prm[3] = (int)(d >> 10) & 0x1f;
     prm[4] = (int)(d >> 6) & 0xf;
     prm[5] = (int)(d >> 1) & 0x1f;
   } else
   if (frame_size == 10)
   {
     prm[1] = 1; //voice frame
#if 1
     unsigned int d0 = (bits[0] << 24) | (bits[1] << 16) | (bits[2] << 8) | (bits[3] << 0);
     unsigned int d1 = (bits[4] << 24) | (bits[5] << 16) | (bits[6] << 8) | (bits[7] << 0);
     unsigned int d2 = (bits[8] <<  8) | (bits[9] <<  0);

     /* MA + 1st stage */
     prm[2] = (d0 >> 24); //7+1
     /* 2nd stage */
     prm[3] = (d0 >> 14) & 0x3ff; //5*2

     /* first subframe  */
     prm[4] = (d0 >>  6) & 0xff; //8 
     prm[5] = (d0 >>  5) & 1; //1
     prm[6] = ((d0 & 0x1f) << 8) | (d1 >> 24); //13
     prm[7] = (d1 >> 20) & 0xf; //4
     prm[8] = (d1 >> 13) & 0x7f; //4+3

     /* second subframe */
     prm[9] = (d1 >>  8) & 0x1f; //5
     prm[10] = (d2 >> 11) | ((d1 & 0xff) << 5); //13
     prm[11] = (d2 >>  7) & 0xf; //4
     prm[12] = (d2 >>  0) & 0x7f; //4+3
#else
     prm[2] = (int)(bits[0]);
     prm[3] = ((int)bits[1]) << 2;
     prm[3] |= (int)(bits[2] >> 6);

     prm[4] = ((int)(bits[2] & 0x3f)) << 2;
     prm[4] |= (int)(bits[3] >> 6);
     prm[5] = (int)((bits[3] & 0x20) >> 5);
     prm[6] = (int)(bits[3] & 0x1f) << 8;
     prm[6] |= (int)bits[4];
     prm[7] = (int)(bits[5] >> 4);
     prm[8] = (int)(bits[5] & 0x0f) << 3;
     prm[8] |= (int)(bits[6] >> 5);

     prm[9] = (int)(bits[6] & 0x1f);
     prm[10] = (int)bits[7] << 5;
     prm[10] |= (int)(bits[8] >> 3);
     prm[11] = ((int)bits[8] & 0x07) << 1;
     prm[11] |= (int)(bits[9] >> 7);
     prm[12] = (int)bits[9] & 0x7f;
#endif
     prm[5]  = check_parity_pitch(prm[4], prm[5]);
   }
   return;
}
