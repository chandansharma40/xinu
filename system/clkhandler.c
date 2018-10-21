/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/

	if(proctab[currpid].usr_proc_flag == 1 && proctab[currpid].prstate == PR_CURR && proctab[currpid].burst_duration > 0 && proctab[currpid].burst_done != 1)
	{
		boost_cnt++;
		proctab[currpid].burst_duration--;
		proctab[currpid].time_alloted--;
		if(proctab[currpid].time_alloted <= 0)
		{
			//resched();
			proctab[currpid].time_alloted = 0;
		}
		if(proctab[currpid].burst_duration == 0)
		{
			proctab[currpid].burst_done = 1;
		}
	}

	/* Decrement the ms counter, and see if a second has passed */

	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	if(proctab[currpid].burst_done == 1)
	{
		proctab[currpid].burst_duration = proctab[currpid].set_burst_duration;
		proctab[currpid].no_of_bursts--;
		proctab[currpid].burst_done = 0;
		if(proctab[currpid].no_of_bursts == 0)
		{
			//kprintf("PID %d is killed here\n", currpid);
			kill(getitem(currpid));
		}
		else if(proctab[currpid].no_of_bursts > 1)
		{
			//kprintf("PID %d going to sleep at no of burst %d\n",currpid,proctab[currpid].no_of_bursts);
			sleepms(proctab[currpid].sleep_duration);
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		preempt = TIME_SLICE;
		resched();
	}
}
