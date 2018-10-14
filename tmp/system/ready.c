/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	readylist_usr;			/* Index of user readylist	*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */
	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	//kprintf("in ready.c %d\n", pid);
	if(proctab[pid].usr_proc_flag == 1)
	{
		insert(pid, readylist_usr, prptr->no_of_tickets);
	}
	else
	{
		insert(pid, readylist, prptr->prprio);
	}
	resched();

	return OK;
}
