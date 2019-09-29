#include <mpi.h>

#include "constants.h"
#include "node.h"

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int rank) {
   s_N *newNode = (s_N*) malloc(sizeof(s_N));
   newNode->self = rank;
   newNode->owner = newNode->next = -1;
   newNode->token = newNode->requesting = false;
   return newNode;
}

void finalize_node(s_N *node, int nodeCount) {

   for (int nodeRank = 0; nodeRank < nodeCount; nodeRank++) {

      int valorMensagem = node->self;

      MPI_Send(&valorMensagem, 1, MPI_INT, nodeRank, TAG_FIM, MPI_COMM_WORLD);

   }

}

void destroy_node(s_N *node) {
   free(node);
}

void request_cs(s_N *node, int nodeCount) {

   printf("(Nó %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

   node->requesting = true;

   if(node->owner != -1) {

      // {The site has not the token, it should request it }

      printf("(Nó %d): Não tenho o TOKEN, vou solicitá-lo ao nó %d!\n\n", node->self, node->owner);

      int requestingNode = node->self;

      MPI_Send(&requestingNode, 1, MPI_INT, node->owner, TAG_REQUEST_TOKEN, MPI_COMM_WORLD);

      node->owner = -1;

   }

}

void release_cs(s_N *node) {

   printf("(Nó %d): Terminei de acessar a CRITICAL SECTION!\n\n", node->self);

   node->requesting = false;

   if(node->next != -1) {

      int requestingNode = node->next;

      printf("(Nó %d): O nó %d quer acessar a CRITICAL SECTION, vou encaminhar o TOKEN para ele!\n\n", node->self, node->next);

      MPI_Send(&requestingNode, 1, MPI_INT, node->next, TAG_TOKEN, MPI_COMM_WORLD);

      node->token = false;
      node->next = -1;

   }

}

void receive_request_cs(s_N *node, int requestingNode) {

   // { Sj is the requesting node }

   if(node->owner == -1) {

      // { root node }

      if(node->requesting = true) {

         // { The node asked for the Critical Section }

         node->next = requestingNode;

      } else {

         // { First request to the token since the last CS: send the token directly to the requesting node }

         node->token = false;

         MPI_Send(&requestingNode, 1, MPI_INT, requestingNode, TAG_TOKEN, MPI_COMM_WORLD);

      }

   } else {

      // { Non-root node, forward the request }

      MPI_Send(&requestingNode, 1, MPI_INT, node->owner, TAG_REQUEST_TOKEN, MPI_COMM_WORLD);

   }

   node->owner = requestingNode;

}

void receive_token(s_N *node) {

   // { Receive the token from node Sj }

   node->token = true;

   printf("(Nó %d): Recebi o TOKEN!\n\n", node->self);

}
