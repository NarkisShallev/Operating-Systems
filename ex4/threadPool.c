// Narkis Kremizi Shallev 205832447

#include "stdio.h"
#include <pthread.h>
#include "threadPool.h"
#include "stdlib.h"
#include <unistd.h>
#include <string.h>

#define SYSTEM_CALL_ERROR_MSG write(2,"Error in system call\n",strlen("Error in system call\n"))
#define ALLOCATION_ERROR_MSG write(2,"Error in allocation\n",strlen("Error in allocation\n"))
#define SUCCESS 0
#define FAILURE -1

int isDestroyed;
pthread_mutex_t destroyMutex;

// create threadPool
ThreadPool *tpCreate(int numOfThreads) {
    // create a threadPool
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    // Check if Malloc has worked
    allocationTest(threadPool);

    // Initialize its fields
    bootThreadPollFields(threadPool, numOfThreads);
    return threadPool;
}

// boot the fields of the threadPool
void bootThreadPollFields(ThreadPool *threadPool, int numOfThreads) {
    isDestroyed = 0;
    int returnVal = pthread_mutex_init(&destroyMutex, NULL);
    systemCallTest(returnVal);
    bootTaskQueue(threadPool);
    bootThreads(threadPool, numOfThreads);

    threadPool->canRunRestOfTasks = 1;
}

// boot the tasks queue
void bootTaskQueue(ThreadPool *threadPool) {
    threadPool->taskQueue = osCreateQueue();
    int returnVal = pthread_mutex_init(&threadPool->taskQueueMutex, NULL);
    systemCallTest(returnVal);
}

// boot the threads
void bootThreads(ThreadPool *threadPool, int numOfThreads) {
    threadPool->numOfThreads = numOfThreads;
    threadPool->threadList = (pthread_t *) malloc(sizeof(pthread_t) * numOfThreads);
    // Check if Malloc has worked
    allocationTest(threadPool->threadList);
    // Create threads and run tasks in background
    int i;
    for (i = 0; i < numOfThreads; ++i) {
        pthread_create(&threadPool->threadList[i], NULL, function, threadPool);
    }
    int returnVal = pthread_cond_init(&threadPool->condVariable, NULL);
    systemCallTest(returnVal);
    returnVal = pthread_mutex_init(&threadPool->condVariableMutex, NULL);
    systemCallTest(returnVal);
}

// The function that the threads will run
void *function(void *threadPool) {
    ThreadPool *threadPoolAfterCasting = (ThreadPool *) threadPool;

    // As long as there are tasks in the queue, the threads will run them.
    while (!osIsQueueEmpty(threadPoolAfterCasting->taskQueue)) {
        Task *task = takeTaskFromQueue(threadPoolAfterCasting);
        // run the task
        task->computeFunc(task->param);
        free(task);
        // if the queue isn't destroyed and get 0 break
        if (!threadPoolAfterCasting->canRunRestOfTasks) {
            break;
        }
        //If there are no more tasks, the threads will sleep until they are alerted that a new task has entered.
        if (osIsQueueEmpty(threadPoolAfterCasting->taskQueue) && (!isDestroyed)) {
            makeTheThreadWait(threadPoolAfterCasting);
        }
    }
    return NULL;
}

// make the thread wait to signal
void makeTheThreadWait(ThreadPool *threadPool) {
    int returnVal = pthread_mutex_lock(&threadPool->condVariableMutex);
    systemCallTest(returnVal);
    pthread_cond_wait(&threadPool->condVariable, &threadPool->condVariableMutex);
    returnVal = pthread_mutex_unlock(&threadPool->condVariableMutex);
    systemCallTest(returnVal);
}

// take task from queue, using mutex
Task *takeTaskFromQueue(ThreadPool *threadPool) {
    int returnVal = pthread_mutex_lock(&threadPool->taskQueueMutex);
    systemCallTest(returnVal);
    Task *task = osDequeue(threadPool->taskQueue);
    returnVal = pthread_mutex_unlock(&threadPool->taskQueueMutex);
    systemCallTest(returnVal);
    return task;
}

// Insert new task to the taskQueue. Return -1 for failure and 0 for success
int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if (isDestroyed) {
        return FAILURE;
    }
    Task *task = (Task *) malloc(sizeof(Task));
    // Check if Malloc has worked
    allocationTest(task);
    task->computeFunc = computeFunc;
    task->param = param;
    // Protection by Mutex lock
    int returnVal = pthread_mutex_lock(&threadPool->taskQueueMutex);
    systemCallTest(returnVal);
    osEnqueue(threadPool->taskQueue, task);
    returnVal = pthread_mutex_unlock(&threadPool->taskQueueMutex);
    systemCallTest(returnVal);
    // alert one thread that a new task has entered
    returnVal = pthread_cond_signal(&threadPool->condVariable);
    systemCallTest(returnVal);
    return SUCCESS;
}

// destroy the threadPool
void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    // Handles the case that someone call destroy twice
    if (isDestroyed){
        return;
    }

    isDestroyed = 1;
    // If the function has received 0, the threads will run only the tasks already removed from the queue
    if (shouldWaitForTasks == 0) {
        threadPool->canRunRestOfTasks = 0;
    }
    // alert all of the threads
    int returnVal = pthread_cond_broadcast(&threadPool->condVariable);
    systemCallTest(returnVal);

    // Waiting for all the threads to complete the tasks
    waitTheThreadsToFinish(threadPool);

    // Release memory allocations
    freeAll(threadPool);
}

// wait the threads to finish run the tasks
void waitTheThreadsToFinish(ThreadPool *threadPool) {
    int i;
    for (i = 0; i < threadPool->numOfThreads; ++i) {
        int returnVal = pthread_join(threadPool->threadList[i], NULL);
        systemCallTest(returnVal);
    }
}

// free all the allocations
void freeAll(ThreadPool *threadPool) {
    while (!osIsQueueEmpty(threadPool->taskQueue)) {
        Task *task = takeTaskFromQueue(threadPool);
        free(task);
    }
    osDestroyQueue(threadPool->taskQueue);
    free(threadPool->threadList);
    int returnVal = pthread_mutex_destroy(&threadPool->taskQueueMutex);
    systemCallTest(returnVal);
    returnVal = pthread_mutex_destroy(&threadPool->condVariableMutex);
    systemCallTest(returnVal);
    returnVal = pthread_mutex_destroy(&destroyMutex);
    systemCallTest(returnVal);
    free(threadPool);
}

// Check if Malloc has worked
void allocationTest(void *tested) {
    if (tested == NULL) {
        ALLOCATION_ERROR_MSG;
        _exit(-1);
    }
}

// Check if system call has worked
void systemCallTest(int tested) {
    if (tested != 0) {
        SYSTEM_CALL_ERROR_MSG;
        _exit(-1);
    }
}