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

void perform_c_s(s_N *node);

void request_c_s(s_N *node);

void release_c_s(s_N *node);

void received_request_message(s_N *node, int requestingNode);

void received_token_message(s_N *node);
