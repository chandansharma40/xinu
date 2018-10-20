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

	curr = firstid(readylisy_usr1);

	kprintf("HIGH PRIO List\n----------\n");

	while(curr != queuetail(readylisy_usr1))
	{
		kprintf("PID = %d   State = %s\n",curr,pstate[(int)proctab[curr].prstate]);
		curr = queuetab[curr].qnext;	
	}

	curr = firstid(readylisy_usr2);

	kprintf("Ready List\n----------\n");

	while(curr != queuetail(readylisy_usr2))
	{
		kprintf("PID = %d   State = %s\n",curr,pstate[(int)proctab[curr].prstate]);
		curr = queuetab[curr].qnext;	
	}

	curr = firstid(readylisy_usr3);

	kprintf("Ready List\n----------\n");

	while(curr != queuetail(readylisy_usr3))
	{
		kprintf("PID = %d   State = %s\n",curr,pstate[(int)proctab[curr].prstate]);
		curr = queuetab[curr].qnext;	
	}
	return OK;
}
