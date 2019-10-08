#include <mpi.h>

#include "constants.h"
#include "node.h"

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int nodeRank) {
   s_N *newNode = (s_N*) malloc(sizeof(s_N));
   newNode->self = nodeRank;
   newNode->last = NIL;
   newNode->next = NIL;
   newNode->tokenPresent = false;
   newNode->requestingCS = false;
   return newNode;
}

void finalize_node(s_N *node, int nodeCount) {

   for (int nodeRank = 0; nodeRank < nodeCount; nodeRank++) {

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, nodeRank, TAG_IDLE, MPI_COMM_WORLD);

   }

}

void destroy_node(s_N *node) {
   free(node);
}

void perform_c_s(s_N *node) {

   srand(time(NULL));

   int criticalSectionPerformanceDelay = rand() % 10;

   if (criticalSectionPerformanceDelay == 0) {

      criticalSectionPerformanceDelay = 1;

   }

   printf("(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, criticalSectionPerformanceDelay);

   sleep(criticalSectionPerformanceDelay);

}

void request_c_s(s_N *node, int nodeCount) {

   printf("(Node %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

   node->requestingCS = true;

   if (node->last != NIL) {

      // {The site has not the token, it should request it }

      printf("(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d!\n\n", node->self, node->last);

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, node->last, TAG_REQUEST, MPI_COMM_WORLD);

      node->last = NIL;

   }

}

void release_c_s(s_N *node) {

   printf("(Node %d): Terminei de acessar a CRITICAL SECTION!\n\n", node->self);

   node->requestingCS = false;

   if (node->next != NIL) {

      printf("(Node %d): O node %d quer acessar a CRITICAL SECTION, vou encaminhar o TOKEN para ele!\n\n", node->self, node->next);

      node->tokenPresent = false;

      int messageContent = node->next;

      MPI_Send(&messageContent, 1, MPI_INT, node->next, TAG_TOKEN, MPI_COMM_WORLD);

      node->next = NIL;

   }

}

void received_request_message(s_N *node, int requestingNode) {

   // { Sj is the requesting node }

   if (node->last == NIL) {

      // { root node }

      if (node->requestingCS = true) {

         // { The node asked for the Critical Section }

         node->next = requestingNode;

      } else {

         // { First request to the token since the last CS: send the token directly to the requesting node }

         node->tokenPresent = false;

         int messageContent = requestingNode;

         MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_TOKEN, MPI_COMM_WORLD);

      }

   } else {

      // { Non-root node, forward the request }

      int messageContent = requestingNode;

      MPI_Send(&messageContent, 1, MPI_INT, node->last, TAG_REQUEST, MPI_COMM_WORLD);

   }

   node->last = requestingNode;

}

void received_token_message(s_N *node) {

   // { Receive the token from node Sj }

   node->tokenPresent = true;

   printf("(Node %d): Recebi o TOKEN!\n\n", node->self);

}
