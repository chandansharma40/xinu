#include <xinu.h>
#include <stdio.h>

int prA, prB;
int proc_a(), proc_b();
volatile int a_cnt = 0;
volatile int b_cnt = 0;

//TIME_SLICE            2
//TIME_ALLOTMENT        400
//PRIORITY_BOOST_PERIOD 2200

int main() {
    double total_cnt, a_percent, b_percent;
	int i;

    kprintf("\n\n########## Test Case 1, MLFQ scheduling (2 processes):\n");
    resume( prA = create_user_proc(proc_a, 1024, 10000, "proc A", 3, 1, 10000, 0) );
    resume( prB = create_user_proc(proc_b, 1024, 10000, "proc B", 3, 1, 10000, 0) );
    sleepms(1000);
	for(i=0;i<100000000;i++);
	kill(prA);
	kill(prB);
    total_cnt = a_cnt + b_cnt ;
    a_percent = (double) a_cnt / total_cnt * 100;
    b_percent = (double) b_cnt / total_cnt * 100;
    kprintf("Test RESULT: A : B (%d : %d)\n", (int) a_percent, (int) b_percent);

	return OK;
}


proc_a(c)
char c; {
    int i;
    while (1) {
        for (i = 0; i < 1000; i++)
            ;
        a_cnt++;
    }
}

proc_b(c)
char c; {
    int i;
    while (1) {
        for (i = 0; i < 1000; i++)
            ;
        b_cnt++;
    }
}

