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
char *name_client;
char buf[MSGSIZE];

// Create pipes and connect to the server
int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {

  unlink(req_pipe_path);
  if (mkfifo (req_pipe_path, 0777) < 0) {
    return 1;
  }

  unlink(resp_pipe_path);
  if (mkfifo (resp_pipe_path, 0777) < 0) {
    return 1;
  }

  if ((fserv = open(server_pipe_path, O_WRONLY)) < 0) {
    return 1;
  }

  bzero(buf, MSGSIZE);

  char *buffer = (char *)malloc(81);
  if (buffer == NULL) {
    return 1;
  }

  size_t offset = 0;

  write(fserv, "1", sizeof(char));
  memcpy(buffer + offset, "1", sizeof(char));
  offset += sizeof(char);

  write(fserv, req_pipe_path, 40);
  memcpy(buffer + offset, req_pipe_path, 40);
  offset += 40;

  write(fserv, resp_pipe_path, 40);
  memcpy(buffer + offset, resp_pipe_path, 40);
  // DUVIDA: offset += 40; 

  if ((fclient_req = open(req_pipe_path, O_RDONLY)) < 0) {
    return 1;
  }

  if ((fclient_resp = open(resp_pipe_path, O_RDONLY)) < 0) {
    return 1;
  }

  read(fclient_req, buffer, MSGSIZE);
  read(fclient_resp, buffer, MSGSIZE);

  int s = atoi(buffer);
  if (s == -1) {
    return 1;
  }
  session_id = s;
  name_client = req_pipe_path; // DUVIDA: e o resp_pipe_path?

  printf("CHEGOU AO CLIENTE\n");

  free(buffer);
  return 0; // DUVIDA: Como returnar o session id? return s? 
}

// Close pipes
int ems_quit(void) { 
  bzero(buf, MSGSIZE);
  
  write (fserv, "2", sizeof(char));

  read(fclient_req, buf, MSGSIZE);
  read(fclient_resp, buf, MSGSIZE);

  int s = atoi(buf);
  if (s == 0) {
    close (fclient_req); 
    close (fclient_resp); 
    close (fserv);
    unlink(name_client);
  }

  return s;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  //TODO: send create request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  //TODO: send reserve request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_show(int out_fd, unsigned int event_id) {
  //TODO: send show request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}

int ems_list_events(int out_fd) {
  //TODO: send list request to the server (through the request pipe) and wait for the response (through the response pipe)
  return 1;
}
