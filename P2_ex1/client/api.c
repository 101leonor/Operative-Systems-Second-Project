#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "api.h"
#include "../common/io.h"
#include "../common/common.h"

int session_id = 0;
int fserv = 0, fclient_req = 0, fclient_resp = 0;
const char *name_client_req, *name_client_resp;

int create_client_pipes(char const* req_pipe_path, char const* resp_pipe_path) {
  unlink(req_pipe_path);
  if (mkfifo(req_pipe_path, 0777) < 0) {
    fprintf(stderr, "Error creating request pipe\n");
    return 1;
  }

  unlink(resp_pipe_path);
  if (mkfifo(resp_pipe_path, 0777) < 0) {
    fprintf(stderr, "Error creating response pipe\n");
    return 1;
  }

  return 0;
}

int ems_setup(char const* req_pipe_path, char const* resp_pipe_path, char const* server_pipe_path) {
  create_client_pipes(req_pipe_path, resp_pipe_path);

  if ((fserv = open(server_pipe_path, O_WRONLY)) < 0) {
    fprintf(stderr, "Error opening server pipe\n");
    return 1;
  }

  size_t buffer_size = 1 + 40 + 40;
  char buffer[buffer_size];
  bzero(buffer, buffer_size);
  buffer[0] = '1';
  strncpy(buffer + 1, req_pipe_path, 40);
  strncpy(buffer + 1 + 40, resp_pipe_path, 40);

  write_all(fserv, buffer, buffer_size*sizeof(char));

  if ((fclient_resp = open(resp_pipe_path, O_RDONLY)) < 0) {
    return 1;
  }

  if ((fclient_req = open(req_pipe_path, O_WRONLY)) < 0) {
    return 1;
  }

  name_client_resp = resp_pipe_path;
  name_client_req = req_pipe_path;

  read_all(fclient_resp, &session_id, sizeof(int));
  printf("session_id: %d\n", session_id);

  return 0;
}

int ems_quit(void) { 
  write_all(fclient_req, "2", sizeof(char));

  read_all(fclient_resp, NULL, 0);

  close(fclient_req); 
  close(fclient_resp);  
  close(fserv);
  unlink(name_client_req);
  unlink(name_client_resp);

  return 0;
}

int ems_create(unsigned int event_id, size_t num_rows, size_t num_cols) {
  write_all(fclient_req, "3", sizeof(char));
  write_all(fclient_req, &event_id, sizeof(unsigned int));
  write_all(fclient_req, &num_rows, sizeof(size_t));
  write_all(fclient_req, &num_cols, sizeof(size_t));

  int output;
  read_all(fclient_resp, &output, sizeof(int));

  printf("OUTPUT_CREATE: %d\n", output);
  return output;
}

int ems_reserve(unsigned int event_id, size_t num_seats, size_t* xs, size_t* ys) {
  write_all(fclient_req, "4", sizeof(char));
  write_all(fclient_req, &event_id, sizeof(unsigned int));
  write_all(fclient_req, &num_seats, sizeof(size_t));
  write_all(fclient_req, xs, sizeof(size_t)*num_seats);
  write_all(fclient_req, ys, sizeof(size_t)*num_seats);

  int output;
  read_all(fclient_resp, &output, sizeof(int));

  printf("OUTPUT_RESERVE: %d\n", output);
  return output;
}

int ems_show(int out_fd, unsigned int event_id) {
  write_all(fclient_req, "5", sizeof(char));
  write_all(fclient_req, &event_id, sizeof(unsigned int));

  int output;
  size_t rows;
  size_t cols;
  read_all(fclient_resp, &output, sizeof(int));
  read_all(fclient_resp, &rows, sizeof(size_t));
  read_all(fclient_resp, &cols, sizeof(size_t));

  unsigned int* data = (unsigned int*) malloc(sizeof(unsigned int) * rows * cols);
  read_all(fclient_resp, data, sizeof(unsigned int) * rows * cols);

  for (size_t i = 0; i < rows; i++) {
    for (size_t j = 0; j < cols; j++) {
      char buffer[16];
      sprintf(buffer, "%u", data[i * cols + j]);

      if (print_str(out_fd, buffer)) {
        perror("Error writing to file descriptor");
        return 1;
      }

      if (j < cols - 1) {
        if (print_str(out_fd, " ")) {
          perror("Error writing to file descriptor");
          return 1;
        }
      }
    }

    if (print_str(out_fd, "\n")) {
      perror("Error writing to file descriptor");
      return 1;
    }
  }

  free(data);
  printf("OUTPUT_SHOW: %d\n", output);
  return output;
}

int ems_list_events(int out_fd) {
  write_all(fclient_req, "6", sizeof(char));

  int output;
  size_t num_events;
  read_all(fclient_resp, &output, sizeof(int));
  read_all(fclient_resp, &num_events, sizeof(size_t));

  printf("num events: %zu\n", num_events);
 
  if (num_events == 0) {
    char buff[] = "No events\n";
    if (print_str(out_fd, buff)) {
      perror("Error writing to file descriptor");
      return 1;
    }

    return 0;
  }

  unsigned int* ids = (unsigned int*) malloc(sizeof(unsigned int) * num_events);
  read_all(fclient_resp, ids, sizeof(unsigned int) * num_events);

  for(size_t i = 0; i < num_events; i++) {
    char buff[] = "Event: ";
    if (print_str(out_fd, buff)) {
      perror("Error writing to file descriptor");
      return 1;
    }

    char id[16];
    sprintf(id, "%u\n", ids[i]);
    if (print_str(out_fd, id)) {
      perror("Error writing to file descriptor");
      return 1;
    }

    // printf("ids: %u\n", ids[i]);

  }
  
  free(ids);
  return output;
}
