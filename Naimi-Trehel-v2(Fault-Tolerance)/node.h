#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

// Enumeração de estados possíveis de um node.
typedef enum nodeState {
   rest,
   waiting,
   active,
   consulting,
   candidate,
   observer,
   query
} s_NS;

// Estrutura de dados de um int array dinâmico.
typedef struct intArray {
   int *array;
   int arrayLength;
} s_IA;

// Estrutura de dados de um node.
typedef struct node {
   int self; // Indica o id deste node. (a.k.a. me)
   int father; // Indica o provável atual TOKEN OWNER. (a.k.a. owner, last) 
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool requestingCS; // Indica se este node está requisitando ou não acesso à CRITICAL SECTION.
   bool tokenPresent; // Indica se este node detém ou não o TOKEN.
   s_IA *x; // Indica o conjunto de nodes da rede.
   s_IA *xc; // Indica o conjunto de nodes que me enviaram a mensagem "failure".
   s_NS myState; // Indica o estado atual do node.
   bool printingEvents; // Indica se este node irá imprimir ou não eventos de solicitação e acesso à CRITICAL SECTION.
   bool loggingEvents; // Indica se este node irá gerar ou não um log de eventos de solicitação e acesso à CRITICAL SECTION.
   MPI_File *logFile; // Indica o arquivo MPI_File associado ao log de eventos dos nodes.
   char *logBuffer; // Indica o buffer associado ao log de eventos dos nodes.
   double requestedTokenTime; // Indica o início do wall-clock que contabiliza o tempo de espera para este node receber o TOKEN.
   double receivedTokenTime; // Indica o fim do wall-clock que contabiliza o tempo de espera para este node receber o TOKEN.
   bool failed; // Indica se este node falhou.
} s_N;

s_N *initialize_node(void);

s_N *create_node(int nodeRank, int nodeCount, bool printingEvents, bool loggingEvents);

s_IA *load_x_set_node(int nodeRank, int nodeCount);

s_IA *load_xc_set_node();

bool isContainedInSet(s_IA *set, int element);

char *stateToString(s_N *node);

char *tagToString(int TAG_MPI_MESSAGE);

char *setToString(s_IA *set);

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void perform_c_s(s_N *node);

void send_broadcast_message(s_N *node, int TAG_MPI_MESSAGE);

void received_timeout_signal(size_t timerId, void *userData);

void request_c_s(s_N *node);

void release_c_s(s_N *node);

void received_request_message(s_N *node, int nodeSj);

void received_token_message(s_N *node);

void received_consult_message(s_N *node, int nodeSj);

void received_quiet_message(s_N *node, int nodeSj);

void received_failure_message(s_N *node, int nodeSj);

void received_election_message(s_N *node, int nodeSj);

void received_present_message(s_N *node, int nodeSj);

void received_candidate_elected_message(s_N *node, int nodeSj);
