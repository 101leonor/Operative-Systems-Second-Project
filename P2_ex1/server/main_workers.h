#ifndef MAIN_WORKERS_H
#define MAIN_WORKERS_H

typedef struct {
    int session_id;
    Buffer_prod_consumer* buffer_prod_consumer;
} inputs_task_W;

void *task_W(void *argt);
void process_client(int fcli_req, int fcli_resp);
int treat_OP(int fcli_req, int fcli_resp);
int op_code_2(); 
int op_code_3(int fcli_req, int fcli_resp);
int op_code_4(int fcli_req, int fcli_resp);
int op_code_5(int fcli_req, int fcli_resp);
int op_code_6(int fcli_resp);

#endif  // MAIN_WORKERS_H
