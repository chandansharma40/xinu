#include <xinu.h>

syscall print_ready_list()
{
	int16	curr;
	int16	prev;
	char *pstate[]	= {		/* names for process states	*/
		"free ", "curr ", "ready", "recv ", "sleep", "susp ",
		"wait ", "rtime"};

	curr = firstid(readylist);

	kprintf("Ready List\n----------\n");

	while(curr != queuetail(readylist))
	{
		kprintf("PID = %d   State = %s\n",curr,pstate[(int)proctab[curr].prstate]);
		curr = queuetab[curr].qnext;	
	}

	curr = firstid(readylist_usr);
	kprintf("in print.c %d\n",curr);

	kprintf("User List\n----------\n");

	while(curr != queuetail(readylist_usr))
	{
		kprintf("PID = %d   State = %s\n",curr,pstate[(int)proctab[curr].prstate]);
		curr = queuetab[curr].qnext;	
	}
	return OK;
}
