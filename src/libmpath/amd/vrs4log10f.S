#/* ============================================================
#Copyright (c) 2006 Advanced Micro Devices, Inc.
#
#All rights reserved.
#
#Redistribution and  use in source and binary  forms, with or
#without  modification,  are   permitted  provided  that  the
#following conditions are met:
#
#
#+ Redistributions  of source  code  must  retain  the  above
#  copyright  notice,   this  list  of   conditions  and  the
#  following disclaimer.
#
#+ Redistributions  in binary  form must reproduce  the above
#  copyright  notice,   this  list  of   conditions  and  the
#  following  disclaimer in  the  documentation and/or  other
#  materials provided with the distribution.
#
#+ Neither the  name of Advanced Micro Devices,  Inc. nor the
#  names  of  its contributors  may  be  used  to endorse  or
#  promote  products  derived   from  this  software  without
#  specific prior written permission.
#
#THIS  SOFTWARE  IS PROVIDED  BY  THE  COPYRIGHT HOLDERS  AND
#CONTRIBUTORS "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,
#INCLUDING,  BUT NOT  LIMITED TO,  THE IMPLIED  WARRANTIES OF
#MERCHANTABILITY  AND FITNESS  FOR A  PARTICULAR  PURPOSE ARE
#DISCLAIMED.  IN  NO  EVENT  SHALL  ADVANCED  MICRO  DEVICES,
#INC.  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT, INDIRECT,
#INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL  DAMAGES
#(INCLUDING,  BUT NOT LIMITED  TO, PROCUREMENT  OF SUBSTITUTE
#GOODS  OR  SERVICES;  LOSS  OF  USE, DATA,  OR  PROFITS;  OR
#BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON  ANY THEORY OF
#LIABILITY,  WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
#(INCLUDING NEGLIGENCE  OR OTHERWISE) ARISING IN  ANY WAY OUT
#OF  THE  USE  OF  THIS  SOFTWARE, EVEN  IF  ADVISED  OF  THE
#POSSIBILITY OF SUCH DAMAGE.
#
#In  the  redistribution and use of this software, each party
#shall  at  all times comply with all applicable governmental
#laws,  statutes, ordinances, rules, regulations, orders, and
#other   requirements,   including  without  limitation  such
#governmental   requirements   applicable   to  environmental
#protection,  health,  safety, wages, hours, equal employment
#opportunity,  nondiscrimination,  working conditions, import
#or export control, and transportation.  Without limiting the
#foregoing,  each  party  shall  adhere  to  the  U.S. Export
#Administration   Regulations  (EAR),  currently   found   at
#15 C.F.R. Sections 730  through  744,  and,  unless properly
#authorized  by  the  U.S. Government,  shall not (1) export,
#re-export  or  release  restricted  technology, software, or
#source  code  to  a  national of a country in Country Groups
#D:1 or E:1,  or  (2) export to Country Groups D:1 or E:1 the
#direct  product  of  such  technology  or  software, if such
#foreign  produced  direct  product  is  subject  to national
#security controls as identified on the Commerce Control List
#(currently  found  in  Supplement 1  to Section 774 of EAR).
#These  export  requirements  shall survive any expiration or
#termination of this agreement.
#============================================================ */

#
# vrs4logf.s
#
# A vector implementation of the logf libm function.
#  This routine implemented in single precision.  It is slightly
#  less accurate than the double precision version, but it will
#  be better for vectorizing.
#
# Prototype:
#
#     __m128 __vrs4_log10f(__m128 x);
#
#   Computes the natural log of x.
#   Returns proper C99 values, but may not raise status flags properly.
#   Less than 1 ulp of error.  
#
#
#ifdef __ELF__
.section .note.GNU-stack,"",@progbits
#endif

        .text
        .align 16
        .p2align 4,,15

# define local variable storage offsets
.equ	p_x,0			# save x
.equ	p_idx,0x010		# xmmword index
.equ	p_z1,0x020		# xmmword index
.equ	p_q,0x030		# xmmword index
.equ	p_corr,0x040		# xmmword index
.equ	p_omask,0x050		# xmmword index
.equ	save_xmm6,0x060		#
.equ	save_rbx,0x070		#

.equ	stack_size,0x088



.globl __vrs4_log10f
    .type   __vrs4_log10f,@function
__vrs4_log10f:
	sub		$stack_size,%rsp
	mov		%rbx,save_rbx(%rsp)	# save rbx

# check e as a special case
	movdqa	%xmm0,p_x(%rsp)	# save x
#	movdqa	%xmm0,%xmm2
#	cmpps	$0,.L__real_ef(%rip),%xmm2
#	movmskps	%xmm2,%r9d

#
# compute the index into the log tables
#
	movdqa	%xmm0,%xmm3
	movaps	%xmm0,%xmm1
	psrld	$23,%xmm3
	subps	.L__real_one(%rip),%xmm1
	psubd	.L__mask_127(%rip),%xmm3
	cvtdq2ps	%xmm3,%xmm6			# xexp


	movdqa	%xmm0,%xmm3
	pand	.L__real_mant(%rip),%xmm3
	xor		%r8,%r8
	movdqa	%xmm3,%xmm2
	movaps	.L__real_half(%rip),%xmm5							# .5

#/* Now  x = 2**xexp  * f,  1/2 <= f < 1. */
	psrld	$16,%xmm3
	lea		.L__np_ln_lead_table(%rip),%rdx
	movdqa	%xmm3,%xmm4
	psrld	$1,%xmm3
	paddd	.L__mask_040(%rip),%xmm3
	pand	.L__mask_001(%rip),%xmm4
	paddd	%xmm4,%xmm3
	cvtdq2ps	%xmm3,%xmm1
	packssdw	%xmm3,%xmm3
	movq	%xmm3,p_idx(%rsp)


# reduce and get u
	movdqa	%xmm0,%xmm3
	orps		.L__real_half(%rip),%xmm2


	mulps	.L__real_3c000000(%rip),%xmm1				# f1 = index/128


	subps	%xmm1,%xmm2											# f2 = f - f1
	mulps	%xmm2,%xmm5
	addps	%xmm5,%xmm1

	divps	%xmm1,%xmm2				# u

	mov		p_idx(%rsp),%rcx 			# get the indexes
	mov		%cx,%r8w
	ror		$16,%rcx
	mov		-256(%rdx,%r8,4),%eax		# get the f1 value

	mov		%cx,%r8w
	ror		$16,%rcx
	mov		-256(%rdx,%r8,4),%ebx		# get the f1 value
	shl		$32,%rbx
	or		%rbx,%rax
	mov		 %rax,p_z1(%rsp) 			# save the f1 values		

	mov		%cx,%r8w
	ror		$16,%rcx
	mov		-256(%rdx,%r8,4),%eax		# get the f1 value

	mov		%cx,%r8w
	ror		$16,%rcx
	or		-256(%rdx,%r8,4),%ebx		# get the f1 value
	shl		$32,%rbx
	or		%rbx,%rax
	mov		 %rax,p_z1+8(%rsp) 			# save the f1 value		

# solve for ln(1+u)
	movaps	%xmm2,%xmm1				# u
	mulps	%xmm2,%xmm2				# u^2
	movaps	%xmm2,%xmm5
	lea		.L__np_ln_tail_table(%rip),%rdx
	movaps	.L__real_cb3(%rip),%xmm3
	mulps	%xmm2,%xmm3				#Cu2
	mulps	%xmm1,%xmm5				# u^3
	addps	.L__real_cb2(%rip),%xmm3 #B+Cu2
	movaps	%xmm2,%xmm4
	mulps	%xmm5,%xmm4				# u^5
	movaps	.L__real_log2_lead(%rip),%xmm2

	mulps	.L__real_cb1(%rip),%xmm5 #Au3
	addps	%xmm5,%xmm1				# u+Au3
	mulps	%xmm3,%xmm4				# u5(B+Cu2)

	addps	%xmm4,%xmm1				# poly

# recombine		
	mov		%cx,%r8w
	shr		$16,%rcx
	mov		-256(%rdx,%r8,4),%eax		# get the f1 value

	mov		%cx,%r8w
	shr		$16,%rcx
	or		-256(%rdx,%r8,4),%ebx		# get the f1 value
	shl		$32,%rbx
	or		%rbx,%rax
	mov		 %rax,p_q(%rsp) 			# save the f1 value		

	mov		%cx,%r8w
	shr		$16,%rcx
	mov		-256(%rdx,%r8,4),%eax		# get the f1 value

	mov		%cx,%r8w
	mov		-256(%rdx,%r8,4),%ebx		# get the f1 value
	shl		$32,%rbx
	or		%rbx,%rax
	mov		 %rax,p_q+8(%rsp) 			# save the f1 value		

	addps	p_q(%rsp),%xmm1 #z2	+=q

	movaps	p_z1(%rsp),%xmm0			# z1  values

	mulps	%xmm6,%xmm2
	addps	%xmm2,%xmm0				#r1
	movaps	%xmm0,%xmm2
	mulps	.L__real_log2_tail(%rip),%xmm6
	addps	%xmm6,%xmm1				#r2
	movaps	%xmm1,%xmm3

#	logef to log10f
	mulps 	.L__real_log10e_tail(%rip),%xmm1	
	mulps 	.L__real_log10e_tail(%rip),%xmm0
	mulps 	.L__real_log10e_lead(%rip),%xmm3
	mulps 	.L__real_log10e_lead(%rip),%xmm2
	addps 	%xmm1,%xmm0
	addps 	%xmm3,%xmm0
	addps	%xmm2,%xmm0	
#	addps	%xmm1,%xmm0

# check for e
#	test		$0x0f,%r9d
#	jnz			.L__vlogf_e
.L__f1:

# check for negative numbers or zero
	xorps	%xmm1,%xmm1
	cmpps	$1,p_x(%rsp),%xmm1	# 0 greater than =?. catches NaNs also.
	movmskps	%xmm1,%r9d
	cmp		$0x0f,%r9d
	jnz		.L__z_or_neg

.L__f2:
##  if +inf
	movaps	p_x(%rsp),%xmm3 
	cmpps	$0,.L__real_inf(%rip),%xmm3
	movmskps	%xmm3,%r9d
	test		$0x0f,%r9d
	jnz		.L__log_inf
.L__f3:

	movaps	p_x(%rsp),%xmm3 
	subps	.L__real_one(%rip),%xmm3
	andps	.L__real_notsign(%rip),%xmm3
	cmpps	$2,.L__real_threshold(%rip),%xmm3
	movmskps	%xmm3,%r9d
	test	$0x0f,%r9d
	jnz		.L__near_one


.L__finish:
	mov		save_rbx(%rsp),%rbx		# restore rbx
	add		$stack_size,%rsp
	ret

.L__vlogf_e:
	movdqa	p_x(%rsp),%xmm2
	cmpps	$0,.L__real_ef(%rip),%xmm2
	movdqa	%xmm2,%xmm3
	andnps	%xmm0,%xmm3							# keep the non-e values
	andps	.L__real_one(%rip),%xmm2			# setup the 1 values
	orps	%xmm3,%xmm2							# merge
	movdqa	%xmm2,%xmm0							# and replace
	jmp		.L__f1

	.align	16
.L__near_one:
# saves 10 cycles
#      r = x - 1.0;
	movdqa	%xmm3,p_omask(%rsp)	# save ones mask
	movaps	p_x(%rsp),%xmm3 
	movaps	.L__real_two(%rip),%xmm2
	subps	.L__real_one(%rip),%xmm3	   # r
#      u          = r / (2.0 + r);
	addps	%xmm3,%xmm2
	movaps	%xmm3,%xmm1
	divps	%xmm2,%xmm1		# u
	movaps	.L__real_ca4(%rip),%xmm4	  #D
	movaps	.L__real_ca3(%rip),%xmm5	  #C
#      correction = r * u;
	movaps	%xmm3,%xmm6
	mulps	%xmm1,%xmm6		# correction
	movdqa	%xmm6,p_corr(%rsp)	# save correction
#      u          = u + u;
	addps	%xmm1,%xmm1		#u
	movaps	%xmm1,%xmm2
	mulps	%xmm2,%xmm2		#v =u^2
#      r2 = (u * v * (ca_1 + v * (ca_2 + v * (ca_3 + v * ca_4))) - correction);
	mulps	%xmm1,%xmm5		# Cu
	movaps	%xmm1,%xmm6
	mulps	%xmm2,%xmm6		# u^3
	mulps	.L__real_ca2(%rip),%xmm2	#Bu^2
	mulps	%xmm6,%xmm4		#Du^3

	addps	.L__real_ca1(%rip),%xmm2	# +A
	movaps	%xmm6,%xmm1
	mulps	%xmm1,%xmm1		# u^6
	addps	%xmm4,%xmm5		#Cu+Du3

	mulps	%xmm6,%xmm2		#u3(A+Bu2)
	mulps	%xmm5,%xmm1		#u6(Cu+Du3)
	addps	%xmm1,%xmm2
	subps	p_corr(%rsp),%xmm2		# -correction

	movaps  %xmm3,%xmm5 	#r1=r
	pand 	.L__mask_lower(%rip),%xmm5
	subps	%xmm5,%xmm3
	addps	%xmm3,%xmm2	#r2 = r2 + (r-r1)
	
	movaps	%xmm5,%xmm3
	movaps	%xmm2,%xmm1
	
	mulps 	.L__real_log10e_tail(%rip),%xmm2	
	mulps 	.L__real_log10e_tail(%rip),%xmm3
	mulps 	.L__real_log10e_lead(%rip),%xmm1
	mulps 	.L__real_log10e_lead(%rip),%xmm5
	addps 	%xmm2,%xmm3
	addps 	%xmm1,%xmm3
	addps	%xmm5,%xmm3		
#      return r + r2;
#	addps	%xmm2,%xmm3

	movdqa	p_omask(%rsp),%xmm6
	movdqa	%xmm6,%xmm2
	andnps	%xmm0,%xmm6					# keep the non-nearone values
	andps	%xmm3,%xmm2					# setup the nearone values
	orps	%xmm6,%xmm2					# merge
	movdqa	%xmm2,%xmm0					# and replace

	jmp		.L__finish

# we have a zero, a negative number, or both.
# the mask is already in .LNaNs,%xmm1 are also picked up here, along with -inf.
.L__z_or_neg:
# deal with negatives first
	movdqa	%xmm1,%xmm3
	andps	%xmm0,%xmm3							# keep the non-error values
	andnps	.L__real_nan(%rip),%xmm1			# setup the nan values
	orps	%xmm3,%xmm1							# merge
	movdqa	%xmm1,%xmm0							# and replace
# check for +/- 0
	xorps	%xmm1,%xmm1
	cmpps	$0,p_x(%rsp),%xmm1	# 0 ?.
	movmskps	%xmm1,%r9d
	test		$0x0f,%r9d
	jz		.L__zn2

	movdqa	%xmm1,%xmm3
	andnps	%xmm0,%xmm3							# keep the non-error values
	andps	.L__real_ninf(%rip),%xmm1		# ; C99 specs -inf for +-0
	orps	%xmm3,%xmm1							# merge
	movdqa	%xmm1,%xmm0							# and replace

.L__zn2:
# check for NaNs
	movaps	p_x(%rsp),%xmm3 
	andps	.L__real_inf(%rip),%xmm3
	cmpps	$0,.L__real_inf(%rip),%xmm3		# mask for max exponent

	movdqa	p_x(%rsp),%xmm4 
	pand	.L__real_mant(%rip),%xmm4		# mask for non-zero mantissa
	pcmpeqd	.L__real_zero(%rip),%xmm4
	pandn	%xmm3,%xmm4							# mask for NaNs
	movdqa	%xmm4,%xmm2
	movdqa	p_x(%rsp),%xmm1			# isolate the NaNs
	pand	%xmm4,%xmm1

	pand	.L__real_qnanbit(%rip),%xmm4		# now we have a mask that will set QNaN bit
	por		%xmm1,%xmm4							# turn SNaNs to QNaNs

	movdqa	%xmm2,%xmm1
	andnps	%xmm0,%xmm2							# keep the non-error values
	orps	%xmm4,%xmm2							# merge
	movdqa	%xmm2,%xmm0							# and replace
	xorps	%xmm4,%xmm4

	jmp		.L__f2

# handle only +inf	 log(+inf) = inf
.L__log_inf:
	movdqa	%xmm3,%xmm1
	andnps	%xmm0,%xmm3							# keep the non-error values
	andps	p_x(%rsp),%xmm1			# setup the +inf values
	orps	%xmm3,%xmm1							# merge
	movdqa	%xmm1,%xmm0							# and replace
	jmp		.L__f3


        .data
        .align 64

.L__real_zero:				.quad 0x00000000000000000	# 1.0
					.quad 0x00000000000000000
.L__real_one:				.quad 0x03f8000003f800000	# 1.0
					.quad 0x03f8000003f800000
.L__real_two:				.quad 0x04000000040000000	# 1.0
					.quad 0x04000000040000000
.L__real_ninf:				.quad 0x0ff800000ff800000	# -inf
					.quad 0x0ff800000ff800000
.L__real_inf:				.quad 0x07f8000007f800000	# +inf
					.quad 0x07f8000007f800000
.L__real_nan:				.quad 0x07fc000007fc00000	# NaN
					.quad 0x07fc000007fc00000
.L__real_ef:				.quad 0x0402DF854402DF854	# float e
					.quad 0x0402DF854402DF854

.L__real_sign:				.quad 0x08000000080000000	# sign bit
					.quad 0x08000000080000000
.L__real_notsign:			.quad 0x07ffFFFFF7ffFFFFF	# ^sign bit
					.quad 0x07ffFFFFF7ffFFFFF
.L__real_qnanbit:			.quad 0x00040000000400000	# quiet nan bit
					.quad 0x00040000000400000
.L__real_mant:				.quad 0x0007FFFFF007FFFFF	# mantipsa bits
					.quad 0x0007FFFFF007FFFFF
.L__real_3c000000:			.quad 0x03c0000003c000000	# /* 0.0078125 = 1/128 */
					.quad 0x03c0000003c000000
.L__mask_127:				.quad 0x00000007f0000007f	# 
					.quad 0x00000007f0000007f
.L__mask_040:				.quad 0x00000004000000040	# 
					.quad 0x00000004000000040
.L__mask_001:				.quad 0x00000000100000001	# 
					.quad 0x00000000100000001


.L__real_threshold:			.quad 0x03CF5C28F3CF5C28F	# .03
					.quad 0x03CF5C28F3CF5C28F

.L__real_ca1:				.quad 0x03DAAAAAB3DAAAAAB	# 8.33333333333317923934e-02
					.quad 0x03DAAAAAB3DAAAAAB
.L__real_ca2:				.quad 0x03C4CCCCD3C4CCCCD	# 1.25000000037717509602e-02
					.quad 0x03C4CCCCD3C4CCCCD
.L__real_ca3:				.quad 0x03B1249183B124918	# 2.23213998791944806202e-03
					.quad 0x03B1249183B124918
.L__real_ca4:				.quad 0x039E401A639E401A6	# 4.34887777707614552256e-04
					.quad 0x039E401A639E401A6
.L__real_cb1:				.quad 0x03DAAAAAB3DAAAAAB	# 8.33333333333333593622e-02
					.quad 0x03DAAAAAB3DAAAAAB
.L__real_cb2:				.quad 0x03C4CCCCD3C4CCCCD	# 1.24999999978138668903e-02
					.quad 0x03C4CCCCD3C4CCCCD
.L__real_cb3:				.quad 0x03B124A123B124A12	# 2.23219810758559851206e-03
					.quad 0x03B124A123B124A12
.L__real_log2_lead:        		.quad 0x03F3170003F317000  # 0.693115234375
                        		.quad 0x03F3170003F317000
.L__real_log2_tail:        		.quad 0x03805FDF43805FDF4  # 0.000031946183
                        		.quad 0x03805FDF43805FDF4
.L__real_half:				.quad 0x03f0000003f000000	# 1/2
					.quad 0x03f0000003f000000
.L__real_log10e_lead:	     .quad 0x03EDE00003EDE0000	# log10e_lead  0.4335937500
                       .quad 0x03EDE00003EDE0000
.L__real_log10e_tail:	      .quad 0x03A37B1523A37B152  # log10e_tail  0.0007007319	
                       .quad 0x03A37B1523A37B152


.L__mask_lower:				.quad 0x0ffff0000ffff0000	#
					.quad 0x0ffff0000ffff0000

.L__np_ln__table:
	.quad	0x0000000000000000 		# 0.00000000000000000000e+00
	.quad	0x3F8FC0A8B0FC03E4		# 1.55041813850402832031e-02
	.quad	0x3F9F829B0E783300		# 3.07716131210327148438e-02
	.quad	0x3FA77458F632DCFC		# 4.58095073699951171875e-02
	.quad	0x3FAF0A30C01162A6		# 6.06245994567871093750e-02
	.quad	0x3FB341D7961BD1D1		# 7.52233862876892089844e-02
	.quad	0x3FB6F0D28AE56B4C		# 8.96121263504028320312e-02
	.quad	0x3FBA926D3A4AD563		# 1.03796780109405517578e-01
	.quad	0x3FBE27076E2AF2E6		# 1.17783010005950927734e-01
	.quad	0x3FC0D77E7CD08E59		# 1.31576299667358398438e-01
	.quad	0x3FC29552F81FF523		# 1.45181953907012939453e-01
	.quad	0x3FC44D2B6CCB7D1E		# 1.58604979515075683594e-01
	.quad	0x3FC5FF3070A793D4		# 1.71850204467773437500e-01
	.quad	0x3FC7AB890210D909		# 1.84922337532043457031e-01
	.quad	0x3FC9525A9CF456B4		# 1.97825729846954345703e-01
	.quad	0x3FCAF3C94E80BFF3		# 2.10564732551574707031e-01
	.quad	0x3FCC8FF7C79A9A22		# 2.23143517971038818359e-01
	.quad	0x3FCE27076E2AF2E6		# 2.35566020011901855469e-01
	.quad	0x3FCFB9186D5E3E2B		# 2.47836112976074218750e-01
	.quad	0x3FD0A324E27390E3		# 2.59957492351531982422e-01
	.quad	0x3FD1675CABABA60E		# 2.71933674812316894531e-01
	.quad	0x3FD22941FBCF7966		# 2.83768117427825927734e-01
	.quad	0x3FD2E8E2BAE11D31		# 2.95464158058166503906e-01
	.quad	0x3FD3A64C556945EA		# 3.07025015354156494141e-01
	.quad	0x3FD4618BC21C5EC2		# 3.18453729152679443359e-01
	.quad	0x3FD51AAD872DF82D		# 3.29753279685974121094e-01
	.quad	0x3FD5D1BDBF5809CA		# 3.40926527976989746094e-01
	.quad	0x3FD686C81E9B14AF		# 3.51976394653320312500e-01
	.quad	0x3FD739D7F6BBD007		# 3.62905442714691162109e-01
	.quad	0x3FD7EAF83B82AFC3		# 3.73716354370117187500e-01
	.quad	0x3FD89A3386C1425B		# 3.84411692619323730469e-01
	.quad	0x3FD947941C2116FB		# 3.94993782043457031250e-01
	.quad	0x3FD9F323ECBF984C		# 4.05465066432952880859e-01
	.quad	0x3FDA9CEC9A9A084A		# 4.15827870368957519531e-01
	.quad	0x3FDB44F77BCC8F63		# 4.26084339618682861328e-01
	.quad	0x3FDBEB4D9DA71B7C		# 4.36236739158630371094e-01
	.quad	0x3FDC8FF7C79A9A22		# 4.46287095546722412109e-01
	.quad	0x3FDD32FE7E00EBD5		# 4.56237375736236572266e-01
	.quad	0x3FDDD46A04C1C4A1		# 4.66089725494384765625e-01
	.quad	0x3FDE744261D68788		# 4.75845873355865478516e-01
	.quad	0x3FDF128F5FAF06ED		# 4.85507786273956298828e-01
	.quad	0x3FDFAF588F78F31F		# 4.95077252388000488281e-01
	.quad	0x3FE02552A5A5D0FF		# 5.04556000232696533203e-01
	.quad	0x3FE0723E5C1CDF40		# 5.13945698738098144531e-01
	.quad	0x3FE0BE72E4252A83		# 5.23248136043548583984e-01
	.quad	0x3FE109F39E2D4C97		# 5.32464742660522460938e-01
	.quad	0x3FE154C3D2F4D5EA		# 5.41597247123718261719e-01
	.quad	0x3FE19EE6B467C96F		# 5.50647079944610595703e-01
	.quad	0x3FE1E85F5E7040D0		# 5.59615731239318847656e-01
	.quad	0x3FE23130D7BEBF43		# 5.68504691123962402344e-01
	.quad	0x3FE2795E1289B11B		# 5.77315330505371093750e-01
	.quad	0x3FE2C0E9ED448E8C		# 5.86049020290374755859e-01
	.quad	0x3FE307D7334F10BE		# 5.94707071781158447266e-01
	.quad	0x3FE34E289D9CE1D3		# 6.03290796279907226562e-01
	.quad	0x3FE393E0D3562A1A		# 6.11801505088806152344e-01
	.quad	0x3FE3D9026A7156FB		# 6.20240390300750732422e-01
	.quad	0x3FE41D8FE84672AE		# 6.28608644008636474609e-01
	.quad	0x3FE4618BC21C5EC2		# 6.36907458305358886719e-01
	.quad	0x3FE4A4F85DB03EBB		# 6.45137906074523925781e-01
	.quad	0x3FE4E7D811B75BB1		# 6.53301239013671875000e-01
	.quad	0x3FE52A2D265BC5AB		# 6.61398470401763916016e-01
	.quad	0x3FE56BF9D5B3F399		# 6.69430613517761230469e-01
	.quad	0x3FE5AD404C359F2D		# 6.77398800849914550781e-01
	.quad	0x3FE5EE02A9241675		# 6.85303986072540283203e-01
	.quad	0x3FE62E42FEFA39EF		# 6.93147122859954833984e-01
	.quad 0					# for alignment

.L__np_ln_lead_table:
    .long 0x00000000  # 0.000000000000 0
    .long 0x3C7E0000  # 0.015502929688 1
    .long 0x3CFC1000  # 0.030769348145 2
    .long 0x3D3BA000  # 0.045806884766 3
    .long 0x3D785000  # 0.060623168945 4
    .long 0x3D9A0000  # 0.075195312500 5
    .long 0x3DB78000  # 0.089599609375 6
    .long 0x3DD49000  # 0.103790283203 7
    .long 0x3DF13000  # 0.117767333984 8
    .long 0x3E06B000  # 0.131530761719 9
    .long 0x3E14A000  # 0.145141601563 10
    .long 0x3E226000  # 0.158569335938 11
    .long 0x3E2FF000  # 0.171813964844 12
    .long 0x3E3D5000  # 0.184875488281 13
    .long 0x3E4A9000  # 0.197814941406 14
    .long 0x3E579000  # 0.210510253906 15
    .long 0x3E647000  # 0.223083496094 16
    .long 0x3E713000  # 0.235534667969 17
    .long 0x3E7DC000  # 0.247802734375 18
    .long 0x3E851000  # 0.259887695313 19
    .long 0x3E8B3000  # 0.271850585938 20
    .long 0x3E914000  # 0.283691406250 21
    .long 0x3E974000  # 0.295410156250 22
    .long 0x3E9D3000  # 0.307006835938 23
    .long 0x3EA30000  # 0.318359375000 24
    .long 0x3EA8D000  # 0.329711914063 25
    .long 0x3EAE8000  # 0.340820312500 26
    .long 0x3EB43000  # 0.351928710938 27
    .long 0x3EB9C000  # 0.362792968750 28
    .long 0x3EBF5000  # 0.373657226563 29
    .long 0x3EC4D000  # 0.384399414063 30
    .long 0x3ECA3000  # 0.394897460938 31
    .long 0x3ECF9000  # 0.405395507813 32
    .long 0x3ED4E000  # 0.415771484375 33
    .long 0x3EDA2000  # 0.426025390625 34
    .long 0x3EDF5000  # 0.436157226563 35
    .long 0x3EE47000  # 0.446166992188 36
    .long 0x3EE99000  # 0.456176757813 37
    .long 0x3EEEA000  # 0.466064453125 38
    .long 0x3EF3A000  # 0.475830078125 39
    .long 0x3EF89000  # 0.485473632813 40
    .long 0x3EFD7000  # 0.494995117188 41
    .long 0x3F012000  # 0.504394531250 42
    .long 0x3F039000  # 0.513916015625 43
    .long 0x3F05F000  # 0.523193359375 44
    .long 0x3F084000  # 0.532226562500 45
    .long 0x3F0AA000  # 0.541503906250 46
    .long 0x3F0CF000  # 0.550537109375 47
    .long 0x3F0F4000  # 0.559570312500 48
    .long 0x3F118000  # 0.568359375000 49
    .long 0x3F13C000  # 0.577148437500 50
    .long 0x3F160000  # 0.585937500000 51
    .long 0x3F183000  # 0.594482421875 52
    .long 0x3F1A7000  # 0.603271484375 53
    .long 0x3F1C9000  # 0.611572265625 54
    .long 0x3F1EC000  # 0.620117187500 55
    .long 0x3F20E000  # 0.628417968750 56
    .long 0x3F230000  # 0.636718750000 57
    .long 0x3F252000  # 0.645019531250 58
    .long 0x3F273000  # 0.653076171875 59
    .long 0x3F295000  # 0.661376953125 60
    .long 0x3F2B5000  # 0.669189453125 61
    .long 0x3F2D6000  # 0.677246093750 62
    .long 0x3F2F7000  # 0.685302734375 63
    .long 0x3F317000  # 0.693115234375 64
    .long 0					# for alignment

.L__np_ln_tail_table:
    .long 0x00000000  # 0.000000000000 0
    .long 0x35A8B0FC  # 0.000001256848 1
    .long 0x361B0E78  # 0.000002310522 2
    .long 0x3631EC66  # 0.000002651266 3
    .long 0x35C30046  # 0.000001452871 4
    .long 0x37EBCB0E  # 0.000028108738 5
    .long 0x37528AE5  # 0.000012549314 6
    .long 0x36DA7496  # 0.000006510479 7
    .long 0x3783B715  # 0.000015701671 8
    .long 0x383F3E68  # 0.000045596069 9
    .long 0x38297C10  # 0.000040408282 10
    .long 0x3815B666  # 0.000035694240 11
    .long 0x38183854  # 0.000036292084 12
    .long 0x38448108  # 0.000046850211 13
    .long 0x373539E9  # 0.000010801924 14
    .long 0x3864A740  # 0.000054515200 15
    .long 0x387BE3CD  # 0.000060055219 16
    .long 0x3803B715  # 0.000031403342 17
    .long 0x380C36AF  # 0.000033429529 18
    .long 0x3892713A  # 0.000069829126 19
    .long 0x38AE55D6  # 0.000083129547 20
    .long 0x38A0FDE8  # 0.000076766883 21
    .long 0x3862BAE1  # 0.000054056643 22
    .long 0x3798AAD3  # 0.000018199358 23
    .long 0x38C5E10E  # 0.000094356117 24
    .long 0x382D872E  # 0.000041372310 25
    .long 0x38DEDFAC  # 0.000106274470 26
    .long 0x38481E9B  # 0.000047712219 27
    .long 0x38EBFB5E  # 0.000112524940 28
    .long 0x38783B83  # 0.000059183232 29
    .long 0x374E1B05  # 0.000012284848 30
    .long 0x38CA0E11  # 0.000096347307 31
    .long 0x3891F660  # 0.000069600297 32
    .long 0x386C9A9A  # 0.000056410769 33
    .long 0x38777BCD  # 0.000059004688 34
    .long 0x38A6CED4  # 0.000079540216 35
    .long 0x38FBE3CD  # 0.000120110439 36
    .long 0x387E7E01  # 0.000060675669 37
    .long 0x37D40984  # 0.000025276800 38
    .long 0x3784C3AD  # 0.000015826745 39
    .long 0x380F5FAF  # 0.000034182969 40
    .long 0x38AC47BC  # 0.000082149607 41
    .long 0x392952D3  # 0.000161479504 42
    .long 0x37F97073  # 0.000029735476 43
    .long 0x3865C84A  # 0.000054784388 44
    .long 0x3979CF17  # 0.000238236375 45
    .long 0x38C3D2F5  # 0.000093376184 46
    .long 0x38E6B468  # 0.000110008579 47
    .long 0x383EBCE1  # 0.000045475437 48
    .long 0x39186BDF  # 0.000145360347 49
    .long 0x392F0945  # 0.000166927537 50
    .long 0x38E9ED45  # 0.000111545007 51
    .long 0x396B99A8  # 0.000224685878 52
    .long 0x37A27674  # 0.000019367064 53
    .long 0x397069AB  # 0.000229275480 54
    .long 0x39013539  # 0.000123222257 55
    .long 0x3947F423  # 0.000190690669 56
    .long 0x3945E10E  # 0.000188712234 57
    .long 0x38F85DB0  # 0.000118430122 58
    .long 0x396C08DC  # 0.000225100142 59
    .long 0x37B4996F  # 0.000021529120 60
    .long 0x397CEADA  # 0.000241200818 61
    .long 0x3920261B  # 0.000152729845 62
    .long 0x35AA4906  # 0.000001268724 63
    .long 0x3805FDF4  # 0.000031946183 64
    .long 0					# for alignment


#if (defined(__FreeBSD__) || defined(__linux__)) && defined(__ELF__)
    .section .note.GNU-stack,"",@progbits
#endif
