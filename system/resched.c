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
	struct procent *prptr;	
	int proc_temp;			/* For switching queues and stuff		*/
	int temp_time_slice;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	/************PRIORITY BOOSTING SEQUENCE***************/
	if(boost_cnt == PRIORITY_BOOST_PERIOD)
	{
		if(nonempty(readylisy_usr1)){
			proc_temp = firstid(readylisy_usr1);
			while(proc_temp != lastid(readylisy_usr1)){
				prptr = &proctab[proc_temp];
				prptr->time_alloted = TIME_ALLOTMENT;
				proc_temp = queuetab[proc_temp].qnext;
			}
		}
		while(!isempty(readylisy_usr2)){
			proc_temp = dequeue(readylisy_usr2);
			enqueue(proc_temp, readylisy_usr1);
			prptr = &proctab[proc_temp];
			prptr->qnum = 1;
			prptr->time_alloted = TIME_ALLOTMENT;
		}
		while(!isempty(readylisy_usr3)){
			proc_temp = dequeue(readylisy_usr3);
			enqueue(proc_temp, readylisy_usr1);
			prptr = &proctab[proc_temp];
			prptr->qnum = 1;
			prptr->time_alloted = TIME_ALLOTMENT;
		}
	}
	/************PRIORITY BOOSTING SEQUENCE END***********/

	/************USER PROCESS*****************************/
	else if(ptold->usr_proc_flag == 1)
	{
		if((proctab[currpid].time_alloted == 0) && (proctab[currpid].prstate == PR_CURR))
		{
			/* If time alloted finishes and need to demote current process */
			switch(proctab[currpid].qnum)
			{
				case 1:
					insert(currpid, readylisy_usr2, proctab[currpid].prprio);
					proctab[currpid].qnum = 2;
					proctab[currpid].time_alloted = TIME_ALLOTMENT;
					/* if burst duration finishes alongwith time allotted */
					if(proctab[currpid].burst_duration == 0)
						proctab[currpid].burst_done = 1;
					break;
				case 2:
					insert(currpid, readylisy_usr3, proctab[currpid].prprio);
					proctab[currpid].qnum = 3;
					proctab[currpid].time_alloted = TIME_ALLOTMENT;
					if(proctab[currpid].burst_duration == 0)
						proctab[currpid].burst_done = 1;
					break;
				case 3:
					proctab[currpid].time_alloted = TIME_ALLOTMENT;
					if(proctab[currpid].burst_duration == 0)
						proctab[currpid].burst_done = 1;
					break;
				default:
					break;
			}

			/* If readylist not empty and first proc not NULL/ Main came back from sleep */
			if ((nonempty(readylist)) && (firstid(readylist) != NULLPROC))
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylist);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = QUANTUM;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				return;
			}
			/* then just going by the flow */
			else if(nonempty(readylisy_usr1))
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylisy_usr1);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = TIME_SLICE;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				return;
			}
			else if(nonempty(readylisy_usr2))
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylisy_usr2);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = TIME_SLICE * 2;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				return;
			}
			else if(nonempty(readylisy_usr3))
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylisy_usr3);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = TIME_SLICE * 4;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				return;
			}
			/* if all empty, schedule NULL */
			else 
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylist);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				preempt = QUANTUM;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			}
		}
		/* if main came back from sleep, make current user proc PR_READY and push back to same queue*/
		else if(ptold->prstate == PR_CURR)
		{
			if(firstid(readylist != NULLPROC))
			{
				ptold->prstate = PR_READY;
				switch(ptold->qnum){
					case 1:
						insert(currpid, readylisy_usr1, ptold->prprio);
						break;
					case 2:
						insert(currpid, readylisy_usr2, ptold->prprio);
						break;
					case 3:
						insert(currpid, readylisy_usr3, ptold->prprio);
						break;
					default:
						break;
				}
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylist);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = QUANTUM;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			}
			else
			{
				if(nonempty(readylisy_usr1))
				{
					/* assigning ptold it's current user queue*/
					if(proctab[currpid].qnum == 1)
						insert(currpid, readylisy_usr1, proctab[currpid].prprio);
					else if(proctab[currpid].qnum == 2)
						insert(currpid, readylisy_usr2, proctab[currpid].prprio);
					else if(proctab[currpid].qnum == 3)
						insert(currpid, readylisy_usr3, proctab[currpid].prprio);
					
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylisy_usr1);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptold->prstate = PR_READY;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
				else if(nonempty(readylisy_usr2))
				{
					if(proctab[currpid].qnum == 1)
						return; //Since there is someone in a higher queue
					else if(proctab[currpid].qnum == 2)
						insert(currpid, readylisy_usr2, proctab[currpid].prprio);
					else if(proctab[currpid].qnum == 3)
						insert(currpid, readylisy_usr3, proctab[currpid].prprio);
					
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylisy_usr2);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptold->prstate = PR_READY;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE * 2;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
				else if(nonempty(readylisy_usr3))
				{
					if(proctab[currpid].qnum == 1)
						return;
					else if(proctab[currpid].qnum == 2)
						return;
					else if(proctab[currpid].qnum == 3)
						insert(currpid, readylisy_usr3, proctab[currpid].prprio);
					
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylisy_usr2);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptold->prstate = PR_READY;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE * 4;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
				else
				{
					return;
				}
			}
		}
		/* If no user proc is in current state */
		else
		{
			/* Check if there is any sys proc apart from NULL */
			if(firstid(readylist) != NULLPROC)
			{
				#ifdef CTXSW
				kprintf("ctxsw::%d-",currpid);
				#endif
				currpid = dequeue(readylisy_usr2);
				#ifdef CTXSW
				kprintf("%d\n",currpid);
				#endif
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = QUANTUM;
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			}
			else
			{
				/* If every user list is empty then just push NULL to current*/
				if((isempty(readylisy_usr1))&&
				(isempty(readylisy_usr2))&&
				(isempty(readylisy_usr3)))
				{
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylisy_usr2);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = QUANTUM;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
				/* check which queue to pick user process from */
				else
				{
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif

					if(nonempty(readylisy_usr1))
					{
						currpid = dequeue(readylisy_usr1);
						temp_time_slice = TIME_SLICE;
					}
					else if(nonempty(readylisy_usr2))
					{
						currpid = dequeue(readylisy_usr2);
						temp_time_slice = TIME_SLICE * 2;
					}
					else 
					{
						currpid = dequeue(readylisy_usr3);
						temp_time_slice = TIME_SLICE * 4;
					}

					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif

					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = temp_time_slice;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
			}
		}
	}
	/************END USER PROCESS*************************/

	/************SYSTEM PROCESS***************************/
	else
	{
		if(currpid == NULLPROC)
		{
			if(ptold->prstate == PR_CURR)
			{
				if(isempty(readylist))
				{
					if(nonempty(readylisy_usr1))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr1);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else if(nonempty(readylisy_usr2))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr2);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE * 2;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else if(nonempty(readylisy_usr3))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr3);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE * 4;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else 
					{
						return;
					}
				}
				else
				{
					insert(currpid, readylist, ptold->prprio);
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylist);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = QUANTUM;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}

			}
		}
		else /* Not NULLPROC */
		{
			if(ptold->prstate == PR_CURR)
			{
				if(ptold->prprio > firstkey(readylist))
				{
					return;
				}
				else
				{
					insert(currpid, readylist, ptold->prprio);
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylist);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = QUANTUM;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}
			}
			/* Not NULL and not current */
			else
			{
				if(firstid(readylist) == NULLPROC)
				{
					if(nonempty(readylisy_usr1))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr1);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else if(nonempty(readylisy_usr2))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr2);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE * 2;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else if(nonempty(readylisy_usr3))
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylisy_usr3);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE * 4;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
					else
					{
						#ifdef CTXSW
						kprintf("ctxsw::%d-",currpid);
						#endif
						currpid = dequeue(readylist);
						#ifdef CTXSW
						kprintf("%d\n",currpid);
						#endif
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = QUANTUM;
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
				}
				else
				{
					#ifdef CTXSW
					kprintf("ctxsw::%d-",currpid);
					#endif
					currpid = dequeue(readylist);
					#ifdef CTXSW
					kprintf("%d\n",currpid);
					#endif
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = QUANTUM;
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}
			}
		}
	}

	/************END SYSTEM PROCESS***********************/

	// if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
	// 	if (ptold->prprio > firstkey(readylist)) {
	// 		return;
	// 	}

	// 	/* Old process will no longer remain current */

	// 	ptold->prstate = PR_READY;
	// 	insert(currpid, readylist, ptold->prprio);
	// }

	// /* Force context switch to highest priority ready process */

	// currpid = dequeue(readylist);
	// ptnew = &proctab[currpid];
	// ptnew->prstate = PR_CURR;
	// preempt = QUANTUM;		/* Reset time slice for process	*/
	// ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	// /* Old process returns here when resumed */

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
