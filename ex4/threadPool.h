// Narkis Kremizi Shallev 205832447

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <sys/param.h>
#include "osqueue.h"

typedef struct task{
    void* param;
    void (*computeFunc) (void *);
}Task;

typedef struct thread_pool{
    int numOfThreads;
    OSQueue* taskQueue;
    pthread_mutex_t taskQueueMutex;
    pthread_t* threadList;
    pthread_cond_t condVariable;
    pthread_mutex_t condVariableMutex;
    int canRunRestOfTasks;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

void allocationTest(void* tested);

void systemCallTest(int tested);

Task* takeTaskFromQueue(ThreadPool* threadPool);

void bootThreadPollFields(ThreadPool* threadPool, int numOfThreads);

void bootTaskQueue(ThreadPool* threadPool);

void bootThreads(ThreadPool* threadPool, int numOfThreads);

void makeTheThreadWait(ThreadPool* threadPool);

void waitTheThreadsToFinish(ThreadPool *threadPool);

void freeAll(ThreadPool *threadPool);

#endif
