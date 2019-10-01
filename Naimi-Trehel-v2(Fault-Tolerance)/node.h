#include <stdbool.h>

typedef enum state {rest, waiting, active, consulting, candidate, observer, query} state; //Enumeração de estados possíveis de um node.

// Estrutura de dados de um int array dinâmico.
typedef struct intArray {
   int *array;
   int arrayLength;
}s_IA;

// Estrutura de dados de um node.
typedef struct node {
   int self; // Indica o id deste node. (a.k.a. me)
   int last; // Indica o provável atual TOKEN OWNER. (a.k.a. owner, father) 
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool tokenPresent; // Indica se este node detém ou não o TOKEN.
   bool requestingCS; // Indica se este node está requisitando ou não acesso à CRITICAL SECTION.
   s_IA *xc; // Indica o conjunto de nodes que enviaram mensagem "failure".
   s_IA *x; // Indica o conjunto de nodes da rede.
   state myState; // Indica o estado em que o node se encontra.
}s_N;

s_N *initialize_node(void);

s_N *create_node(int nodeRank, int nodeCount);

s_IA *load_x_set_node(int nodeRank, int nodeCount);

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void perform_cs(s_N *node);

void broadcast_message(s_N *node, int TAG_MPI_MESSAGE);

void request_cs(s_N *node, int nodeCount);

void release_cs(s_N *node);

void receive_request_cs(s_N *node, int requestingNode);

void receive_token(s_N *node);
