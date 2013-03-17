// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#include "synch.h"

// testnum is set in main.cc
int testnum = 4;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread priority %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void 
ThreadStatus(int which) 
{
	while (1>0) {
		char buf[10];
		scanf("%s", buf);
		if (strcmp(buf, "ts") == 0) {
			scheduler->Print();
		}
		if (strcmp(buf, "exit") == 0) break;
		
		currentThread->Yield();
		
	}
}

void
ThreadTest_lab1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("thread 1", 1);
    t->Fork(SimpleThread, 1);

	t = new Thread("thread 2", 2);
	t->Fork(SimpleThread, 2);
    
	//t = new Thread("thread status");
	//t->Fork(ThreadStatus, 2);

	SimpleThread(5);
}

int N = 10, M = 10, C = 5;
int buffer = 0;

Lock lock("lock");
Condition cp("producter"), cc("comsumer");
void producter(int which)
{
	printf("yyy\n");
	while(true) {
		lock.Acquire();
		while (buffer == C) cp.Wait(&lock);
		lock.Release();
		
		lock.Acquire();
		buffer++;
		lock.Release();
		printf("product id %d generate, buffer is %d\n", which, buffer);
		
		lock.Acquire();
		cc.Signal(&lock);
		lock.Release();
	}
}

void consumer(int which) 
{
	while (true) {
		lock.Acquire();
		while (buffer == 0) cc.Wait(&lock);
		lock.Release();

		lock.Acquire();
		buffer--;
		printf("consumer id %d, buffer is %d\n", which, buffer);
		lock.Release();

		lock.Acquire();
		cp.Signal(&lock);
		lock.Release();
	}
}

void ThreadTest_lab3() {
	Thread *t;
	for (int i = 1; i <= N; i++) {
		t = new Thread("producter");
		t->Fork(producter, i);
	}
	for (int i = 1; i <= M; i++) {
		t = new Thread("consumer");
		t->Fork(consumer, i);
	}
}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void sender(int which) {
	int id;
	char buf[100];
	int flag;

	while (1>0) {
		printf("please enter the chanel identifier you want to send\n");
		scanf("%s", buf);
		id = ipcManager.msgget(buf);	
		printf("please enter the message to send from sender #%d\n", which);
		scanf("%s", buf);
		flag = ipcManager.msgsend(id, buf);
		if (flag == -1) {
			printf("Error\n");
		}
		currentThread->Yield();
	}
}
void receiver(int which) {
	int id, flag;
	char buf[100], chanel[100];
	while (1>0) {
		printf("please enter the chanel identifier you want to recieve from \n");
		scanf("%s", chanel);
		id = ipcManager.msgget(chanel);

		flag = ipcManager.msgrcv(id, buf);
		
		if (flag == -1) {
			printf("Erroe\n");
		}
		else printf("The message in %s is: %s\n", chanel, buf);

		currentThread->Yield();
	}
}

void ThreadTest_lab4() {
	Thread* t1 = new Thread("send1");
	Thread* t2 = new Thread("receiver1");
	t1->Fork(sender, 1);
	t2->Fork(receiver, 1);
	currentThread->Finish();
}

void
ThreadTest()
{
    switch (testnum) {
    case 1:
		ThreadTest_lab1();
		break;
	case 3:
		ThreadTest_lab3();
		break;
	case 4:
		ThreadTest_lab4();
		break;
    default:
		printf("No test specified.\n");
		break;
    }
}

