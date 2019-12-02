#include <mpi.h>

#include <stdio.h>
#include "constants.h"
#include "timer.h"

size_t timerTwait = NULL, timerTelec = NULL;

s_N *initialize_node(void) {
   return NULL;
}

s_N *create_node(int nodeRank, int nodeCount, bool printingEvents, bool loggingEvents) {

   s_N *newNode = (s_N*) malloc(sizeof(s_N));

   newNode->self = nodeRank;
   newNode->father = ELECTED_NODE;
   newNode->next = NIL;
   newNode->requestingCS = false;
   newNode->tokenPresent = (newNode->father == nodeRank);

   if (newNode->father == nodeRank) {
      newNode->father = NIL;
   }

   newNode->x = load_x_set_node(nodeRank, nodeCount);
   newNode->xc = load_xc_set_node();
   newNode->myState = rest;

   // Inicializando atributos de print e log.
   newNode->printingEvents = printingEvents;
   newNode->loggingEvents = loggingEvents;
   MPI_File *logFile = malloc(sizeof(MPI_File));
   newNode->logFile = logFile;
   char *logBuffer = malloc(sizeof(char) * 2048);
   newNode->logBuffer = logBuffer;

   newNode->failed = false;

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

bool isContainedInSet(s_IA *set, int element) {

   for (int i = 0; i < set->arrayLength; i++) {

      if (set->array[i] == element) {

         return true;

      }

   }

   return false;

}

char *stateToString(s_N *node) {

   char *stateToString = malloc(sizeof(char) * 20);

   switch (node->myState) {

      case rest:

         strcpy(stateToString, "rest");

         break;

      case waiting:

         strcpy(stateToString, "waiting");

         break;

      case active:

         strcpy(stateToString, "active");

         break;

      case consulting:

         strcpy(stateToString, "consulting");

         break;

      case candidate:

         strcpy(stateToString, "candidate");

         break;

      case observer:

         strcpy(stateToString, "observer");

         break;

      case query:

         strcpy(stateToString, "query");

         break;

   }

   return stateToString;

}

char *tagToString(int TAG_MPI_MESSAGE) {

   char *tagToString = malloc(sizeof(char) * 25);

   switch (TAG_MPI_MESSAGE) {

      case 0: // TAG_IDLE

         strcpy(tagToString, "\'IDLE\'");

         break;

      case 1: // TAG_REQUEST

         strcpy(tagToString, "\'REQUEST\'");

         break;

      case 2: // TAG_TOKEN

         strcpy(tagToString, "\'TOKEN\'");

         break;

      case 3: // TAG_CONSULT

         strcpy(tagToString, "\'CONSULT\'");

         break;

      case 4: // TAG_QUIET

         strcpy(tagToString, "\'QUIET\'");

         break;

      case 5: // TAG_FAILURE

         strcpy(tagToString, "\'FAILURE\'");

         break;

      case 6: // TAG_PRESENT

         strcpy(tagToString, "\'PRESENT\'");

         break;

      case 7: // TAG_ELECTION

         strcpy(tagToString, "\'ELECTION\'");

         break;

      case 8: // TAG_CANDIDATE_ELECTED

         strcpy(tagToString, "\'CANDIDATE ELECTED\'");

         break;

   }

   return tagToString;

}

char *setToString(s_IA *set) {

   s_IA *auxSet = set;

   char *setToString = malloc(sizeof(char) * 1000);

   int i = 0;

   strcat(setToString, "{");

   for (i = 0; i < auxSet->arrayLength; i++) {

      int someInt = auxSet->array[i];

      char str[5];

      if (i == auxSet->arrayLength - 1) { // Último node do conjunto.

         sprintf(str, "%d", someInt);

      } else {

         sprintf(str, "%d, ", someInt);

      }

      strcat(setToString, str);

   }

   strcat(setToString, "}");

   return setToString;

}

void finalize_node(s_N *node, int nodeCount) {

   // Pode acontecer de haver algum timer ativo neste node durante a finalização, cancelando por garantia...

   struct timer_node *auxTimerTelec = (struct timer_node*) timerTelec;

   stop_timer(auxTimerTelec);

   struct timer_node *auxTimerTwait = (struct timer_node*) timerTwait;

   stop_timer(auxTimerTwait);

   for (int nodeRank = 0; nodeRank < nodeCount; nodeRank++) {

      int messageContent = node->self;

      MPI_Send(&messageContent, 1, MPI_INT, nodeRank, TAG_IDLE, MPI_COMM_WORLD);

   }

}

void destroy_node(s_N *node) {
   free(node->x);
   free(node->xc);
   free(node->logFile);
   free(node->logBuffer);
   free(node);
}

void perform_c_s(s_N *node) {

   if (node->failed == false) {

      node->myState = active;

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)... [node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, C_S_PASSAGE_DELAY, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, C_S_PASSAGE_DELAY);

      }

      sleep(C_S_PASSAGE_DELAY);

   }

}

void send_broadcast_message(s_N *node, int TAG_MPI_MESSAGE) {

   if (node->failed == false) {

      int messageContent = node->self;

      int k = 0;

      for (k = 0; k < node->x->arrayLength; k++) {

         int messageDestinataryNode = node->x->array[k];

         MPI_Send(&messageContent, 1, MPI_INT, messageDestinataryNode, TAG_MPI_MESSAGE, MPI_COMM_WORLD);

      }

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char *tag = tagToString(TAG_MPI_MESSAGE);

         char *set = setToString(node->x);

         char *state = stateToString(node);

         if (timerTelec == NULL) {

            sprintf(node->logBuffer, "(Node %d): Enviei a mensagem %s em broadcast para o(s) node(s) do conjunto x = %s. Ativei o meu timer 'Telec'. [node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, tag, set, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         } else {

            sprintf(node->logBuffer, "(Node %d): Enviei a mensagem %s em broadcast para o(s) node(s) do conjunto x = %s. Reiniciei o meu timer 'Telec'. [node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, tag, set, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         }

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(tag);

         free(set);

         free(state);

      }

      if (node->printingEvents == true) {

         char *tag = tagToString(TAG_MPI_MESSAGE);

         char *set = setToString(node->x);

         if (timerTelec == NULL) {

            printf("(Node %d): Enviei a mensagem %s em broadcast para o(s) node(s) do conjunto x = %s. Ativei o meu timer 'Telec'.\n\n", node->self, tag, set);

         } else {

            printf("(Node %d): Enviei a mensagem %s em broadcast para o(s) node(s) do conjunto x = %s. Reiniciei o meu timer 'Telec'.\n\n", node->self, tag, set);

         }

         free(tag);

         free(set);

      }

      timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

   }

}

void received_timeout_signal(size_t timerId, void *userData) {

   s_N *node = (s_N*) userData;

   if (node->failed == false) {

      int myState = node->myState;

      switch (myState) {

         case waiting:

            node->myState = consulting;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *tag = tagToString(TAG_CONSULT);

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Meu timer 'Twait' expirou e não recebi o TOKEN. Talvez tenha ocorrido uma falha no sistema! Enviando a mensagem %s em broadcast. [node->token = %s | node->myState = %s]\n", node->self, tag, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(tag);

               free(state);

            }

            if (node->printingEvents == true) {

               char *tag = tagToString(TAG_CONSULT);

               printf("(Node %d): Meu timer 'Twait' expirou e não recebi o TOKEN. Talvez tenha ocorrido uma falha no sistema! Enviando a mensagem %s em broadcast.\n\n", node->self, tag);

               free(tag);

            }

            send_broadcast_message(node, TAG_CONSULT);

            break;

         case consulting:

            node->myState = query;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *tag = tagToString(TAG_FAILURE);

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'CONSULT'! Ocorreu uma falha no sistema! Enviando a mensagem %s em broadcast. [node->token = %s | node->myState = %s]\n", node->self, tag, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(tag);

               free(state);

            }

            if (node->printingEvents == true) {

               char *tag = tagToString(TAG_FAILURE);

               printf("(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'CONSULT'! Ocorreu uma falha no sistema! Enviando a mensagem %s em broadcast.\n\n", node->self, tag);

               free(tag);

            }

            send_broadcast_message(node, TAG_FAILURE);

            break;

         case observer:

            node->myState = candidate;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *tag = tagToString(TAG_ELECTION);

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'PRESENT'! o TOKEN foi perdido! Enviando a mensagem %s em broadcast. [node->token = %s | node->myState = %s]\n", node->self, tag, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(tag);

               free(state);

            }

            if (node->printingEvents == true) {

               char *tag = tagToString(TAG_ELECTION);

               printf("(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'PRESENT'! o TOKEN foi perdido! Enviando a mensagem %s em broadcast.\n\n", node->self, tag);

               free(tag);

            }

            send_broadcast_message(node, TAG_ELECTION);

            break;

         case query:

            node->myState = candidate;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *tag = tagToString(TAG_ELECTION);

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'PRESENT'! o TOKEN foi perdido! Enviando a mensagem %s em broadcast. [node->token = %s | node->myState = %s]\n", node->self, tag, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(tag);

               free(state);

            }

            if (node->printingEvents == true) {

               char *tag = tagToString(TAG_ELECTION);

               printf("(Node %d): Meu timer 'Telec' expirou e não recebi a resposta da mensagem 'PRESENT'! o TOKEN foi perdido! Enviando a mensagem %s em broadcast.\n\n", node->self, tag);

               free(tag);

            }

            send_broadcast_message(node, TAG_ELECTION);

            break;

         case candidate:

            MPI_Send(&node->self, 1, MPI_INT, node->self, TAG_TOKEN, MPI_COMM_WORLD); // Necessário auto-envio de mensagem do TOKEN para dar unlock no tokenSemaphore.

            node->father = NIL;

            node->xc->array = NULL;
            node->xc->arrayLength = 0;

            node->next = NIL;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *tag = tagToString(TAG_CANDIDATE_ELECTED);

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Fui o candidato eleito e regenerei o TOKEN! Enviando a mensagem %s em broadcast. [node->token = %s | node->myState = %s]\n", node->self, tag, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(tag);

               free(state);

            }

            if (node->printingEvents == true) {

               char *tag = tagToString(TAG_CANDIDATE_ELECTED);

               printf("(Node %d): Fui o candidato eleito e regenerei o TOKEN! Enviando a mensagem %s em broadcast.\n\n", node->self, tag);

               free(tag);

            }

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

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char auxFather[5];
         sprintf(auxFather, "%d", node->father);

         char auxNext[5];
         sprintf(auxNext, "%d", node->next);

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Terminei de executar 'received_timeout_signal' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

   }

}

void request_c_s(s_N *node) {

   if (node->failed == false) {

      node->myState = waiting;

      node->requestingCS = true;

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Quero acessar a CRITICAL SECTION... [node->requestingCS = %s | node->myState = %s]\n", node->self, node->requestingCS ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): Quero acessar a CRITICAL SECTION...\n\n", node->self);

      }

      if (node->father != NIL) { // {The site has not the token, it should request it}

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            sprintf(node->logBuffer, "(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d! Ativei o meu timer 'Twait'. [node->tokenPresent = %s | node->father = %d]\n", node->self, node->father, node->tokenPresent ? "true" : "false", node->father);

            write_mpi_log_event(node->logFile, node->logBuffer);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): Não tenho o TOKEN, vou solicitá-lo ao node %d! Ativei o meu timer 'Twait'.\n\n", node->self, node->father);

         }

         int messageContent = node->self;

         MPI_Send(&messageContent, 1, MPI_INT, node->father, TAG_REQUEST, MPI_COMM_WORLD); // Send Request(i) to father

         node->father = NIL;

         timerTwait = start_timer(timerTwait, TWAIT, received_timeout_signal, singleShot, node);

      }

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char auxFather[5];
         sprintf(auxFather, "%d", node->father);

         char auxNext[5];
         sprintf(auxNext, "%d", node->next);

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Terminei de executar 'request_c_s' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

   }

}

void release_c_s(s_N *node) {

   if (node->failed == false) {

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

      node->myState = rest;

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char auxFather[5];
         sprintf(auxFather, "%d", node->father);

         char auxNext[5];
         sprintf(auxNext, "%d", node->next);

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Terminei de executar 'release_c_s' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

   }

}

void received_request_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      if (node->father == NIL) { // { root node }

         if (node->requestingCS == true) { // { The root node asked for the Critical Section }

            node->next = nodeSj;

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, mas quero acessar a CRITICAL SECTION. Enviarei para ele em seguida! [node->tokenPresent = %s | node->requestingCS = %s | node->next = %d]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", node->requestingCS ? "true" : "false", node->next);

               write_mpi_log_event(node->logFile, node->logBuffer);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d pediu o TOKEN, mas quero acessar a CRITICAL SECTION. Enviarei para ele em seguida!\n\n", node->self, nodeSj);

            }

         } else { // { First request to the token since the last CS: send the token directly to the requesting node }

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, vou encaminhá-lo pois não quero acessar a CRITICAL SECTION! [node->tokenPresent = %s | node->requestingCS = %s | node->next = %d]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", node->requestingCS ? "true" : "false", node->next);

               write_mpi_log_event(node->logFile, node->logBuffer);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d pediu o TOKEN, vou encaminhá-lo pois não quero acessar a CRITICAL SECTION!\n\n", node->self, nodeSj);

            }

            node->tokenPresent = false;

            int messageContent = node->self;

            MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_TOKEN, MPI_COMM_WORLD); // Send Token() to i

         }

      } else { // { Non-root node, forward the request }

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            sprintf(node->logBuffer, "(Node %d): O node %d pediu o TOKEN, mas não está comigo! Encaminharei a sua solicitação para o meu father! [node->tokenPresent = %s | node->father = %d | node->requestingCS = %s | node->next = %d]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", node->father, node->requestingCS ? "true" : "false", node->next);

            write_mpi_log_event(node->logFile, node->logBuffer);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): O node %d pediu o TOKEN, mas não está comigo! Encaminharei a sua solicitação para o meu father!\n\n", node->self, nodeSj);

         }

         int messageContent = nodeSj;

         MPI_Send(&messageContent, 1, MPI_INT, node->father, TAG_REQUEST, MPI_COMM_WORLD); // Send Request(i) to father

      }

      node->father = nodeSj;

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char auxFather[5];
         sprintf(auxFather, "%d", node->father);

         char auxNext[5];
         sprintf(auxNext, "%d", node->next);

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): Terminei de executar 'received_request_message' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

   }

}

void received_token_message(s_N *node) {

   if (node->failed == false) {

      // { Receive the token from node Sj }

      struct timer_node *auxTimerTelec = (struct timer_node*) timerTelec;

      stop_timer(auxTimerTelec);

      struct timer_node *auxTimerTwait = (struct timer_node*) timerTwait;

      stop_timer(auxTimerTwait);

      if (node->xc->array != NULL) {

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

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         sprintf(node->logBuffer, "(Node %d): Recebi o TOKEN! Cancelei o meu timer. [node->tokenPresent = %s]\n", node->self, node->tokenPresent ? "true" : "false");

         write_mpi_log_event(node->logFile, node->logBuffer);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): Recebi o TOKEN! Cancelei o meu timer.\n\n", node->self);

      }

   }

}

void received_consult_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      if (node->next == nodeSj) {

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            sprintf(node->logBuffer, "(Node %d): O node %d desconfia que houve uma falha, mas ele é o meu NEXT! Respondendo a mensagem 'CONSULT' dele. [node->next = %d]\n\n", node->self, node->next, node->next);

            write_mpi_log_event(node->logFile, node->logBuffer);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): O node %d desconfia que houve uma falha, mas ele é o meu NEXT! Respondendo a mensagem 'CONSULT' dele.\n\n", node->self, node->next);

         }

         int messageContent = node->self;

         MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_QUIET, MPI_COMM_WORLD);

      }

   }

}

void received_quiet_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      if (node->myState == consulting) {

         node->myState = waiting;

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            char *state = stateToString(node);

            sprintf(node->logBuffer, "(Node %d): Recebi a resposta da mensagem 'CONSULT' do node %d! Não houve falha no sistema! Reiniciei o meu timer 'Twait'. [node->myState = %s]\n", node->self, state);

            write_mpi_log_event(node->logFile, node->logBuffer);

            free(state);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): Recebi a resposta da mensagem 'CONSULT' do node %d! Não houve falha no sistema! Reiniciei o meu timer 'Twait'.\n\n", node->self, nodeSj);

         }

         timerTwait = start_timer(timerTwait, TWAIT, received_timeout_signal, singleShot, node);

      }

   }

}

void received_failure_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      int myState = node->myState;

      switch (myState) {

         case waiting:

            if (node->tokenPresent == true) {

               if (node->loggingEvents == true) {

                  memset(node->logBuffer, 0, sizeof(node->logBuffer));

                  char *state = stateToString(node);

                  sprintf(node->logBuffer, "(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele. [node->tokenPresent = %s | node->myState = %s]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", state);

                  write_mpi_log_event(node->logFile, node->logBuffer);

                  free(state);

               }

               if (node->printingEvents == true) {

                  printf("(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele.\n\n", node->self, nodeSj);

               }

               int messageContent = node->self;

               MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_PRESENT, MPI_COMM_WORLD);

            } else {

               if (isContainedInSet(node->xc, nodeSj) == false) {

                  node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
                  node->xc->array[node->xc->arrayLength] = nodeSj;
                  node->xc->arrayLength++;

               }

            }

            break;

         case rest:

            if (node->tokenPresent == true) {

               if (node->loggingEvents == true) {

                  memset(node->logBuffer, 0, sizeof(node->logBuffer));

                  char *state = stateToString(node);

                  sprintf(node->logBuffer, "(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele. [node->tokenPresent = %s | node->myState = %s]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", state);

                  write_mpi_log_event(node->logFile, node->logBuffer);

                  free(state);

               }

               if (node->printingEvents == true) {

                  printf("(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele.\n\n", node->self, nodeSj);

               }

               int messageContent = node->self;

               MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_PRESENT, MPI_COMM_WORLD);

            } else {

               if (isContainedInSet(node->xc, nodeSj) == false) {

                  node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
                  node->xc->array[node->xc->arrayLength] = nodeSj;
                  node->xc->arrayLength++;

               }

            }

            break;

         case active:

            if (node->tokenPresent == true) {

               if (node->loggingEvents == true) {

                  memset(node->logBuffer, 0, sizeof(node->logBuffer));

                  char *state = stateToString(node);

                  sprintf(node->logBuffer, "(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele. [node->tokenPresent = %s | node->myState = %s]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", state);

                  write_mpi_log_event(node->logFile, node->logBuffer);

                  free(state);

               }

               if (node->printingEvents == true) {

                  printf("(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele.\n\n", node->self, nodeSj);

               }

               int messageContent = node->self;

               MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_PRESENT, MPI_COMM_WORLD);

            } else {

               if (isContainedInSet(node->xc, nodeSj) == false) {

                  node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
                  node->xc->array[node->xc->arrayLength] = nodeSj;
                  node->xc->arrayLength++;

               }

            }

            break;

         case consulting:

            if (node->tokenPresent == true) {

               if (node->loggingEvents == true) {

                  memset(node->logBuffer, 0, sizeof(node->logBuffer));

                  char *state = stateToString(node);

                  sprintf(node->logBuffer, "(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele. [node->tokenPresent = %s | node->myState = %s]\n", node->self, nodeSj, node->tokenPresent ? "true" : "false", state);

                  write_mpi_log_event(node->logFile, node->logBuffer);

                  free(state);

               }

               if (node->printingEvents == true) {

                  printf("(Node %d): O node %d está procurando o TOKEN, que está comigo! Respondendo a mensagem 'FAILURE' dele.\n\n", node->self, nodeSj);

               }

               int messageContent = node->self;

               MPI_Send(&messageContent, 1, MPI_INT, nodeSj, TAG_PRESENT, MPI_COMM_WORLD);

            } else {

               if (isContainedInSet(node->xc, nodeSj) == false) {

                  node->xc->array = realloc(node->xc->array, sizeof(int) * (node->xc->arrayLength + 1));
                  node->xc->array[node->xc->arrayLength] = nodeSj;
                  node->xc->arrayLength++;

               }

            }

            break;

         case observer:

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

                memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): Estou observando a eleição. Reiniciei o meu timer 'Telec'. [node->tokenPresent = %s | node->myState = %s]\n", node->self, node->tokenPresent ? "true" : "false", state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): Estou observando a eleição. Reiniciei o meu timer 'Telec'.\n\n", node->self);

            }

            break;

      }

   }

}

void received_election_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      int myState = node->myState;

      switch (myState) {

         case waiting:

            node->myState = observer;

            node->xc->array = NULL;
            node->xc->arrayLength = 0;

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

            }

            break;

         case rest:

            node->myState = observer;

            node->xc->array = NULL;
            node->xc->arrayLength = 0;

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

            }

            break;

         case consulting:

            node->myState = observer;

            node->xc->array = NULL;
            node->xc->arrayLength = 0;

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

            }

            break;

         case query:

            node->myState = observer;

            node->xc->array = NULL;
            node->xc->arrayLength = 0;

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

            }

            break;

         case candidate:

            if (nodeSj < node->self) {

               node->myState = observer;

               timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

               if (node->loggingEvents == true) {

                  memset(node->logBuffer, 0, sizeof(node->logBuffer));

                  char *state = stateToString(node);

                  sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN e é melhor candidato do que eu. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

                  write_mpi_log_event(node->logFile, node->logBuffer);

                  free(state);

               }

               if (node->printingEvents == true) {

                  printf("(Node %d): O node %d se candidatou para regenerar o TOKEN e é melhor candidato do que eu. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

               }

            }

            break;

         case observer:

            timerTelec = start_timer(timerTelec, TELEC, received_timeout_signal, singleShot, node);

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               char *state = stateToString(node);

               sprintf(node->logBuffer, "(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'. [node->myState = %s]\n", node->self, nodeSj, state);

               write_mpi_log_event(node->logFile, node->logBuffer);

               free(state);

            }

            if (node->printingEvents == true) {

               printf("(Node %d): O node %d se candidatou para regenerar o TOKEN. Reiniciei o meu timer 'Telec'.\n\n", node->self, nodeSj);

            }

            break;

      }

   }

}

void received_present_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      if (node->myState == query) {

         struct timer_node *auxTimerTelec = (struct timer_node*) timerTelec;

         stop_timer(auxTimerTelec);

         node->father = nodeSj;

         node->next = NIL;

         if (node->loggingEvents == true) {

            memset(node->logBuffer, 0, sizeof(node->logBuffer));

            char *state = stateToString(node);

            sprintf(node->logBuffer, "(Node %d): Recebi a resposta da mensagem 'FAILURE' do node %d! O TOKEN está com ele! Solicitei-o e cancelei o meu timer 'Telec'. [node->father = %s | node->next = %s | node->myState = %s]\n", node->self, nodeSj, node->father == -1 ? "NIL" : node->father, node->next == -1 ? "NIL" : node->next, state);

            write_mpi_log_event(node->logFile, node->logBuffer);

            free(state);

         }

         if (node->printingEvents == true) {

            printf("(Node %d): Recebi a resposta da mensagem 'FAILURE' do node %d! O TOKEN está com ele! Solicitei-o e cancelei o meu timer 'Telec'.\n\n", node->self, nodeSj);

         }

         request_c_s(node);

      }

   }

}

void received_candidate_elected_message(s_N *node, int nodeSj) {

   if (node->failed == false) {

      struct timer_node *auxTimerTelec = (struct timer_node*) timerTelec;

      stop_timer(auxTimerTelec);

      node->father = nodeSj;

      node->xc->array = NULL;
      node->xc->arrayLength = 0;

      node->next = NIL;

      if (node->loggingEvents == true) {

         memset(node->logBuffer, 0, sizeof(node->logBuffer));

         char *state = stateToString(node);

         sprintf(node->logBuffer, "(Node %d): O node %d foi eleito! Cancelei o meu timer 'Telec'. [node->father = %s | node->next = %s | node->myState = %s]\n", node->self, nodeSj, node->father == -1 ? "NIL" : node->father, node->next == -1 ? "NIL" : node->next, state);

         write_mpi_log_event(node->logFile, node->logBuffer);

         free(state);

      }

      if (node->printingEvents == true) {

         printf("(Node %d): O node %d foi eleito! Cancelei o meu timer 'Telec'.\n\n", node->self, nodeSj);

      }

      if (node->requestingCS == true) {

         request_c_s(node);

      } else {

         node->myState = rest;

      }

   }

}
