#include <xinu.h>
#include <stdio.h>

int main() {
	pid32 prA, prB;

    prA = create_user_proc(timed_execution, 1024, 10, "timed_execution", 1, 10);
    prB = create_user_proc(timed_execution, 1024, 10, "timed_execution", 1, 10);
	set_tickets(prA, 0);
	set_tickets(prB, 100);
    resume(prA);
    resume(prB);
    sleepms(10);
    kill(prA);
    kill(prB);

    return OK;
}
