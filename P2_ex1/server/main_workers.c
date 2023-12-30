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

#define MSGSIZE 1000

void *task_W(void *argt);
int treat_OP(int fcli_req, int fcli_resp);
void process_client(int fcli_req, int fcli_resp);
int op_code_2(); 
int op_code_3(int fcli_req, int fcli_resp);
int op_code_4(int fcli_req, int fcli_resp);
int op_code_5(int fcli_req, int fcli_resp);
int op_code_6(int fcli_resp);

void *task_W(void *argt) {
  inputs_task_W* inputs = ((inputs_task_W*)argt);
	Buffer_prod_consumer* buffer_prod_consumer = inputs->buffer_prod_consumer;
  int session_id = inputs->session_id;

  for (;;) {
    // 1. Ler pedido sessao do cliente(pedido setup) do buffer_prod_consumer
    Register_msg* setup_msg = read_buffer_prod_consumer(buffer_prod_consumer);
    // 1.1 Este pedido tem 2 inf, o req e o resp
    int fcli_req, fcli_resp;
    if ((fcli_resp = open(setup_msg->name_client_resp, O_WRONLY)) < 0) {
      exit(1);
    }

    if ((fcli_req = open(setup_msg->name_client_req, O_RDONLY)) < 0) {
      exit(1);
    }

		printf("Comecei o cliente\n");
    // 2. A partir de agora so comunico com o cliente atraves do req e resp, nao volto a tocar no buf_com
    // 2.1 Envio ao cliente o seu id sessao(atraves do resp)
    char buf[MSGSIZE];
    snprintf(buf, sizeof(int), "%d",session_id);
    write(fcli_resp, buf, sizeof(int));

    // 2.2 A sessao esta estabelecida, entro no seguinto loop
		process_client(fcli_req, fcli_resp);
    //     2.2.1 Ler(espero) novo pedido do cliente (req)
    //     2.2.2 Processo pedido recebido (treatOP)
    //     2.2.3 Prepara resposta para cliente e envia ao cliente (resp)
    //     2.2.4 Volto ao passo 2.2.1 
    // 2.3 O loop de 2.2(sessao) termina quando o cliente pede(quit) ou quando ...
    // 2.4 Apos terminar volto ao ponto 1.
    free(setup_msg);
    close(fcli_req);
    close(fcli_resp);
		printf("Terminei o cliente\n");
  }

  return NULL;
}

int treat_OP(int fcli_req, int fcli_resp) {
  char op;
  printf("TREAT OP\n");
  int numTokens = read(fcli_req, &op, sizeof(char));
  printf("DEPOIS READ\n");

  if(numTokens < 0) {
    fprintf(stderr, "Unable to read op code\n");
    return 1;
  }

  printf("SWITCH TREAT OP\n");
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
  }
  return 1;
}

void process_client(int fcli_req, int fcli_resp) {
  // 2.2 A sessao esta estabelecida, entro no seguinto loop
  //     2.2.1 Ler(espero) novo pedido do cliente (req)
  //     2.2.2 Processo pedido recebido (treatOP)
  //     2.2.3 Prepara resposta para cliente e envia ao cliente (resp)
  //     2.2.4 Volto ao passo 2.2.1 
  // 2.3 O loop de 2.2(sessao) termina quando o cliente pede(quit)
  printf("PROC CLIENT\n");
  while(1) {
    int op_code = treat_OP(fcli_req, fcli_resp);
    printf("SAIU  TREAT OP\n");
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

/* ---------------------------------------------------------------------------------------------------------------------------------------*/
// A partir daqui ja nao e preciso do session_id! PS: Nunca foi preciso, ajuda so no debugging
// o OP_CODE é seguido do session_id da sessão atual do cliente => adicionar o session_id como arg em cada op_code?

int op_code_2() {
  return 2;
}

int op_code_3(int fcli_req, int fcli_resp) {  
  printf("COMECAR OP3\n");
  // 2.2.1 Ler o resto do pedido( - op) (req)
  unsigned int event_id;
  size_t num_rows; 
  size_t num_cols;

  read(fcli_req, &event_id, sizeof(unsigned int));
  read(fcli_req, &num_rows, sizeof(size_t));
  read(fcli_req, &num_cols, sizeof(size_t));
  
  // 2.2.2 Processo pedido recebido 
  int file = ems_create(event_id, num_rows, num_cols);
  printf("FILE: %d\n", file);
  printf("VOU FAZER WFRFITE\n");
  // 2.2.3 Prepara resposta para cliente e envia ao cliente (resp)
  write(fcli_resp, &file, sizeof(int));
  return 3;
}

int op_code_4(int fcli_req, int fcli_resp) {
  // unsigned int event_id;
  // size_t num_seats;
  // size_t *xs;
  // size_t *ys;


  
  // int file = ems_reserve(event_id, num_seats, xs, ys);
  // sprintf(buf, "%d", file);
  // write(fcli_resp, buf, MSGSIZE);

	printf("Not implemented\n");
  return 4;
}

int op_code_5(int fcli_req, int fcli_resp) {
	printf("Not implemented\n");
  return 5;
}

int op_code_6(int fcli_resp) {
	printf("Not implemented\n");
  return 6;
}