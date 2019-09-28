#include <stdbool.h>

// Estrutura de dados de um node.
typedef struct node {
   int self; // Indica a identificação deste node.
   int owner; // Indica o provável atual TOKEN OWNER.
   int next; // Indica o node que receberá o token quando a CRITICAL SECTION for liberada.
   bool token; // Indica se este node detém ou não o TOKEN.
   bool requesting; // Indica se este node está requisitando ou não o TOKEN.
}s_N;

s_N *initialize_node(void);

s_N *create_node(int rank);

void finalize_node(s_N *node, int nodeCount);

void destroy_node(s_N *node);

void request_cs(s_N *node, int nodeCount);

void release_cs(s_N *node);

void receive_request_cs(s_N *node, int requestingNode);

void receive_token(s_N *node);
