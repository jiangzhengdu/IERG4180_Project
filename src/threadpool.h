//
// Created by du on 11/6/22.
//

#ifndef IERG4180_PROJECT3_THREADPOOL_H
#define IERG4180_PROJECT3_THREADPOOL_H


#include <pthread.h>
#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// task struct
typedef struct Task {
    void (*function) (void* arg);
    void* arg;
}Task;

// threadPoll struct
typedef struct ThreadPool {
    Task * taskQ;
    int queueCapacity; // the queue capacity
    int queueSize; // current stored tasks number
    int queueFront; // the queue front -> retrieve data
    int queueRear; // queue rear -> put data

    pthread_t managerID; // manager thread id
    pthread_t *threadIDs; // workers thread ids
    int minNum; // min thread num
    int maxNum; // max thread num
    int busyNum; // busy number should (add extra lock
    int liveNum;  // live thread num
    int exitNum; // number of thread should be killed
    int tcpNum; // tcp Num
    int udpNum; // udp Num
    pthread_mutex_t mutexPool; // the lock for entire thread pool
    pthread_mutex_t mutexBusy; // the lock for busy number
    pthread_cond_t notFull; // the task queue is full
    pthread_cond_t notEmpty; // the task queue is empty

    int shutdown; // whether destroy the thread poll 1 : should 0 : not
}ThreadPool;

// create threadPoll and init
ThreadPool* threadPollCreate(int min, int max, int queueSize);

// destroy the thread poll
int threadPoolDestroy(ThreadPool* pool);

// get threadPoll thread busy number
int threadPoolBusyNumber(ThreadPool* pool);

// add task for ThreadPool
void threadPoolAdd(ThreadPool* pool, void(*func)(void*), void* arg);

// get threadPool alive thread number
int threadPoolAliveNumber(ThreadPool* pool);

void * worker(void* arg);

void * manager(void* arg);

void threadExit(ThreadPool * pool);

int getTcpNum(ThreadPool* pool);

int getUdpNum(ThreadPool* pool);

void addTcpNum(ThreadPool* pool);

void addUdpNum(ThreadPool* pool);

void deleteTcpNum(ThreadPool* pool);

void deleteUdpNum(ThreadPool* pool);

#endif //IERG4180_PROJECT3_THREADPOOL_H




