#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

// Estrutura de dados de um node.
typedef struct node {
   int self; // Indica o id deste node. (a.k.a. me)
   int father; // Indica o provável atual TOKEN OWNER. (a.k.a. owner, last) 
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool requestingCS; // Indica se este node está requisitando ou não acesso à CRITICAL SECTION.
   bool tokenPresent; // Indica se este node detém ou não o TOKEN.
   bool printingEvents; // Indica se este node irá imprimir ou não eventos de solicitação e acesso à CRITICAL SECTION.
   bool loggingEvents; // Indica se este node irá gerar ou não um log de eventos de solicitação e acesso à CRITICAL SECTION.
   MPI_File *logFile; // Indica o arquivo MPI_File associado ao log de eventos dos nodes.
   char *logBuffer; // Indica o buffer associado ao log de eventos dos nodes.
   double requestedTokenTime; // Indica o tempo cronológico no qual este node solicitou o TOKEN.
   double receivedTokenTime; // Indica o tempo cronológico no qual este node recebeu o TOKEN.
} s_N;

s_N *initialize_node(void);

s_N *create_node(int nodeRank, bool printingEvents, bool loggingEvents);

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void perform_c_s(s_N *node);

void request_c_s(s_N *node);

void release_c_s(s_N *node);

void received_request_message(s_N *node, int requestingNode);

void received_token_message(s_N *node);
