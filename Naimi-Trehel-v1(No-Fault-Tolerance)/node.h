#include <stdbool.h>

// Estrutura de dados de um node.
typedef struct node {
   int self; // Indica o id deste node. (a.k.a. me)
   int last; // Indica o provável atual TOKEN OWNER. (a.k.a. owner, father) 
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool tokenPresent; // Indica se este node detém ou não o TOKEN.
   bool requestingCS; // Indica se este node está requisitando ou não acesso à CRITICAL SECTION.
} s_N;

s_N *initialize_node(void);

s_N *create_node(int nodeRank);

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void perform_cs(s_N *node);

void request_cs(s_N *node, int nodeCount);

void release_cs(s_N *node);

void receive_request_cs(s_N *node, int requestingNode);

void receive_token(s_N *node);
