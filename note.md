# Experiment2：进程管理



- ### 核心是 `int chrt(long deadline)` 函数

- ### 在 MINIX3.3.0 的 应用层、服务层、内核层 修改代码，实现系统调用 chrt



#### 注意事项：

- 添加C文件后，需在同目录下的Makefile/Makefile.inc中添加条目



#### 调试方法：

​		在vmware虚拟机关闭情况下，在设置中点击添加设备，再选择添加串口，简单情况下可以使用物理机文件output.txt作为串口输出目标。

- 启动虚拟机，运行 echo “hello world” > /dev/tty00，可以在output.txt中观察到hello world的输出结果。

- 在内核调试中，可以通过向/dev/tty00文件写入字符的形式进行调试（例如使用fopen和fprintf）。



#### 编译：

- `make build`
- `make build MKUPDATE=yes`



#### C 关键词：

register - 这个关键字请求编译器尽可能的将变量存在CPU内部寄存器中，而不是通过内存寻址访问，以提高效率。



## 一、应用层

> 需要添加的系统调用 chrt 可以定义在 unistd 头文件中，并在 libc 中添加 chrt 函数体实现。



- 在 `/usr/src/include/unistd.h` 中添加 `chrt` 函数定义（line 88 - line 91）

  ```c
  int chrt(long);		/* 没有形式参数！ */
  ```

  

- 在 `/usr/src/minix/lib/libc/sys/chrt.c` 中添加 `chrt` 函数实现

  ```c
  #include <sys/cdefs.h>
  #include "namespace.h"
  #include <lib.h>
  
  #include <string.h>
  #include <unistd.h>
  #include <time.h>
  
  /* 	int clock_gettime(clockid_t clk_id, struct timespec* tp) - Get time
   *  ===================================================================
   *
   *	clockid_t :				
   *				CLOCK_REALTIME		//系统当前时间，从1970年1月1日算起
   *				CLOCK_MONOTONIC     //系统的启动后运行时间，不能被设置
   *				CLOCK_PROCESS_CPUTIME_ID	//本进程运行时间
   *				CLOCK_THREAD_CPUTIME_ID     //本线程运行时间
   *
   */
  
  /* 	struct timespec
   *	{
   *  	time_t tv_sec;		//秒
   *      long tv_nsec; 		//纳秒
   *	};
   *
   */
  
  /*	unsigned int alarm(unsigned int seconds) - Set the alarm for signal
   *		transmission
   *	Sets the signal SIGALRM to be sent to the current process after a 
   *  number of seconds has elapsed
   *	===================================================================
   *
   *	If alarm is called again within seconds to set a new alarm, 
   *	the setting of the later timer will override the previous setting, 
   *	that is, the number of seconds set before is replaced 
   *  by the new alarm time.
   *	When the seconds parameter is 0, the previously set timer alarm is 
   *	cancelled and the remaining time is returned.
   *
   */
  
  /*	int _syscall(endpoint_t who, int syscallnr, message *msgptr)
   *	===================================================================
   *
   *	Pass the message to the server
   *
   */
  
  int chrt(long deadline)
  {
      message m;
      struct timespec time;
  
      memset(&m, 0, sizeof(m));
  
      alarm((unsigned int)deadline);
  
      clock_gettingtime(CLOCK_REALTIME, &time);
      deadline = time.tv_sec + deadline;
      m.m2_l1 = deadline;
  
      return(_syscall(PM_PROC_NR, PM_CHRT, &m));
  }
  ```
  
  注：为什么选择使用 `m2_m1` 存放 `deadline` ？
  
  在《操作系统设计与实现 3rd ed.》Page 98中，进程间通信的数据结构在 `ipc.h` 中定义，一条 `message` 包含 `m_source` 域、 `m_type` 域和数据域。MINIX使用了如下的七种数据类型。
  
  <img src="E:\undergraduate\sophomore\SEM2\OS\experiments\2 Process Management\note_img\message.png" alt="message" style="zoom: 50%;" />
  
  形如 `m1_i1` 这样的短名字代表了消息格式（消息类型为1，i代表int，其他短名字以此类推）。
  
  `m2_l1` 代表消息类型2的第一个长整数，恰好可以用来存放 `deadline`。
  
  
  
- 在 `/usr/src/minix/lib/libc/sys` 中 `Makefile.inc` 文件添加 `chrt.c` 条目



## 二、服务层

> 需要向MINIX系统的进程管理服务中注册 chrt，使得 chrt 服务可以向应用层提供。



- 在 `/usr/src/minix/servers/pm/proto.h` 中添加 `chrt` 函数定义

  ```c
  int do_chrt(void);
  ```



- 在 `/usr/src/minix/servers/pm/chrt.c` 中添加 `chrt` 函数实现，调用 `sys_chrt()` 

  ```c
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
  ```

  

- 在 `/usr/src/minix/include/minix/callnr.h` 中定义 `PM_CHRT` 编号（line 62 -  line 67）

  ```c
  #define PM_CHRT (PM_BASE + 48)
  
  #define NR_PM_CALLS 49 /* highest number from base plus one */
  ```



- 在 `/usr/src/minix/servers/pm/Makefile` 中添加 `chrt.c` 条目



- 在 `/usr/src/minix/servers/pm/table.c` 中调用映射表

  ```c
  CALL(PM_CHRT) = do_chrt			/* chrt(2) */
  ```

  

- 在 `/usr/src/minix/include/minix/syslib.h` 中添加 `sys_chrt ()` 定义

  ```c
  int sys_chrt(endpoint_t proc_ep, long deadline);
  ```



- 在 `/usr/src/minix/lib/libsys/sys_chrt.c` 中添加 `sys_chrt ()` 实现

  ```c
  /* 	message struct - /usr/src/minix/include/minix/ipc.h
   *  ===================================================
   *
   *	typedef struct {
   * 		endpoint_t m_source;	//who sent the message
   * 		int m_type;             //what kind of message is it
   * 		union {
   *          mess_1 m_m1;
   *       	mess_2 m_m2;
   *       	mess_3 m_m3;
   *       	mess_4 m_m4;
   *      	mess_5 m_m5;
   *       	mess_7 m_m7;
   *       	mess_8 m_m8;
   *       	mess_6 m_m6;
   *       	mess_9 m_m9;
   * 		} m_u;
   *	} message;
   *
   */
  
  #include "syslib.h"
  
  int sys_chrt(endpoint_t proc_ep, long deadline)
  {
      message m;
      m.m2_l1 = deadline;
      m.m2_i1 = proc_ep;
      return(_kernel_call(SYS_CHRT, &m));
  }
  ```



- 在 `/usr/src/minix/lib/libsys` 中的 `Makefile` 中添加 `sys_chrt.c` 条目



## 三、内核层

> 在MINIX内核中实现进程调度功能，此处可以直接修改内核信息，例如进程的截止时间。



- 在 `/usr/src/minix/kernel/system.h` 中添加 `do_chrt` 函数定义

  ```c
  int do_chrt(struct proc * caller, message *m_ptr);
  #if ! USE_CHRT
  #define do_chrt NULL
  #endif
  ```



- 在 `/usr/src/minix/kernel/system/do_chrt.c` 中添加 `do_chrt` 函数实现

  > The **process number** of the system task is easy. It is **SYSTEM**. This is a macro defined in /usr/src/include/minix as **(-2)**. So it is **not quite** the index into the process table. To get the index into the kernel's version of the process table you have to add **NR_TASKS** (which is 4). The kernel's process table has 4 more entries than the process manager's process table.
  >
  > ```c
  > #define proc_addr(n)      (&(proc[NR_TASKS + (n)]))
  > ```
  
  
  
  ```c
  int do_chrt(struct proc *caller, message *m_ptr)
  {
      struct proc *rp;
      long exp_time;
  
      exp_time = m_ptr->m2_l1;
      rp = proc_addr(m_ptr->m2_i1);
      rp->p_deadline = exp_time;
  
      return(OK);
  }
  ```



> ### Pm's glo.h header
>
> The "glo.h" file in the pm's directory defines a pointer to the slot in the process table (the pm's table) for the current process; i.e., the user process that has sent a message to the pm process.
>
> ```c
> struct mproc * mp;
> ```
>
> The header file also declares several important global pm variables that are filled in for each message that is received:
>
> ```c
> message m_in;
> int who_e;
> int who_p;
> int call_nr;
> ```
>
> The process manager repeatedly tries to receive a message from any (user) process by calling a function: *get_work*.
>
> This function calls *receive*, which will block until some message is sent to the process manager.
>
> Once a message is received, the get_work function extracts information from the message about the sender and assigns values to:
>
> - m_in - where the incomming message is placed by the kernel.
> - mp - pointer to the process table entry of the user who sent the message
> - who_e - the endpoint value of the same user
> - who_p - the index of this user's entry in the process table.
> - call_nr - the number of the user requested system call
>
> The get_work function makes a check that the process table entry at index who_p is still being used by the user who sent the message.
>
> via. https://condor.depaul.edu/glancast/443class/docs/slides/Sep14/slide25.html



- 在 `/usr/src/minix/kernel/system/` 中 `Makefile.inc` 文件添加 `do_chrt.c` 条目



- 在 `/usr/src/minix/include/minix/com.h` 中定义 `SYS_CHRT` 编号

  ```c
  #  define SYS_CHRT (KERNEL_CALL + 58) 		/* sys_chrt() */
  
  /* Total */
  #define NR_SYS_CALLS	59	/* number of kernel calls */
  ```



- 在 `/usr/src/minix/kernel/system.c` 中添加 `SYS_CHRT` 编号到 `do_chrt` 的映射

  ```c
  map(SYS_CHRT, do_chrt);   		/* set the process execution period */
  ```



- 在 `/usr/src/minix/commands/service/parse.c` 的 `system_tab` 中添加名称编号对

  ```c
  { "CHRT",			SYS_CHRT},
  ```




## 四、进程调度

> 进程调度模块位于 /usr/src/minix/kernel/ 下的 proc.h 和 proc.c ，修改影响进程调度顺序的部分。



> struct proc 维护每个进程的信息，用于调度决策
>
> switch_to_user() 选择进程进行切换
>
> enqueue_head() 按优先级将进程加入列队首
>
> enqueue() 按优先级将进程加入列队尾
>
> pick_proc() 从队列中返回一个可调度的进程 ，遍历设置的优先级队列，返回剩余时间最小并可运行的进程



- 在`struct proc`中添加 deadline 成员

  ```c
  long p_deadline;	/* chrt deadline */
  ```

  

- 在`enqueue_head()`和`enqueue()`中，将实时进程的优先级设置成合适的优先级

  > ```c
  > 05562   #define NR_SCHED_QUEUES   16    /* MUST equal minimum priority + 1 */ 
  > 05563   #define TASK_Q             0    /* highest, used for kernel tasks */ 
  > 05564   #define MAX_USER_Q         0    /* highest priority for user processes */ 
  > 05565   #define USER_Q             7    /* default (should correspond to nice 0) */ 
  > 05566   #define MIN_USER_Q        14    /* minimum priority for user processes */ 
  > 05567   #define IDLE_Q            15    /* lowest, only IDLE process goes here */
  > ```

  ```c
  /* if a process's deadline is positive, it is a real-time process.
   * Its priority is supposed to be set to a appropriate value, which is 5 here
   */
  if (rp->p_deadline > 0)
  {
  	rp->p_priority = 5;
  }
  ```



- 遍历设置的优先级队列，返回剩余时间最小并可运行的进程

  在`pick_proc`函数中遍历`p_priority`等于 5 的进程队列，比较`p_deadline`的大小。`p_deadline`较小的应被置于要调度的队列的前面

  

  > Minix has three ready queues, for different types of process. This is like a simple priority system.
  >
  > - Task queue
  > - Server queue
  > - User queue
  >
  > They are all held together, in an array called `rdy_head`.

  

  ```c
  static struct proc * pick_proc(void)
  {
  /* Decide who to run now.  A new process is selected and returned.
   * When a billable process is selected, record it in 'bill_ptr', so that the 
   * clock task can tell who to bill for system time.
   *
   * This function always uses the run queues of the local cpu!
   */
    register struct proc *rp;			/* process to run */
    struct proc **rdy_head;
    struct proc *temp;
    int q;				/* iterate over queues */
  
    /* Check each of the scheduling queues for ready processes. The number of
     * queues is defined in proc.h, and priorities are set in the task table.
     * If there are no processes ready to run, return NULL.
     */
    rdy_head = get_cpulocal_var(run_q_head);
    for (q=0; q < NR_SCHED_QUEUES; q++) {	
  	if(!(rp = rdy_head[q])) {
  		TRACE(VF_PICKPROC, printf("cpu %d queue %d empty\n", cpuid, q););
  		continue;
  	}
  
  	/* Compare p_deadline. The Process that has the smaller p_deadline should
       * be placed in front of the queue that is to be scheduled.
       */
      if (q == 5) {
    		rp = rdy_head[q];
  		temp = rp->p_nextready;
  		
  		while (!temp)
  		{
  			if (temp->p_deadline > 0) {
  				if (rp->p_deadline == 0 || rp->p_deadline > temp->p_deadline) {
  					if (proc_is_runnable(temp)) {
  						rp = temp;
  					}
  				}
  			}
  			temp = temp->p_nextready;
  		}
    	}
  
  	assert(proc_is_runnable(rp));
  	if (priv(rp)->s_flags & BILLABLE)	 	
  		get_cpulocal_var(bill_ptr) = rp; /* bill for system time */
  	return rp;
    }
    return NULL;
  }
  ```
