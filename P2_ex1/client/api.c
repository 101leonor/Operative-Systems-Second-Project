#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "api.h"

#define MSGSIZE 1000

int session_id = 0;
int fserv = 0, fclient_req = 0, fclient_resp = 0;
char *name_client_req, *name_client_resp;

int create_client_pipes(char const* req_pipe_path, char const* resp_pipe_path) {
  unlink(req_pipe_path);
  if (mkfifo(req_pipe_path, 0777) < 0) {
    return 1;
  }

  unlink(resp_pipe_path);
  if (mkfifo(resp_pipe_path, 0777) < 0) {
    return 1;
  }

  return 0;
}

// Create pipes and connect to the server
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  create_client_pipes(req_pipe_path, resp_pipe_path);

  if ((fserv = open(server_pipe_path, O_WRONLY)) < 0) {
    printf("Error: open failed\n");
    return 1;
  }

  char buffer[MSGSIZE];
  bzero(buffer, MSGSIZE);

  write(fserv, "1", sizeof(char));
  write(fserv, req_pipe_path, sizeof(char)*40);
  write(fserv, resp_pipe_path, sizeof(char)*40);

  if ((fclient_resp = open(resp_pipe_path, O_RDONLY)) < 0) {
    return 1;
  }

  if ((fclient_req = open(req_pipe_path, O_WRONLY)) < 0) {
    return 1;
  }

  char buf[MSGSIZE];
  bzero(buf, MSGSIZE); 

  read(fclient_resp, buf, MSGSIZE);

  printf("session_id: %s\n", buf);
  int s = atoi(buf); 
  session_id = s;
  name_client_req = req_pipe_path;
  name_client_resp = resp_pipe_path;

  return 0;
}

// Close pipes
int ems_quit(void) { 
  char buf[MSGSIZE];
  bzero(buf, MSGSIZE);
  
  write(fclient_req, "2", sizeof(char));

  // Nao vou ler nada
  read(fclient_resp, buf, MSGSIZE);

  close(fclient_req); 
  close(fclient_resp);  
  close(fserv);
  unlink(name_client_req);
  unlink(name_client_resp);

  return 0;
}

// Send create request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  write(fclient_req, "3", sizeof(char));
  write(fclient_req, &event_id, sizeof(unsigned int));
  write(fclient_req, &num_rows, sizeof(size_t));
  write(fclient_req, &num_cols, sizeof(size_t));

  int output;
  read(fclient_resp, &output, sizeof(int));

  printf("OUTPUT_CREATE: %d\n", output);
  return output;
}

// Send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  write(fclient_req, "4", sizeof(char));
  write(fclient_req, &event_id, sizeof(unsigned int));
  write(fclient_req, &num_seats, sizeof(size_t));
  write(fclient_req, &xs, sizeof(size_t));
  write(fclient_req, &ys, sizeof(size_t));

  int output;
  read(fclient_resp, &output, sizeof(int));

  printf("OUTPUT_RESERVE: %d\n", output);
  return output;
}

// Send show request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_show(int out_fd, unsigned int event_id) {
  write(fclient_req, "5", sizeof(char));
  write(fclient_req, &event_id, sizeof(unsigned int));
  write(fclient_req, &out_fd, sizeof(int));

  int output;
  read(fclient_resp, &output, sizeof(int));

  // TODO Resposta e (int) retorno (conforme código base) | (size_t) num_rows | (size_t) num_cols | (unsigned int[num_rows * num_cols]) seats
  printf("OUTPUT_SHOW: %d\n", output);
  return output;
}

// Send list request to the server (through the request pipe) and wait for the response (through the response pipe)
int ems_list_events(int out_fd) {
  write(fclient_req, "6", sizeof(char));
  write(fclient_req, &out_fd, sizeof(int));

  int output;
  read(fclient_resp, &output, sizeof(int));

  // TODO: Resposta e (int) retorno (conforme código base) | (size_t) num_events | (unsigned int[num_events]) ids
  printf("OUTPUT_LIST: %d\n", output);
  return output;
}
