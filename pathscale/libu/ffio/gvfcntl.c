/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation version 2.1

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


#pragma ident "@(#) libu/ffio/gvfcntl.c	92.2	06/29/99 13:16:47"

#include <ffio.h>
#include "gvio.h"

/*
 * V class fcntl requests
 *
 * Parameters:
 *	fd	- file descriptor (dummy)
 *	cmd	- command code
 *	arg	- command specific parameter
 *	stat	- pointer to status return word
 */
_gen_vfcntl(
	struct fdinfo	*fio,
	int		cmd,
	void		*arg,
	struct ffsw	*stat)
	{
	struct fdinfo *llfio;
	struct ffc_info_s *ffcp, locinfo;
	int ret;
	struct gen_vf *vf_info;

	llfio = fio->fioptr;

	ret = 0;
	switch(cmd)
		{
		case FC_GETINFO:
			ffcp = (struct ffc_info_s *)arg;
			XRCALL(llfio,fcntlrtn)
				llfio, FC_GETINFO, &locinfo, stat);
			ffcp->ffc_flags = 
				FFC_REC |	/* records */
				FFC_WEOD |	/* can write EOD */

				FFC_RWND |	/* can rewind */

				FFC_VAR |	/* can do variable len recs */
				FFC_BINARY |	/* can do binary */
				FFC_CODED |	/* can do chars */

				FFC_SEQ |	/* can do seq */
				FFC_WRTRUNC |	/* Write implies trunc */
				0;
/*
 *			Can do WEOF IF lower layer can, or if format
 *			supports it directly.
 */
			switch(fio->rtype)
				{
				case TR_IBM_U:
				case TR_IBM_V:
				case TR_IBM_VB:
				case TR_IBM_VBS:
				case TR_VMS_V_DSK:
				case TR_VMS_V_TP:
				case TR_VMS_V_TR:
				case TR_NVE_D:
					/* Only if lower layer can */
					if ((locinfo.ffc_flags & FFC_WEOF) != 0)
						ffcp->ffc_flags |= FFC_WEOF;
					break;
				case TR_VMS_S_DSK:
				case TR_VMS_S_TP:
				case TR_NVE_S:
				case TR_VMS_S_TR:
					/* Support it directly */
					ffcp->ffc_flags |= FFC_WEOF;
					break;
				}
			/* can skip bad data only if lower layer can */
			if ((locinfo.ffc_flags & FFC_SKIPBAD) != 0)
				{
				switch(fio->rtype)
					{
					case TR_IBM_U:
					case TR_IBM_V:
					case TR_IBM_VB:
					case TR_IBM_VBS:
						ffcp->ffc_flags |= FFC_SKIPBAD;
						break;
					}
				}

			ffcp->ffc_gran = 8;	/* granularity is 8 bits */
			ffcp->ffc_reclen = 0;	/* no rec length */
			ffcp->ffc_fd = locinfo.ffc_fd; /* fd from lower layer */
			break;
		case FC_STAT:
		case FC_SETRECL:
		case FC_GETTP:
			ret = XRCALL(llfio,fcntlrtn) llfio, cmd, arg, stat);
			break;
		case FC_ASPOLL:
		case FC_RECALL:
			break;
		case FC_AUTOBAD:
			/* This layer can only skip bad data. */
			if ((uintps_t) arg == AUTO_SKIP) 
			   {
			   switch(fio->rtype)
				{
				case TR_IBM_U:
				case TR_IBM_V:
				case TR_IBM_VB:
				case TR_IBM_VBS:
				/* Tell the lower layer to skip all bad data */
					ret = XRCALL(llfio,fcntlrtn) llfio, 
					cmd, ((void *) AUTO_SKIPALL), stat);
					if (ret == 0)
					   {
					   vf_info = (struct gen_vf *)fio->lyr_info;
					   vf_info->skipbad = 1;
					   }
					break;
				default:
					ERETURN(stat, FENOSKPB, 0) 
				}
			   }
			else
				ERETURN(stat, FDC_ERR_REQ, 0);
			break;

		default:
			if (IS_RESERVED_FC(cmd))
				{
				ret = XRCALL(llfio,fcntlrtn) 
					llfio, cmd, arg, stat); 
				}
			else
				{ 
				ERETURN(stat, FDC_ERR_NOSUP, 0) 
				} 
			break;   
		}
	return(ret);
	}
