#include <string.h>
#include <stdlib.h>

#include "buffer_prod_consumer.h"

Buffer_prod_consumer* create_buffer_prod_consumer() {
  Buffer_prod_consumer* buffer_prod_consumer = malloc(sizeof(Buffer_prod_consumer)*1);
  pthread_cond_init(&buffer_prod_consumer->cond_write, NULL);
  pthread_cond_init(&buffer_prod_consumer->cond_read, NULL);
  pthread_mutex_init(&buffer_prod_consumer->mutex_write, NULL);
  pthread_mutex_init(&buffer_prod_consumer->mutex_read, NULL);
  buffer_prod_consumer->index_read = 0;
  buffer_prod_consumer->index_write = 0;

  return buffer_prod_consumer;
}

int write_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer, char name_client_req[40], char name_client_resp[40]) {
  pthread_mutex_lock(&buffer_prod_consumer->mutex_write);
  while (buffer_prod_consumer->buf[buffer_prod_consumer->index_write] != NULL) {
    pthread_cond_wait(&buffer_prod_consumer->cond_write, &buffer_prod_consumer->mutex_write);
  }

  // Criar a mensagem para o buf_prod_consumer com as inf do cliente
  Register_msg* register_msg = malloc(sizeof(register_msg)*1);
  strncpy(register_msg->name_client_req, name_client_req, sizeof(char)*40);
  strncpy(register_msg->name_client_resp, name_client_resp, sizeof(char)*40);

  // Escrever a mensagem(criada em cima) no buf_prod_consumer
  buffer_prod_consumer->buf[buffer_prod_consumer->index_write] = register_msg;
  buffer_prod_consumer->index_write++;
  if(buffer_prod_consumer->index_write == S) {
    buffer_prod_consumer->index_write = 0;
  }

  pthread_cond_signal(&buffer_prod_consumer->cond_read);
  pthread_mutex_unlock(&buffer_prod_consumer->mutex_write);
  return 0;
}

Register_msg* read_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer) {
  pthread_mutex_lock(&buffer_prod_consumer->mutex_read);
  while (buffer_prod_consumer->buf[buffer_prod_consumer->index_read] == NULL) {
    pthread_cond_wait(&buffer_prod_consumer->cond_read, &buffer_prod_consumer->mutex_read);
  }

  Register_msg* msg = buffer_prod_consumer->buf[buffer_prod_consumer->index_read];
  buffer_prod_consumer->buf[buffer_prod_consumer->index_read] = NULL;
  buffer_prod_consumer->index_read++;
  if(buffer_prod_consumer->index_read == S) {
    buffer_prod_consumer->index_read = 0;
  }

  pthread_cond_signal(&buffer_prod_consumer->cond_write);
  pthread_mutex_unlock(&buffer_prod_consumer->mutex_read);
  return msg;
}

void delete_buffer_prod_consumer(Buffer_prod_consumer* buffer_prod_consumer) {
  free(buffer_prod_consumer);
}