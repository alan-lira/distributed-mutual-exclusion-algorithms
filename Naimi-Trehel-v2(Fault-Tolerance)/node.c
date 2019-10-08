#include <mpi.h>

#include "constants.h"
#include "node.h"

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int nodeRank, int nodeCount) {
   s_N *newNode = (s_N*) malloc(sizeof(s_N));
   newNode->self = nodeRank;
   newNode->last = NIL;
   newNode->next = NIL;
   newNode->tokenPresent = false;
   newNode->requestingCS = false;
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

s_IA *load_xc_set_node() {

   s_IA *xc = malloc(sizeof(s_IA));

   xc->array = NULL;
   xc->arrayLength = 0;

   return xc;

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

   node->myState = active;

   srand(time(NULL));

   int criticalSectionPerformanceDelay = rand() % 10;

   if (criticalSectionPerformanceDelay == 0) {

      criticalSectionPerformanceDelay = 1;

   }

   printf("(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, criticalSectionPerformanceDelay);

   sleep(criticalSectionPerformanceDelay);

}

void send_broadcast_message(s_N *node, int TAG_MPI_MESSAGE) {

   int messageContent = node->self;

   int k = 0;

   for (k = 0; k < node->x->arrayLength; k++) {

      int messageDestinataryNode = node->x->array[k];

      MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_MPI_MESSAGE, MPI_COMM_WORLD);

   }

   //TO DO: start_timer (TELEC) goes here...

}

void received_timeout_signal(s_N *node) {

   int myState = node->myState;

   switch (myState) {

      case waiting:

         node->myState = consulting;

         send_broadcast_message(node, TAG_CONSULT);

         break;

      case consulting:

         node->myState = query;

         send_broadcast_message(node, TAG_FAILURE);

         break;

      case observer:

         node->myState = candidate;

         send_broadcast_message(node, TAG_ELECTION);

         break;

      case query:

         node->myState = candidate;

         send_broadcast_message(node, TAG_ELECTION);

         break;

      case candidate:

         node->tokenPresent = true;

         node->last = NIL;

         node->xc->array = NULL;
	 node->xc->arrayLength = 0;

         node->next = NIL;

         int messageContent = node->self;

         int k = 0;

         for (k = 0; k < node->x->arrayLength; k++) {

            int messageDestinataryNode = node->x->array[k];

            MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_CANDIDATE_ELECTED, MPI_COMM_WORLD);

         }

         if (node->requestingCS == true) {

            request_c_s(node);

         } else {

            node->myState = rest;

         }

         break;

   }

}

void request_c_s(s_N *node) {

   printf("(Node %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

   node->myState = waiting;

   node->requestingCS = true;

   if (node->last != NIL) {

      // {The site has not the token, it should request it }

      printf("(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d!\n\n", node->self, node->last);

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, node->last, TAG_REQUEST, MPI_COMM_WORLD);

      node->last = NIL;

      //TO DO: start_timer (TWAIT) goes here...

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

   node->myState = rest;

}

void received_request_message(s_N *node, int requestingNode) {

   // { Sj is the requesting node }

   if (node->last == NIL) {

      // { root node }

      if (node->requestingCS == true) {

         // { The node asked for the Critical Section }

         node->next = requestingNode;

      } else {

         // { First request to the token since the last CS: send the token directly to the requesting node }

         node->tokenPresent = false;

         int messageContent = node->self;

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

   //TO DO: cancel_timer goes here...

   if (node->xc != NULL) {

      int messageContent = node->self;

      int j = 0;

      for (j = 0; j < node->xc->arrayLength; j++) {

         int messageDestinataryNode = node->xc->array[j];

         MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_PRESENT, MPI_COMM_WORLD);

      }

      node->xc->array = NULL;
      node->xc->arrayLength = 0;

   }

   node->tokenPresent = true;

   printf("(Node %d): Recebi o TOKEN!\n\n", node->self);

}

void received_consult_message(s_N *node, int requestingNode) {

   if (node->next == requestingNode) {

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_QUIET, MPI_COMM_WORLD);

   }

}

void received_quiet_message(s_N *node, int requestingNode) {

   if (node->myState == consulting) {

      node->myState = waiting;

      //TO DO: start_timer (TWAIT) goes here...

   }

}

void received_failure_message(s_N *node, int requestingNode) {

   int myState = node->myState;

   switch (myState) {

      case waiting:

         if (node->tokenPresent == true) {

            int messageContent = node->self;

            MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_PRESENT, MPI_COMM_WORLD);

         } else {

            node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
            node->xc->array[node->xc->arrayLength] = requestingNode;
            node->xc->arrayLength++;

         }

         break;

      case rest:

         if (node->tokenPresent == true) {

            int messageContent = node->self;

            MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_PRESENT, MPI_COMM_WORLD);

         } else {

            node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
            node->xc->array[node->xc->arrayLength] = requestingNode;
            node->xc->arrayLength++;

         }

         break;

      case active:

         if (node->tokenPresent == true) {

            int messageContent = node->self;

            MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_PRESENT, MPI_COMM_WORLD);

         } else {

            node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
            node->xc->array[node->xc->arrayLength] = requestingNode;
            node->xc->arrayLength++;

         }

         break;

      case consulting:

         if (node->tokenPresent == true) {

            int messageContent = node->self;

            MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_PRESENT, MPI_COMM_WORLD);

         } else {

            node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
            node->xc->array[node->xc->arrayLength] = requestingNode;
            node->xc->arrayLength++;

         }

         break;

      case observer:

         //TO DO: start_timer (TELEC) goes here...

         break;

   }

}

void received_election_message(s_N *node, int requestingNode) {

   int myState = node->myState;

   switch (myState) {

      case waiting:

         node->myState = observer;

         node->xc->array = NULL;
         node->xc->arrayLength = 0;

         //TO DO: start_timer (TELEC) goes here...

         break;

      case rest:

         node->myState = observer;

         node->xc->array = NULL;
         node->xc->arrayLength = 0;

         //TO DO: start_timer (TELEC) goes here...

         break;

      case consulting:

         node->myState = observer;

         node->xc->array = NULL;
         node->xc->arrayLength = 0;

         //TO DO: start_timer (TELEC) goes here...

         break;

      case query:

         node->myState = observer;

         node->xc->array = NULL;
         node->xc->arrayLength = 0;

         //TO DO: start_timer (TELEC) goes here...

         break;

      case candidate:

         if (requestingNode < node->self) {

            node->myState = observer;

            //TO DO: start_timer (TELEC) goes here...

         }

         break;

      case observer:

         //TO DO: start_timer (TELEC) goes here...

         break;

   }

}

void received_present_message(s_N *node, int requestingNode) {

   if (node->myState == query) {

      //TO DO: cancel_timer goes here...

      node->last = requestingNode;

      node->next = NIL;

      request_c_s(node);

   }

}
