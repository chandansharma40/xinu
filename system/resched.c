/* resched.c - resched, resched_cntl */

#include <xinu.h>

void prio_boosting();

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct 	procent *ptold;	/* Ptr to table entry for old process	*/
	struct 	procent *ptnew;	/* Ptr to table entry for new process	*/
	struct 	procent *prptr;	
	int 	proc_temp;			/* For switching queues and stuff		*/
	int 	temp_time_slice;
	pid32	old_pid, new_pid;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	old_pid = currpid;
	ptold = &proctab[currpid];

	if(ptold->usr_proc_flag == 1)
	{
		//kprintf("PID: %d, ta: %d\n", currpid, proctab[currpid].time_alloted);
		if((ptold->time_alloted == 0) && (boost_cnt < PRIORITY_BOOST_PERIOD))
		{
			//kprintf("PID: %d, Qnum: %d\n", currpid, proctab[currpid].qnum);
			switch(proctab[currpid].qnum)
			{
				case 1:	proctab[currpid].qnum = 2;
						proctab[currpid].time_alloted = TIME_ALLOTMENT;
						//kprintf("Time allotment given back as: %d\n", proctab[currpid].time_alloted);
						proctab[currpid].prstate = PR_READY;
						insert(currpid, readylisy_usr2, proctab[currpid].prprio);
						//kprintf("Did this\n");
						break;
				case 2:	proctab[currpid].qnum = 3;
						proctab[currpid].time_alloted = TIME_ALLOTMENT;
						//kprintf("Time allotment given back as: %d\n", proctab[currpid].time_alloted);
						proctab[currpid].prstate = PR_READY;
						insert(currpid, readylisy_usr3, proctab[currpid].prprio);
						break;
				case 3:	proctab[currpid].time_alloted = TIME_ALLOTMENT;
						//kprintf("Time allotment given back as: %d\n", proctab[currpid].time_alloted);
						proctab[currpid].prstate = PR_READY;
						break;
			}
		}
	}

	if(ptold->prstate == PR_CURR)
	{
		if(ptold->usr_proc_flag == 0)
		{
			if (ptold->prprio > firstkey(readylist)) {
				return;
			}
			ptold->prstate = PR_READY;
			insert(currpid, readylist, proctab[currpid].prprio);
		}
		else
		{
			switch(ptold->qnum)
			{
				case 1:	ptold->prstate = PR_READY;
						insert(currpid, readylisy_usr1, proctab[currpid].prprio);
						break;

				case 2:	ptold->prstate = PR_READY;
						insert(currpid, readylisy_usr2, proctab[currpid].prprio);
						break;

				case 3:	ptold->prstate = PR_READY;
						insert(currpid, readylisy_usr3, proctab[currpid].prprio);
						break;				
			}
		}
	}

	if(boost_cnt >= PRIORITY_BOOST_PERIOD)
	{
		//kprintf("In Boost Prio\n");
		prio_boosting();
	}

	if(firstid(readylist) != NULLPROC)
	{
		currpid = dequeue(readylist);
		temp_time_slice = TIME_SLICE;
	}
	else
	{
		if(nonempty(readylisy_usr1))
		{
			currpid = dequeue(readylisy_usr1);
			temp_time_slice = TIME_SLICE;
			//kprintf("PID from HLQ : %d\n",currpid);
		}
		else if(nonempty(readylisy_usr2))
		{
			currpid = dequeue(readylisy_usr2);
			temp_time_slice = TIME_SLICE * 2;
			//kprintf("PID from MLQ : %d\n",currpid);
		}
		else if(nonempty(readylisy_usr3))
		{
			//print_ready_list();
			currpid = dequeue(readylisy_usr3);
			temp_time_slice = TIME_SLICE * 4;
			//kprintf("PID from LLQ : %d\n",currpid);
		}
		else
		{
			currpid = dequeue(readylist);
			temp_time_slice = TIME_SLICE;
		}
	}

	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = temp_time_slice;

	if(old_pid != currpid)
	{
		#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",old_pid, currpid);
		#endif
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

/************PRIORITY BOOSTING SEQUENCE***************/

void prio_boosting()
{
	pid32 proc_temp;
	pid32 it;
	struct procent *prptr;

	boost_cnt = 0;
	if(nonempty(readylisy_usr1)){
		proc_temp = firstid(readylisy_usr1);
		while(proc_temp != queuetail(readylisy_usr1)){
			prptr = &proctab[proc_temp];
			prptr->time_alloted = TIME_ALLOTMENT;
			prptr->prstate = PR_READY;
			proc_temp = queuetab[proc_temp].qnext;
		}
	}
	while(!isempty(readylisy_usr2)){
		proc_temp = dequeue(readylisy_usr2);
		enqueue(proc_temp, readylisy_usr1);
		prptr = &proctab[proc_temp];
		prptr->qnum = 1;
		prptr->time_alloted = TIME_ALLOTMENT;
		prptr->prstate = PR_READY;
	}
	while(!isempty(readylisy_usr3)){
		proc_temp = dequeue(readylisy_usr3);
		enqueue(proc_temp, readylisy_usr1);
		prptr = &proctab[proc_temp];
		prptr->qnum = 1;
		prptr->time_alloted = TIME_ALLOTMENT;
		prptr->prstate = PR_READY;
	}

	it = firstid(sleepq);
	while(it != lastid(sleepq))
	{
		if(proctab[it].usr_proc_flag == 1)
		{
			proctab[it].qnum = 1;
			proctab[it].time_alloted = TIME_ALLOTMENT;
		}
		it = queuetab[it].qnext;
	}

	//print_ready_list();
}

/************PRIORITY BOOSTING SEQUENCE END***********/
