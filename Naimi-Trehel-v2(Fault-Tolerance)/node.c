#include <mpi.h>

#include "constants.h"
#include "node.h"

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int nodeRank, int nodeCount) {
   s_N *newNode = (s_N*) malloc(sizeof(s_N));
   newNode->self = nodeRank;
   newNode->last = newNode->next = -1;
   newNode->tokenPresent = newNode->requestingCS = false;
   newNode->x = load_x_set_node(nodeRank, nodeCount);
   newNode->xc = NULL;
   newNode->myState = rest;
   return newNode;
}

s_IA *load_x_set_node(int nodeRank, int nodeCount) {

   s_IA *x = malloc(sizeof(s_IA));

   x->array = malloc(sizeof(int) * nodeCount - 1);
   x->arrayLength = 0;

   int i = 0, index = 0;

   for (i = 0; i < nodeCount; i++) {

      if (i != nodeRank) {

         x->array[index] = i;

         index++;

         x->arrayLength++;

      }

   }

   return x;

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

void perform_cs(s_N *node) {

   node->myState = active;

   srand(time(NULL));

   int criticalSectionPassageDelay = rand() % 10;

   if (criticalSectionPassageDelay == 0) {

      criticalSectionPassageDelay = 1;

   }

   printf("(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, criticalSectionPassageDelay);

   sleep(criticalSectionPassageDelay);

}

void broadcast_message(s_N *node, int TAG_MPI_MESSAGE) {

   int messageContent = node->self;

   int k = 0;

   for (k = 0; k < node->x->arrayLength; k++) {

      int messageDestinataryNode = node->x->array[k];

      MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_MPI_MESSAGE, MPI_COMM_WORLD);

   }

   //TO DO: start_timer (TELEC) goes here...

}

void timed_out_signal(s_N *node, int nodeCount) {

   int myState = node->myState;

   switch (myState) {

      case waiting:

         node->myState = consulting;

         broadcast_message(node, TAG_CONSULT);

         break;

      case consulting:

         node->myState = query;

         broadcast_message(node, TAG_FAILURE);

         break;

      case query:

         node->myState = candidate;

         broadcast_message(node, TAG_ELECTION);

         break;

      case observer:

         node->myState = candidate;

         broadcast_message(node, TAG_ELECTION);

         break;

      case candidate:

         node->tokenPresent = true;
         node->last = -1;
         node->xc = NULL;
         node->next = NULL;

         int messageContent = node->self;

         int k = 0;

         for (k = 0; k < node->x->arrayLength; k++) {

            int messageDestinataryNode = node->x->array[k];

            MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_CANDIDATE_ELECTED, MPI_COMM_WORLD);

         }

         if (node->requestingCS) {

            request_cs(node, nodeCount);

         } else {

            node->myState = rest;

         }

         break;

   }

}

void request_cs(s_N *node, int nodeCount) {

   printf("(Node %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

   node->myState = waiting;

   node->requestingCS = true;

   if (node->last != -1) {

      // {The site has not the token, it should request it }

      printf("(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d!\n\n", node->self, node->last);

      int requestingNode = node->self;

      MPI_Send(&requestingNode, 1, MPI_INT, node->last, TAG_REQUEST, MPI_COMM_WORLD);

      node->last = -1;

      //TO DO: start_timer (TWAIT) goes here...

   }

}

void release_cs(s_N *node) {

   printf("(Node %d): Terminei de acessar a CRITICAL SECTION!\n\n", node->self);

   node->requestingCS = false;

   if (node->next != -1) {

      int requestingNode = node->next;

      printf("(Node %d): O node %d quer acessar a CRITICAL SECTION, vou encaminhar o TOKEN para ele!\n\n", node->self, node->next);

      MPI_Send(&requestingNode, 1, MPI_INT, node->next, TAG_TOKEN, MPI_COMM_WORLD);

      node->tokenPresent = false;
      node->next = -1;

   }

   node->myState = rest;

}

void receive_request_cs(s_N *node, int requestingNode) {

   // { Sj is the requesting node }

   if (node->last == -1) {

      // { root node }

      if (node->requestingCS = true) {

         // { The node asked for the Critical Section }

         node->next = requestingNode;

      } else {

         // { First request to the token since the last CS: send the token directly to the requesting node }

         node->tokenPresent = false;

         MPI_Send(&requestingNode, 1, MPI_INT, requestingNode, TAG_TOKEN, MPI_COMM_WORLD);

      }

   } else {

      // { Non-root node, forward the request }

      MPI_Send(&requestingNode, 1, MPI_INT, node->last, TAG_REQUEST, MPI_COMM_WORLD);

   }

   node->last = requestingNode;

}

void receive_token(s_N *node) {

   // { Receive the token from node Sj }

   //TO DO: cancel_timer goes here...

   if (node->xc != NULL) {

      int messageContent = node->self;

      int j = 0;

      for (j = 0; j < node->xc->arrayLength; j++) {

         int messageDestinataryNode = node->xc->array[j];

         MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_PRESENT, MPI_COMM_WORLD);

      }

   }

   node->tokenPresent = true;

   printf("(Node %d): Recebi o TOKEN!\n\n", node->self);

}
