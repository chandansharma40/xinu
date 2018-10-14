/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	int 		winner;
	int 		temp_tickets;
	pid32 		curr;
	pid32		next_proc;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	if(max_tickets > 0 && firstid(readylist) == NULLPROC && (((currpid == 5) && (proctab[currpid].prstate == PR_SLEEP)) || (proctab[currpid].usr_proc_flag == 1)))
	{
		curr = firstid(readylist_usr);
		winner = rand()%max_tickets;
		//kprintf("Winner number : %d\n",winner);
		temp_tickets = proctab[curr].no_of_tickets;
		while (curr != queuetail(readylist_usr))
		{
			if (winner >= temp_tickets)
			{
				curr = queuetab[curr].qnext;
				temp_tickets += proctab[curr].no_of_tickets; 
			}
			else break;
		}
			if(curr == currpid)
			{
				preempt = QUANTUM;
			}
			else
			{
				ptold = &proctab[currpid];
				if(ptold->usr_proc_flag == 1)
				{
					if(ptold->run_time > 0)
					{
						ptold->prstate == PR_READY;
					}
				}
				kprintf("ctxsw::%d-",currpid);
				currpid = curr;
				kprintf("%d\n",currpid);
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = QUANTUM;		/* Reset time slice for process	*/
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			}
		
	}
	else
	{
		/* Point to process table entry for the current (old) process */
		ptold = &proctab[currpid];
		if(ptold->usr_proc_flag == 1)
		{
			if(ptold->run_time > 0)
			{
				ptold->prstate == PR_READY;
			}
		}
		else{
			if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
				if (ptold->prprio > firstkey(readylist)) {
					return;
				}

				/* Old process will no longer remain current */

				ptold->prstate = PR_READY;
				insert(currpid, readylist, ptold->prprio);
			}
		}
		

		/* Force context switch to highest priority ready process */
		kprintf("ctxsw::%d-",currpid);

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/

		kprintf("%d\n",currpid);
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	}
	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
