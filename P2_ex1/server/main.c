#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "common/constants.h"
#include "common/io.h"
#include "operations.h"
#include "buffer_prod_consumer.h"
#include "main_workers.h"

int op_code_1(Buffer_prod_consumer* buffer_prod_consumer, char* name_client_req, char* name_client_resp) {
  return write_buffer_prod_consumer(buffer_prod_consumer, name_client_req, name_client_resp);
}

void read_setup(int fserv, char *pipename, Buffer_prod_consumer* buffer_prod_consumer) {
  char op;
  ssize_t n;
  char name_client_req[40], name_client_resp[40];

  while (1) {
    n = read(fserv, &op, sizeof(char));

    if (n <= 0) {
      close(fserv);
      open(pipename, O_RDONLY);
      break;
    }

    if (n == -1) {
      fprintf(stderr, "Error: Read failed");
      exit(1);
    }

    switch (op) {
      case '1':
        read(fserv, name_client_req, sizeof(char)*40);
        read(fserv, name_client_resp, sizeof(char)*40);

        op_code_1(buffer_prod_consumer, name_client_req, name_client_resp);

        break;        
      
      default: /* error */
        fprintf(stderr, "Error: Invalid op code number\n");
    }
  }
}

int main(int argc, char* argv[]) { 
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  char *pipename = argv[1];
  printf("Starting IST-EMS server with pipe called %s\n", pipename);

  unlink(pipename);
  if (mkfifo(pipename, 0777) < 0) {
    fprintf(stderr, "Failed to create pipe\n");
    return 1;
  }

  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "Failed to initialize EMS (server)\n");
    return 1;
  }

  Buffer_prod_consumer* buffer_prod_consumer = create_buffer_prod_consumer();

  pthread_t t_works[S]; 
  inputs_task_W t_inputs[S];
  for (int i = 0; i < S; i++) {
    t_inputs[i].session_id = i;
    t_inputs[i].buffer_prod_consumer = buffer_prod_consumer;
    int host = pthread_create(&t_works[i], NULL, task_W, &t_inputs[i]);
    if (host) {
      fprintf(stderr, "ERROR PTHREAD_CREATE (main):\n");
      exit(1);
    }
  }
  
  int fserv;
  if ((fserv = open(pipename, O_RDONLY)) < 0) {
    fprintf(stderr, "Failed to open pipe\n");
    return 1;
  }

  while (1) {
    read_setup(fserv, pipename, buffer_prod_consumer); 
  }

  for (int i = 0; i < S; i++) {
    pthread_join(t_works[i], NULL);
  }

  delete_buffer_prod_consumer(buffer_prod_consumer);
  close(fserv);
  unlink(pipename);
  ems_terminate();
  return 0;
}

