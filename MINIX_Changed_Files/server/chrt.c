/* This file deals with the chrt system call.
 *
 * The entry points into this file is:
 *   do_chrt: perform the chrt system call
 */

#include "pm.h"
#include <sys/wait.h>
#include <assert.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/sched.h>
#include <minix/vm.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <signal.h>
#include "mproc.h"

int do_chrt(void)
{
    sys_chrt(who_p, m_in.m2_l1);
    return(OK);
}