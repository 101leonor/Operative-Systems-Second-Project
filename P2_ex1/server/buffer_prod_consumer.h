#ifndef BUFFER_PROD_CONSUMER_H
#define BUFFER_PROD_CONSUMER_H

#include <pthread.h>

#include "common/constants.h"

typedef struct {
  char name_client_req[40];
  char name_client_resp[40];
} Register_msg;

typedef struct {
  Register_msg* buf[S];
  int index_read;
  int index_write;
  pthread_cond_t cond_write;
  pthread_cond_t cond_read;
  pthread_mutex_t mutex_write;
  pthread_mutex_t mutex_read;
} Buffer_prod_consumer;

Buffer_prod_consumer* create_buffer_prod_consumer();
int write_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer, char name_client_req[40], char name_client_resp[40]);
Register_msg* read_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer);
void delete_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer);

#endif  // BUFFER_PROD_CONSUMER_H
