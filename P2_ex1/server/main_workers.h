#ifndef MAIN_WORKERS_H
#define MAIN_WORKERS_H

typedef struct {
    int session_id;
    Buffer_prod_consumer* buffer_prod_consumer;
} inputs_task_W;

void *task_W(void *argt);

#endif  // MAIN_WORKERS_H
