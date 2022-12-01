//
// Created by du on 11/6/22.
//

#include "threadpool.h"



ThreadPool* threadPollCreate(int min, int max, int queueSize) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    do{
        if (pool == NULL) {
            printf("malloc threadPool fail.. \n");
            break;
        }
        pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * max);
        if (pool->threadIDs == NULL) {
            printf("malloc threadIDs fail.. \n");
            break;
        }
        memset(pool->threadIDs, 0, sizeof(pthread_t) * max); // init the poll threadIDs to 0 then it not been used
        pool->minNum = min;
        pool->maxNum = max;
        pool->busyNum = 0;
        pool->liveNum = min; // equal to the min number
        pool->exitNum = 0;
        pool->tcpNum = 0;
        pool->udpNum = 0;

        if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
            pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
            pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
            pthread_cond_init(&pool->notFull, NULL) != 0
                ) {
            printf("mutex init fail.. \n");
            break;
        }

        // init task queue
        pool->taskQ = (Task*)malloc(sizeof(Task) * queueSize);
        pool->queueCapacity =  queueSize;
        pool->queueSize = 0;
        pool->queueFront = 0;
        pool->queueRear = 0;
        pool->shutdown = 0;// should not shut down


        // init manager
        pthread_create(&pool->managerID, NULL, manager, pool);

        for (int i = 0; i < min; ++i) {
            pthread_create(&pool->threadIDs[i], NULL, worker, pool);
        }
        return pool;
    }while(false);

    // free the resource if not success
    if (pool && pool->threadIDs) free(pool->threadIDs);
    if (pool && pool->taskQ) free(pool->taskQ);
    if (pool) free(pool);
    return NULL;
}


void * worker(void * arg) {
    ThreadPool* pool  = (ThreadPool*)arg;
    while(true) {
        pthread_mutex_lock(&pool->mutexPool);

        // to check if the task queue is empty
        while(pool->queueSize == 0 && !pool->shutdown) {
            // block the worker thread
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // check if we need to destroy thread
            if (pool->exitNum > 0) {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum) {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    threadExit(pool);
                }
            }
        }

        // to check if the threadPool has been shut down
        if (pool->shutdown){
            pthread_mutex_unlock(&pool->mutexPool);
            threadExit(pool);
        }

        // do consumer operation
        // get a task from task queue front
        Task task;
        task.function = pool->taskQ[pool->queueFront].function;
        task.arg = pool->taskQ[pool->queueFront].arg;

        // move the front position and modify the queue current size
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
        pool->queueSize--;

        // wake up the producer
        pthread_cond_signal(&pool->notFull);

        pthread_mutex_unlock(&pool->mutexPool);


        printf("thread %ld start working\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexBusy);

        task.function(task.arg);
        if (task.arg != NULL) {
//            free(task.arg);
        }
        task.arg = NULL;
        printf("thread %ld end working\n", pthread_self());
//        (*task.function)(task.arg);
        // complete the function operation and delete the busy num
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexBusy);

    }
    return NULL;
}

void * manager(void* arg) {
    ThreadPool* pool  = (ThreadPool*)arg;
    clock_t previous_time = clock();
    clock_t current_time;
    while(!pool->shutdown) {
//        printf("manager!\n");
        // get the number of the thread poll tasks number and threads number
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->queueSize;
        int liveNum = pool->liveNum;
        pthread_mutex_unlock(&pool->mutexPool);

        // get the number of busy thread
        pthread_mutex_lock(&pool->mutexBusy);
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexBusy);

        // add the thread
        // if the queue task number > alive thread number && alive thread number < max threads
        // double the thread alive number

        if (queueSize > (liveNum - busyNum) && liveNum < pool->maxNum) {
//            printf("go to manager add!\n");
            int counter = 0;
            if (liveNum == 0) liveNum = 1;
            pthread_mutex_lock(&pool->mutexPool);
            for (int i = 0; i < pool->maxNum &&
            counter < liveNum && liveNum < pool->maxNum ; ++i) {
                if (pool->threadIDs[i] == 0) {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        current_time = clock();
        if ((current_time - previous_time) / CLOCKS_PER_SEC >= 60) {
            previous_time = current_time;
           // printf("3 s\n");
            // destroy the thread, kill 2 threads every time
            // if busyNum * 2  < alive number && alive number > min
            // destroy the half live num
            if (busyNum * 2 < liveNum && liveNum > pool->minNum) {
                pthread_mutex_lock(&pool->mutexPool);
                pool->exitNum = liveNum / 2;
                pthread_mutex_unlock(&pool->mutexPool);
                // make alive thread die kill 2 threads
                for (int i = 0; i < liveNum / 2; ++i) {
                    pthread_cond_signal(&pool->notEmpty);
                }
//                pthread_mutex_lock(&pool->mutexPool);
//                //printf("current live number %d\n", pool->liveNum);
//                pthread_mutex_unlock(&pool->mutexPool);
            }
        }

        // every 30 seconds check once
//        sleep(30);
    }
    return NULL;
}

// write the pool->threadID to 0
void threadExit(ThreadPool * pool) {

    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxNum ; ++i) {
        if (pool->threadIDs[i] == tid) {
            pool->threadIDs[i] = 0;
            printf("threadExit() called , %ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}


void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg){
    pthread_mutex_lock(&pool->mutexPool);
    while (pool->queueSize == pool->queueCapacity && !pool->shutdown) {
//        printf("should block!\n");
        // block the product thread
        pthread_cond_wait(&pool->notFull, &pool->mutexPool);

    }
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutexPool);
        return;
    }

    // add task
//    printf("add tasks\n");
    pool->taskQ[pool->queueRear].function = func;
    pool->taskQ[pool->queueRear].arg = arg;
    pool->queueRear = (pool->queueRear + 1 ) % pool->queueCapacity;
    pool->queueSize++;
//    printf("now queueSize is %d\n",pool->queueSize);
    // wake the consumer/worker
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->mutexPool);
}

int threadPoolBusyNumber(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexBusy);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->mutexBusy);
    return busyNum;
}

int threadPoolAliveNumber(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    int liveNum = pool->liveNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return liveNum;
}



int threadPoolDestroy(ThreadPool* pool){
    printf("destroy the pool\n");
    if (pool == NULL) {
        return -1;
    }
    // close the pool
    pool->shutdown = 1;

    // recycle the manager thread
    pthread_join(pool->managerID, NULL);

    //wake up the consumer threads
    for (int i = 0; i < pool->liveNum; ++i) {
        pthread_cond_signal(&pool->notEmpty);
    }
    // free the heap
    if (pool->taskQ) {
        free(pool->taskQ);
    }
    if (pool->threadIDs) {
        free(pool->threadIDs);
    }
    pthread_mutex_destroy(&pool->mutexPool);
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
    free(pool);
    return 1;

}


int getTcpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    int tcpNum = pool->tcpNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return tcpNum;
}

int getUdpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    int udpNum = pool->udpNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return udpNum;
}

void addTcpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    pool->tcpNum++;
    pthread_mutex_unlock(&pool->mutexPool);
}

void addUdpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    pool->udpNum++;
    pthread_mutex_unlock(&pool->mutexPool);
}

void deleteTcpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    pool->tcpNum--;
    pthread_mutex_unlock(&pool->mutexPool);
}

void deleteUdpNum(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutexPool);
    pool->udpNum--;
    pthread_mutex_unlock(&pool->mutexPool);
}




