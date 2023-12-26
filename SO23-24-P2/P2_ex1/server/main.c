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

#define MSGSIZE 1000

typedef struct {
  int pos;
  char *buf;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
} buffer_com;

buffer_com Buffer_Com[S];

int sessions[S];
int id[S];

void catchSignal(int s);
void *task_W(void *argt);

// Intialize server, create worker threads
int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  char *pipename = argv[1];
  printf("Starting IST-EMS server with pipe called %s\n", pipename);

  unlink(pipename);
  if (mkfifo (pipename, 0777) < 0) {
    return 1;
  }

  int fserv, i;
  if ((fserv = open(pipename, O_RDONLY)) < 0) {
    return 1;
  }

  for (i = 0; i < S; i++) {
    sessions[i] = -1;
    id[i] = i;
    pthread_cond_init(&Buffer_Com[i].cond, NULL);
    pthread_mutex_init(&Buffer_Com[i].mutex, NULL);
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
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

  pthread_t t_work[S]; 
  for(i = 0; i < S; i++) {
    int host = pthread_create(&t_work[i], NULL, task_W, (void*)&id[i]);
    if(host) {
      perror("ERROR PTHREAD_CREATE (main):");
      exit(1);
    }
  }
  
  int n, s; // e returnado no caso 1 (op = 1)
  int event_id;
  size_t num_rows, num_cols, num_seats;
  // size_t* xs, ys;
  char op;
  char name_client_req[40], name_client_resp[40];
  while (1) {
    n = read(fserv, &op, sizeof(char));

    // Escrever a mim proprio; fecha e abro em vez da verificacao min
    if (n <= 0) {
      break;
    }

    if(n == -1) {
      fprintf(stderr, "Error: read failed");
      exit(EXIT_FAILURE);
    }

    switch(op) {
      case '1':
        read(fserv, name_client_req, 40);
        read(fserv, name_client_resp, 40);
        Buffer_Com[s].buf = (char*)malloc(sizeof(char) + sizeof(char) + sizeof(char));
        sprintf(Buffer_Com[s].buf, "%c|%s|%s", op, name_client_req, name_client_resp);
        catchSignal(s);
        break;

      case '2': 
        Buffer_Com[s].buf = (char*)malloc(sizeof(char)); 
        sprintf(Buffer_Com[s].buf, "%c", op);
        catchSignal(s);
        break;

      case '3': 
        read(fserv, &event_id, sizeof(unsigned int));
        read(fserv, &num_rows, sizeof(size_t));
        read(fserv, &num_cols, sizeof(size_t));
        Buffer_Com[s].buf = (char*)malloc(sizeof(char) + sizeof(unsigned int) + sizeof(size_t) + sizeof(size_t));
        sprintf(Buffer_Com[s].buf, "%c|%u|%zu|%zu", op, event_id, num_rows, num_cols); // memcpy
        catchSignal(s);
        break;

      case '4':
        read(fserv, &event_id, sizeof(int));
        read(fserv, &num_seats, sizeof(size_t));
        // read(fserv, &xs, sizeof(size_t));
        // read(fserv, &ys, sizeof(size_t));
        Buffer_Com[s].buf = (char*)malloc(sizeof(char) + sizeof(unsigned int) + sizeof(size_t)); //  sizeof(size_t) + sizeof(size_t)
        sprintf(Buffer_Com[s].buf, "%c|%u|%zu", op, event_id, num_seats); // xs, ys
        // TODO: for (|%zu|%zu) snprintf depois
        for(i = 0; i < num_seats; i++) {
          // snprintf?
        }
        catchSignal(s);
        break;

      case '5':
        read(fserv, &event_id, sizeof(int));
        Buffer_Com[s].buf = (char*)malloc(sizeof(char) + sizeof(unsigned int));
        sprintf(Buffer_Com[s].buf, "%c|%u", op, event_id);
        catchSignal(s);
        break;
      
      case '6':
        Buffer_Com[s].buf = (char*)malloc(sizeof(char));
        sprintf(Buffer_Com[s].buf, "%c", op);
        catchSignal(s);
        break;
      
      default: /* error */
        perror("Error: Invalid op code number\n");
    }
  }

  for(i = 0; i < S; i++) {
    pthread_join(t_work[i], NULL);
  }

  close(fserv);
  unlink(pipename);
  ems_terminate();

  return 0;
}

void catchSignal(int s) {
  pthread_mutex_lock(&Buffer_Com[s].mutex);
  pthread_cond_signal(&Buffer_Com[s].cond);
  pthread_mutex_unlock(&Buffer_Com[s].mutex);
}

void *task_W(void *argt) {
  int t = *((int*)argt);
  for(;;) {

    pthread_mutex_lock(&Buffer_Com[t].mutex);
    while(Buffer_Com[t].buf == NULL) {
      pthread_cond_wait(&Buffer_Com[t].cond, &Buffer_Com[t].mutex);
    }
    pthread_mutex_unlock(&Buffer_Com[t].mutex);

    // treatOP(Buffer_Com[t].buf);

    free(Buffer_Com[t].buf);
    Buffer_Com[t].buf = NULL;
  }

  return NULL;
}

// void treatOP(char *buf) {}