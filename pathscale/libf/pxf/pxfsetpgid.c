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


#pragma ident "@(#) libf/pxf/pxfsetpgid.c	92.1	06/29/99 11:36:06"

#include <errno.h>
#include <liberrno.h>
#include <fortran.h>
#include <sys/types.h>
#include <unistd.h>

/* PXFSETPGID  -- Set Process Group ID for Job Control
 * (section 4.3.3 of Posix 1003.9-1992)
 *
 * Synopsis:
 *
 *    SUBROUTINE PXFSETPGID(IPID, IPGID, IERROR)
 *    INTEGER IPID, IPGID, IERROR
 *
 *
 * Function Description:
 * The routine PXFSETPGID uses the setpgid(2) system call to change the
 * process group ID of the process with process ID IPID. The process group
 * ID may be for an existing process group or a new process group which will
 * be created. Upon sucessful completion, the process with process ID IPID
 * will have its process group ID set to IPGID.
 *
 * Description of Arguments:
 *
 * IPID   is an input integer variable. IPID contains the process ID of the
 *        process to change the process group ID. As a special case, if IPID
 *        is zero the process ID of the calling process is used.
 *
 * IPGID  is an input integer variable containing the new process group ID.
 *
 * IERROR is an output integer variable that will contain
 *        the status:
 *
 *         zero    - PXFSETPGID was successful.
 *
 *         nonzero - PXFSETPGID was not successful.
 *
 *        PXFSETPGID may return any of the following error values:
 *
 *        EACCES         If the value of IPID matches the process ID of a child
 *                       process of the calling process and the child
 *                       process has successfully executed one of the
 *                       PXFEXEC(3F) functions.
 *
 *        EINVAL         If the value of IPGID is less than 0 or is not a value
 *                       supported by the implementation.
 *
 *        EPERM          If the process indicated by IPID is a session leader.
 *                       If the value of IPID is valid but matches the process
 *                       ID of a child process of the calling process and
 *                       the child process is not in the same session as
 *                       the calling process.
 *                       If the value of IPGID does not
 *                       match the process ID of the process indicated by
 *                       pid and no process with a process group ID exists
 *                       that matches the value of IPGID in the same session
 *                       as the calling process.
 *
 *        ESRCH          The value of IPID does not match the ID of the
 *                       calling process or of a child of the calling process.
 *
 */

#ifdef _UNICOS
void
PXFSETPGID(
#else
void
_PXFSETPGID(
#endif
	    _f_int *IPID,
	    _f_int *IPGID,
	    _f_int *IERROR
)
{
  pid_t cipid, cipgid;

  cipid = *IPID;
  cipgid = *IPGID;

  if (setpgid(cipid, cipgid) == -1) {
    *IERROR = errno;
    return;
  }

  *IERROR = 0;
}

#ifndef _UNICOS
void
pxfsetpgid_(
	    _f_int *IPID,
	    _f_int *IPGID,
	    _f_int *IERROR
)
{  
  _PXFSETPGID(IPID, IPGID, IERROR);
}
#endif
