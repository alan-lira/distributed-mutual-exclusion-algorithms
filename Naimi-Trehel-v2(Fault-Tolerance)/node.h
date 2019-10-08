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
   int last; // Indica o provável atual TOKEN OWNER. (a.k.a. owner, father) 
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool tokenPresent; // Indica se este node detém ou não o TOKEN.
   bool requestingCS; // Indica se este node está requisitando ou não acesso à CRITICAL SECTION.
   s_IA *x; // Indica o conjunto de nodes da rede.
   s_IA *xc; // Indica o conjunto de nodes que me enviaram a mensagem "failure".
   s_NS myState; // Indica o estado atual do node.
} s_N;

s_N *initialize_node(void);

s_N *create_node(int nodeRank, int nodeCount);

s_IA *load_x_set_node(int nodeRank, int nodeCount);

s_IA *load_xc_set_node();

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void perform_c_s(s_N *node);

void send_broadcast_message(s_N *node, int TAG_MPI_MESSAGE);

void received_timeout_signal(s_N *node);

void request_c_s(s_N *node);

void release_c_s(s_N *node);

void received_request_message(s_N *node, int requestingNode);

void received_token_message(s_N *node);

void received_consult_message(s_N *node, int requestingNode);

void received_quiet_message(s_N *node, int requestingNode);

void received_failure_message(s_N *node, int requestingNode);

void received_election_message(s_N *node, int requestingNode);

void received_present_message(s_N *node, int requestingNode);
