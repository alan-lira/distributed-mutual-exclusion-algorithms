#include <mpi.h>
#include <stdio.h>
#include "constants.h"
#include "log.h"
#include "node.h"

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int nodeRank, bool printingEvents, bool loggingEvents) {

   s_N *newNode = (s_N*) malloc(sizeof(s_N));

   newNode->self = nodeRank;
   newNode->father = ELECTED_NODE;
   newNode->next = NIL;
   newNode->requestingCS = false;
   newNode->tokenPresent = (newNode->father == nodeRank);

   if (newNode->father == nodeRank) {
      newNode->father = NIL;
   }

   // Inicializando atributos de print e log.
   newNode->printingEvents = printingEvents;
   newNode->loggingEvents = loggingEvents;
   MPI_File *logFile = malloc(sizeof(MPI_File));
   newNode->logFile = logFile;
   char *logBuffer = malloc(sizeof(char) * 1024);
   newNode->logBuffer = logBuffer;

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

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      sprintf(node->logBuffer, "(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)... [node->requestingCS = %s | node->tokenPresent = %s]\n", node->self, C_S_PASSAGE_DELAY, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   if (node->printingEvents == true) {

      printf("(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, C_S_PASSAGE_DELAY);

   }

   sleep(C_S_PASSAGE_DELAY);

}

void request_c_s(s_N *node) {

   node->requestingCS = true;

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      sprintf(node->logBuffer, "(Node %d): Quero acessar a CRITICAL SECTION... [node->requestingCS = %s]\n", node->self, node->requestingCS ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   if (node->printingEvents == true) {

      printf("(Node %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

   }

   if (node->father != NIL) { // {The site has not the token, it should request it}

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         sprintf(node->logBuffer, "(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d! [node->tokenPresent = %s | node->father = %d]\n", node->self, node->father, node->tokenPresent ? "true" : "false", node->father);

         write_mpi_log_event(node->logFile, node->logBuffer);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d!\n\n", node->self, node->father);

      }

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, node->father, TAG_REQUEST, MPI_COMM_WORLD); // Send Request(i) to father

      node->father = NIL;

   }

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      char auxFather[5];
      sprintf(auxFather, "%d", node->father);

      char auxNext[5];
      sprintf(auxNext, "%d", node->next);

      sprintf(node->logBuffer, "(Node %d): Terminei de executar 'request_c_s' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   node->requestedTokenTime = MPI_Wtime();

}

void release_c_s(s_N *node) {

   node->requestingCS = false;

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      sprintf(node->logBuffer, "(Node %d): Terminei de acessar a CRITICAL SECTION! [node->requestingCS = %s]\n", node->self, node->requestingCS ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   if (node->printingEvents == true) {

      printf("(Node %d): Terminei de acessar a CRITICAL SECTION!\n\n", node->self);

   }

   if (node->next != NIL) {

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         sprintf(node->logBuffer, "(Node %d): O node %d quer acessar a CRITICAL SECTION, vou encaminhar o TOKEN para ele! [node->next = %d]\n", node->self, node->next, node->next);

         write_mpi_log_event(node->logFile, node->logBuffer);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): O node %d quer acessar a CRITICAL SECTION, vou encaminhar o TOKEN para ele!\n\n", node->self, node->next);

      }

      node->tokenPresent = false;

      int messageContent = node->next;

      MPI_Send(&messageContent, 1, MPI_INT, node->next, TAG_TOKEN, MPI_COMM_WORLD); // Send Token() to next

      node->next = NIL;

   }

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      char auxFather[5];
      sprintf(auxFather, "%d", node->father);

      char auxNext[5];
      sprintf(auxNext, "%d", node->next);

      sprintf(node->logBuffer, "(Node %d): Terminei de executar 'release_c_s' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

}

void received_request_message(s_N *node, int requestingNode) {

   if (node->father == NIL) { // { root node }

      if (node->requestingCS == true) { // { The root node asked for the Critical Section }

         node->next = requestingNode;

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, mas quero acessar a CRITICAL SECTION. Enviarei para ele em seguida! [node->tokenPresent = %s | node->requestingCS = %s | node->next = %d]\n", node->self, requestingNode, node->tokenPresent ? "true" : "false", node->requestingCS ? "true" : "false", node->next);

            write_mpi_log_event(node->logFile, node->logBuffer);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): O node %d pediu o TOKEN, mas quero acessar a CRITICAL SECTION. Enviarei para ele em seguida!\n\n", node->self, requestingNode);

         }

      } else { // { First request to the token since the last CS: send the token directly to the requesting node }

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, vou encaminhá-lo pois não quero acessar a CRITICAL SECTION! [node->tokenPresent = %s | node->requestingCS = %s | node->next = %d]\n", node->self, requestingNode, node->tokenPresent ? "true" : "false", node->requestingCS ? "true" : "false", node->next);

            write_mpi_log_event(node->logFile, node->logBuffer);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): O node %d pediu o TOKEN, vou encaminhá-lo pois não quero acessar a CRITICAL SECTION!\n\n", node->self, requestingNode);

         }

         node->tokenPresent = false;

         int messageContent = node->self;

         MPI_Send(&messageContent, 1, MPI_INT, requestingNode, TAG_TOKEN, MPI_COMM_WORLD); // Send Token() to i

      }

   } else { // { Non-root node, forward the request }

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, mas não está comigo! Encaminharei a sua solicitação para o meu father! [node->tokenPresent = %s | node->father = %d | node->requestingCS = %s | node->next = %d]\n", node->self, requestingNode, node->tokenPresent ? "true" : "false", node->father, node->requestingCS ? "true" : "false", node->next);

         write_mpi_log_event(node->logFile, node->logBuffer);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): O node %d pediu o TOKEN, mas não está comigo! Encaminharei a sua solicitação para o meu father!\n\n", node->self, requestingNode);

      }

      int messageContent = requestingNode;

      MPI_Send(&messageContent, 1, MPI_INT, node->father, TAG_REQUEST, MPI_COMM_WORLD); // Send Request(i) to father

   }

   node->father = requestingNode;

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      char auxFather[5];
      sprintf(auxFather, "%d", node->father);

      char auxNext[5];
      sprintf(auxNext, "%d", node->next);

      sprintf(node->logBuffer, "(Node %d): Terminei de executar 'received_request_message' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

}

void received_token_message(s_N *node) {

   // { Receive the token from node Sj }

   node->tokenPresent = true;

   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      sprintf(node->logBuffer, "(Node %d): Recebi o TOKEN! [node->tokenPresent = %s]\n", node->self, node->tokenPresent ? "true" : "false");

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   if (node->printingEvents == true) {

      printf("(Node %d): Recebi o TOKEN!\n\n", node->self);

   }

}
