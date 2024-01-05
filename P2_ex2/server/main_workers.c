#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "buffer_prod_consumer.h"
#include "operations.h"
#include "main_workers.h"
#include "../common/common.h"

void *task_W(void *argt) {
  inputs_task_W* inputs = ((inputs_task_W*)argt);
	Buffer_prod_consumer* buffer_prod_consumer = inputs->buffer_prod_consumer;
  int session_id = inputs->session_id;

  for (;;) {
    Register_msg* setup_msg = read_buffer_prod_consumer(buffer_prod_consumer);

    int fcli_resp;
    if ((fcli_resp = open(setup_msg->name_client_resp, O_WRONLY)) < 0) {
      exit(1);
    }

    int fcli_req;
    if ((fcli_req = open(setup_msg->name_client_req, O_RDONLY)) < 0) {
      exit(1);
    }

    write_all(fcli_resp, &session_id, sizeof(int));

		process_client(fcli_req, fcli_resp);

    free(setup_msg);
    close(fcli_req);
    close(fcli_resp);
  }

  pthread_exit(NULL);
  return NULL;
}

void process_client(int fcli_req, int fcli_resp) {
  while(1) {
    int op_code = treat_OP(fcli_req, fcli_resp);
    if(op_code == 2) {
      fprintf(stdout, "Client asked to quit\n");
      return;
    }
    if(op_code == 1) {
      fprintf(stderr, "Error in treat_OP occured\n");
      return;
    }
  }
}

int treat_OP(int fcli_req, int fcli_resp) {
  char op;
  ssize_t numTokens = read_all(fcli_req, &op, sizeof(char)*1);

  if(numTokens < 0) {
    fprintf(stderr, "Unable to read op code\n");
    return 1;
  }

  switch(op) {
    case '2':
      return op_code_2(); // ems_quit
    case '3':
      return op_code_3(fcli_req, fcli_resp); // ems_create
    case '4':
      return op_code_4(fcli_req, fcli_resp); // ems_reserve 
    case '5':
      return op_code_5(fcli_req, fcli_resp); // ems_show 
    case '6':
      return op_code_6(fcli_resp); // ems_list_events
    default:
      fprintf(stderr, "Unknown op_code\n");
      return 1;
  }
}

/* ---------------------------------------------------- OP_CODES ---------------------------------------------------- */

int op_code_2() {
  return 2;
}

int op_code_3(int fcli_req, int fcli_resp) {  
  unsigned int event_id;
  size_t num_rows; 
  size_t num_cols;

  read_all(fcli_req, &event_id, sizeof(unsigned int));
  read_all(fcli_req, &num_rows, sizeof(size_t));
  read_all(fcli_req, &num_cols, sizeof(size_t));
  
  int file = ems_create(event_id, num_rows, num_cols);

  write_all(fcli_resp, &file, sizeof(int));

  return 3;
}

int op_code_4(int fcli_req, int fcli_resp) {
  unsigned int event_id;
  size_t num_seats;

  read_all(fcli_req, &event_id, sizeof(unsigned int));
  read_all(fcli_req, &num_seats, sizeof(size_t));

  size_t* xs = (size_t*) malloc(sizeof(size_t) * num_seats);
  size_t* ys = (size_t*) malloc(sizeof(size_t) * num_seats);

  read_all(fcli_req, xs, sizeof(size_t) * num_seats);
  read_all(fcli_req, ys, sizeof(size_t) * num_seats);

  int file = ems_reserve(event_id, num_seats, xs, ys);

  write_all(fcli_resp, &file, sizeof(int));

  free(xs);
  free(ys);
  return 4;
}

int op_code_5(int fcli_req, int fcli_resp) {
  unsigned int event_id;

  read_all(fcli_req, &event_id, sizeof(unsigned int));
  
  size_t rows;
  size_t cols;
  unsigned int* data;
  int file = ems_show(event_id, &rows, &cols, &data);

  write_all(fcli_resp, &file, sizeof(int));
  write_all(fcli_resp, &rows, sizeof(size_t));
  write_all(fcli_resp, &cols, sizeof(size_t));
  write_all(fcli_resp, data, sizeof(unsigned int) * rows * cols);

  return 5;
}

int op_code_6(int fcli_resp) {
  size_t num_events;
  unsigned int* ids;
  int file = ems_list_events(&num_events, &ids);
  
  write_all(fcli_resp, &file, sizeof(int));
  write_all(fcli_resp, &num_events, sizeof(size_t));

  if (num_events == 0) {
    printf("No events\n");
  }

  if (num_events > 0) {
    write_all(fcli_resp, ids, sizeof(unsigned int) * num_events);
    free(ids);
  }
  
  return 6;
}