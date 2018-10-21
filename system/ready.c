/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16 	readylisy_usr1;		/* High priority			*/
qid16 	readylisy_usr2;		/* Medium priority			*/
qid16 	readylisy_usr3;		/* Low priority				*/

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
	if(prptr->usr_proc_flag == 0)
	{
		insert(pid, readylist, prptr->prprio);
	}
	else
	{
		switch(proctab[currpid].qnum)
		{
			case 1 	:	insert(pid, readylisy_usr1, prptr->prprio);
					 	break;
			case 2	:	insert(pid, readylisy_usr2, prptr->prprio);
						break;
			case 3	:	insert(pid, readylisy_usr3, prptr->prprio);
						break;
			default	:	insert(pid, readylisy_usr1, prptr->prprio);
						break;
		}
	}
	//resched();
	return OK;
}
