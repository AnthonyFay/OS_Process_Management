#include "syslib.h"

int sys_chrt(endpoint_t proc_ep, long deadline)
{
    message m;

    m.m2_i1 = proc_ep;
    m.m2_l1 = deadline;

    return(_kernel_call(SYS_CHRT, &m));
}