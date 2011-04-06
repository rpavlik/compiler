/* ============================================================
Copyright (c) 2006 Advanced Micro Devices, Inc.

All rights reserved.

Redistribution and  use in source and binary  forms, with or
without  modification,  are   permitted  provided  that  the
following conditions are met:


+ Redistributions  of source  code  must  retain  the  above
  copyright  notice,   this  list  of   conditions  and  the
  following disclaimer.

+ Redistributions  in binary  form must reproduce  the above
  copyright  notice,   this  list  of   conditions  and  the
  following  disclaimer in  the  documentation and/or  other
  materials provided with the distribution.

+ Neither the  name of Advanced Micro Devices,  Inc. nor the
  names  of  its contributors  may  be  used  to endorse  or
  promote  products  derived   from  this  software  without
  specific prior written permission.

THIS  SOFTWARE  IS PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND
CONTRIBUTORS "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING,  BUT NOT  LIMITED TO,  THE IMPLIED  WARRANTIES OF
MERCHANTABILITY  AND FITNESS  FOR A  PARTICULAR  PURPOSE ARE
DISCLAIMED.  IN  NO  EVENT  SHALL  ADVANCED  MICRO  DEVICES,
INC.  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT, INDIRECT,
INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES
(INCLUDING,  BUT NOT LIMITED  TO, PROCUREMENT  OF SUBSTITUTE
GOODS  OR  SERVICES;  LOSS  OF  USE, DATA,  OR  PROFITS;  OR
BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON  ANY THEORY OF
LIABILITY,  WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
(INCLUDING NEGLIGENCE  OR OTHERWISE) ARISING IN  ANY WAY OUT
OF  THE  USE  OF  THIS  SOFTWARE, EVEN  IF  ADVISED  OF  THE
POSSIBILITY OF SUCH DAMAGE.

In  the  redistribution and use of this software, each party
shall  at  all times comply with all applicable governmental
laws,  statutes, ordinances, rules, regulations, orders, and
other   requirements,   including  without  limitation  such
governmental   requirements   applicable   to  environmental
protection,  health,  safety, wages, hours, equal employment
opportunity,  nondiscrimination,  working conditions, import
or export control, and transportation.  Without limiting the
foregoing,  each  party  shall  adhere  to  the  U.S. Export
Administration   Regulations  (EAR),  currently   found   at
15 C.F.R. Sections 730  through  744,  and,  unless properly
authorized  by  the  U.S. Government,  shall not (1) export,
re-export  or  release  restricted  technology, software, or
source  code  to  a  national of a country in Country Groups
D:1 or E:1,  or  (2) export to Country Groups D:1 or E:1 the
direct  product  of  such  technology  or  software, if such
foreign  produced  direct  product  is  subject  to national
security controls as identified on the Commerce Control List
(currently  found  in  Supplement 1  to Section 774 of EAR).
These  export  requirements  shall survive any expiration or
termination of this agreement.
============================================================ */

#include "libm_amd.h"
#include "libm_util_amd.h"

/* Define this to get debugging print statements activated */
#define DEBUGGING_PRINT
#undef DEBUGGING_PRINT


#ifdef DEBUGGING_PRINT
#include <stdio.h>
char *d2b(int d, int bitsper, int point)
{
  static char buff[50];
  int i, j;
  j = bitsper;
  if (point >= 0 && point <= bitsper)
    j++;
  buff[j] = '\0';
  for (i = bitsper - 1; i >= 0; i--)
    {
      j--;
      if (d % 2 == 1)
        buff[j] = '1';
      else
        buff[j] = '0';
      if (i == point)
        {
          j--;
          buff[j] = '.';
        }
      d /= 2;
    }
  return buff;
}
#endif

/* Given positive argument x, reduce it to the range [-pi/4,pi/4] using
   extra precision, and return the result in r, rr.
   Return value "region" tells how many lots of pi/2 were subtracted
   from x to put it in the range [-pi/4,pi/4], mod 4. */
void __remainder_piby2(double x, double *r, double *rr, int *region)
{

      /* This method simulates multi-precision floating-point
         arithmetic and is accurate for all 1 <= x < infinity */
      static const double
        piby2_lead = 1.57079632679489655800e+00, /* 0x3ff921fb54442d18 */
        piby2_part1 = 1.57079631090164184570e+00, /* 0x3ff921fb50000000 */
        piby2_part2 = 1.58932547122958567343e-08, /* 0x3e5110b460000000 */
        piby2_part3 = 6.12323399573676480327e-17; /* 0x3c91a62633145c06 */
      const int bitsper = 10;
      unsigned long res[500];
      unsigned long ux, u, carry, mask, mant, highbitsrr;
      int first, last, i, rexp, xexp, resexp, ltb, determ;
      double xx, t;
      static unsigned long pibits[] =
      {
        0,    0,    0,    0,    0,    0,
        162,  998,   54,  915,  580,   84,  671,  777,  855,  839,
        851,  311,  448,  877,  553,  358,  316,  270,  260,  127,
        593,  398,  701,  942,  965,  390,  882,  283,  570,  265,
        221,  184,    6,  292,  750,  642,  465,  584,  463,  903,
        491,  114,  786,  617,  830,  930,   35,  381,  302,  749,
        72,  314,  412,  448,  619,  279,  894,  260,  921,  117,
        569,  525,  307,  637,  156,  529,  504,  751,  505,  160,
        945, 1022,  151, 1023,  480,  358,   15,  956,  753,   98,
        858,   41,  721,  987,  310,  507,  242,  498,  777,  733,
        244,  399,  870,  633,  510,  651,  373,  158,  940,  506,
        997,  965,  947,  833,  825,  990,  165,  164,  746,  431,
        949, 1004,  287,  565,  464,  533,  515,  193,  111,  798
      };

      GET_BITS_DP64(x, ux);

#ifdef DEBUGGING_PRINT
      printf("On entry, x = %25.20e = %s\n", x, double2hex(&x));
#endif

      xexp = (int)(((ux & EXPBITS_DP64) >> EXPSHIFTBITS_DP64) - EXPBIAS_DP64);
      ux = (ux & MANTBITS_DP64) | IMPBIT_DP64;

      /* Now ux is the mantissa bit pattern of x as a long integer */
      carry = 0;
      mask = 1;
      mask = (mask << bitsper) - 1;

      /* Set first and last to the positions of the first
         and last chunks of 2/pi that we need */
      first = xexp / bitsper;
      resexp = xexp - first * bitsper;
      /* 180 is the theoretical maximum number of bits (actually
         175 for IEEE double precision) that we need to extract
         from the middle of 2/pi to compute the reduced argument
         accurately enough for our purposes */
      last = first + 180 / bitsper;

      /* Do a long multiplication of the bits of 2/pi by the
         integer mantissa */
#if 0
      for (i = last; i >= first; i--)
        {
          u = pibits[i] * ux + carry;
          res[i - first] = u & mask;
          carry = u >> bitsper;
        }
      res[last - first + 1] = 0;
#else
      /* Unroll the loop. This is only correct because we know
         that bitsper is fixed as 10. */
      res[19] = 0;
      u = pibits[last] * ux;
      res[18] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-1] * ux + carry;
      res[17] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-2] * ux + carry;
      res[16] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-3] * ux + carry;
      res[15] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-4] * ux + carry;
      res[14] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-5] * ux + carry;
      res[13] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-6] * ux + carry;
      res[12] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-7] * ux + carry;
      res[11] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-8] * ux + carry;
      res[10] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-9] * ux + carry;
      res[9] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-10] * ux + carry;
      res[8] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-11] * ux + carry;
      res[7] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-12] * ux + carry;
      res[6] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-13] * ux + carry;
      res[5] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-14] * ux + carry;
      res[4] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-15] * ux + carry;
      res[3] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-16] * ux + carry;
      res[2] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-17] * ux + carry;
      res[1] = u & mask;
      carry = u >> bitsper;
      u = pibits[last-18] * ux + carry;
      res[0] = u & mask;
#endif

#ifdef DEBUGGING_PRINT
      printf("resexp = %d\n", resexp);
      printf("Significant part of x * 2/pi with binary"
             " point in correct place:\n");
      for (i = 0; i <= last - first; i++)
        {
          if (i > 0 && i % 5 == 0)
            printf("\n ");
          if (i == 1)
            printf("%s ", d2b((int)res[i], bitsper, resexp));
          else
            printf("%s ", d2b((int)res[i], bitsper, -1));
        }
      printf("\n");
#endif

      /* Reconstruct the result */
      ltb = (int)((((res[0] << bitsper) | res[1])
                   >> (bitsper - 1 - resexp)) & 7);

      /* determ says whether the fractional part is >= 0.5 */
      determ = ltb & 1;

#ifdef DEBUGGING_PRINT
      printf("ltb = %d (last two bits before binary point"
             " and first bit after)\n", ltb);
      printf("determ = %d (1 means need to negate because the fractional\n"
             "            part of x * 2/pi is greater than 0.5)\n", determ);
#endif

      i = 1;
      if (determ)
        {
          /* The mantissa is >= 0.5. We want to subtract it
             from 1.0 by negating all the bits */
          *region = ((ltb >> 1) + 1) & 3;
          mant = 1;
          mant = ~(res[1]) & ((mant << (bitsper - resexp)) - 1);
          while (mant < 0x0020000000000000)
            {
              i++;
              mant = (mant << bitsper) | (~(res[i]) & mask);
            }
          highbitsrr = ~(res[i + 1]) << (64 - bitsper);
        }
      else
        {
          *region = (ltb >> 1);
          mant = 1;
          mant = res[1] & ((mant << (bitsper - resexp)) - 1);
          while (mant < 0x0020000000000000)
            {
              i++;
              mant = (mant << bitsper) | res[i];
            }
          highbitsrr = res[i + 1] << (64 - bitsper);
        }

      rexp = 52 + resexp - i * bitsper;

      while (mant >= 0x0020000000000000)
        {
          rexp++;
          highbitsrr = (highbitsrr >> 1) | ((mant & 1) << 63);
          mant >>= 1;
        }

#ifdef DEBUGGING_PRINT
      printf("Normalised mantissa = 0x%016lx\n", mant);
      printf("High bits of rest of mantissa = 0x%016lx\n", highbitsrr);
      printf("Exponent to be inserted on mantissa = rexp = %d\n", rexp);
#endif

      /* Put the result exponent rexp onto the mantissa pattern */
      u = ((unsigned long)rexp + EXPBIAS_DP64) << EXPSHIFTBITS_DP64;
      ux = (mant & MANTBITS_DP64) | u;
      if (determ)
        /* If we negated the mantissa we negate x too */
        ux |= SIGNBIT_DP64;
      PUT_BITS_DP64(ux, x);

      /* Create the bit pattern for rr */
      highbitsrr >>= 12; /* Note this is shifted one place too far */
      u = ((unsigned long)rexp + EXPBIAS_DP64 - 53) << EXPSHIFTBITS_DP64;
      PUT_BITS_DP64(u, t);
      u |= highbitsrr;
      PUT_BITS_DP64(u, xx);

      /* Subtract the implicit bit we accidentally added */
      xx -= t;
      /* Set the correct sign, and double to account for the
         "one place too far" shift */
      if (determ)
        xx *= -2.0;
      else
        xx *= 2.0;

#ifdef DEBUGGING_PRINT
      printf("(lead part of x*2/pi) = %25.20e = %s\n", x, double2hex(&x));
      printf("(tail part of x*2/pi) = %25.20e = %s\n", xx, double2hex(&xx));
#endif

      /* (x,xx) is an extra-precise version of the fractional part of
         x * 2 / pi. Multiply (x,xx) by pi/2 in extra precision
         to get the reduced argument (r,rr). */
      {
        double hx, tx, c, cc;
        /* Split x into hx (head) and tx (tail) */
        GET_BITS_DP64(x, ux);
        ux &= 0xfffffffff8000000;
        PUT_BITS_DP64(ux, hx);
        tx = x - hx;

        c = piby2_lead * x;
        cc = ((((piby2_part1 * hx - c) + piby2_part1 * tx) +
               piby2_part2 * hx) + piby2_part2 * tx) +
          (piby2_lead * xx + piby2_part3 * x);
        *r = c + cc;
        *rr = (c - *r) + cc;
      }

#ifdef DEBUGGING_PRINT
      printf(" (r,rr) = lead and tail parts of frac(x*2/pi) * pi/2:\n");
      printf(" r = %25.20e = %s\n", *r, double2hex(r));
      printf("rr = %25.20e = %s\n", *rr, double2hex(rr));
      printf("region = (number of pi/2 subtracted from x) mod 4 = %d\n",
             *region);
#endif
  return;
}
