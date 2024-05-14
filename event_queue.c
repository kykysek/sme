#include <stdbool.h>
#include <stdlib.h>

#include <pthread.h>

#include "event_queue.h"

#ifndef QUEUE_CAPACITY
#define QUEUE_CAPACITY 32
#endif

typedef struct {
    event queue[QUEUE_CAPACITY];
    int in;//head
    int out;//tail
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    bool quit;
} queue;

static queue q = { .in = 0, .out = 0};
//inicializace mutexu (aby se necetlo 2 vlaknama ve stejnej moment)
void queue_init(void)
{
    pthread_mutex_init(&(q.mtx), NULL);
    pthread_cond_init(&(q.cond), NULL);
}
//vymaze queue a free-ne pamet
void queue_cleanup(void)
{
    while (q.in != q.out) {
        event ev = queue_pop();
        if (ev.data.msg) {
            free(ev.data.msg);
        }
    }
}

event queue_pop(void)
{
    event ev = { .type = EV_TYPE_NUM };
    pthread_mutex_lock(&(q.mtx));//zakazat  cteni 2 vlaknama najednou - 1 vlakno ho lockne, jiny tam nemuzou
    while (!q.quit && q.in == q.out) {//fronta je prazdna -> cekame az tam neco prijde nebo na quit
        pthread_cond_wait(&(q.cond), &(q.mtx));//da signal ze fronta neni prazdna
    }
    if (q.in != q.out) {
        ev = q.queue[q.out]; // v podstate q.queue[tail]
        q.out = (q.out + 1) % QUEUE_CAPACITY;//novy tail
        pthread_cond_broadcast(&(q.cond)); // vysle signal ostatnim vlaknum ze zas muzou cist
    }
    pthread_mutex_unlock(&(q.mtx));//cteni dokonceno -> unlockneme pro dalsi vlakna
    return ev;
}

void queue_push(event ev)
{
    pthread_mutex_lock(&(q.mtx));
    while (((q.in + 1) % QUEUE_CAPACITY) == q.out) { 
        pthread_cond_wait(&(q.cond), &(q.mtx));
    }
    q.queue[q.in] = ev;
    q.in = (q.in + 1) % QUEUE_CAPACITY;
    pthread_cond_broadcast(&(q.cond));
    pthread_mutex_unlock(&(q.mtx));
}

bool is_quit ()
{
    bool quit;
    pthread_mutex_lock(&(q.mtx)); 
    quit = q.quit;
    pthread_mutex_unlock(&(q.mtx));
    return quit;
}

void set_quit ()
{
   pthread_mutex_lock(&(q.mtx));
   q.quit = true;
   pthread_mutex_unlock(&(q.mtx));
}