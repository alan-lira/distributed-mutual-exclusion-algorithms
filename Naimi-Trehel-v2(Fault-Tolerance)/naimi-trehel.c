#include <mpi.h>
#include <semaphore.h>

#include "constants.h"
#include "node.h"
#include "mpi-message-processing-thread-parameters.h"

sem_t g_TokenSemaphore; // Unnamed semaphore (thread-shared semaphore) 'g_TokenSemaphore'.

// Job de processamento de mensagens MPI.
void jobMPIMessageProcessing(const void *parameters) {

   s_MPIMPTP *mpiMessageProcessingThreadParameters = (s_MPIMPTP*) parameters;

   s_N *node = mpiMessageProcessingThreadParameters->node;

   int nodeCount = mpiMessageProcessingThreadParameters->nodeCount;

   int messageContent = 0;

   int idleNodeCount = 0;

   int nodeSj = 0;

   while (idleNodeCount != nodeCount) {

      MPI_Status status;

      MPI_Recv(&messageContent, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int mpiTag = status.MPI_TAG;

      int mpiSource = status.MPI_SOURCE;

      switch (mpiTag) {

         case TAG_IDLE: // Este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.

            idleNodeCount += 1;

            break;

         case TAG_REQUEST: // Existe um node Sj (messageContent) solicitando para mim (node) o TOKEN para acessar a CRITICAL SECTION.

	    nodeSj = messageContent;

            received_request_message(node, nodeSj);

            break;

         case TAG_TOKEN: // Eu (node) estou recebendo o TOKEN para acessar a CRITICAL SECTION.

            received_token_message(node);

	    sem_post(&g_TokenSemaphore); // tokenSemaphore UNLOCK.

            node->receivedTokenTime = MPI_Wtime(); // Fim do wall-clock que contabiliza o tempo de espera para este node receber o TOKEN.

            if (node->loggingEvents == true) {

               memset(node->logBuffer, 0, sizeof(node->logBuffer));

               sprintf(node->logBuffer, "(Node %d): Esperei %f segundo(s) para receber o TOKEN!\n", node->self, node->self == ELECTED_NODE ? 0 : (node->receivedTokenTime - node->requestedTokenTime));

               write_mpi_log_event(node->logFile, node->logBuffer);

            }

            printf("(Node %d): Esperei %f segundo(s) para receber o TOKEN!\n\n", node->self, node->self == ELECTED_NODE ? 0 : (node->receivedTokenTime - node->requestedTokenTime));

            break;

         case TAG_CONSULT: // Existe um node Sj (messageContent) verificando se é o meu NEXT.

	    nodeSj = messageContent;

            received_consult_message(node, nodeSj);

            break;

         case TAG_QUIET: // Eu (node) estou recebendo a confirmação do node Sj (messageContent) de que não houve uma falha.

	    nodeSj = messageContent;

            received_quiet_message(node, nodeSj);

            break;

         case TAG_FAILURE: // Existe um node Sj (messageContent) verificando se o TOKEN está comigo.

	    nodeSj = messageContent;

            received_failure_message(node, nodeSj);

            break;

         case TAG_PRESENT: // Eu (node) estou recebendo a confirmação de que o TOKEN está sob posse do node Sj (messageContent).

	    nodeSj = messageContent;

            received_present_message(node, nodeSj);

            break;

         case TAG_ELECTION: // O node Sj (messageContent) se candidatou para regenerar o TOKEN perdido.

	    nodeSj = messageContent;

            received_election_message(node, nodeSj);

            break;

         case TAG_CANDIDATE_ELECTED: // O node Sj (messageContent) foi eleito para regenerar o TOKEN perdido.

	    nodeSj = messageContent;

            received_candidate_elected_message(node, nodeSj);

            break;

         case TAG_FAILED_NODE: // Recebimento de uma mensagem enviada por um node Sj que falhou.
                               //(O envio desta mensagem é necessário para garantir a consistência entre mensagens MPI).

            break;

      }

   }

   if (node->printingEvents == true) {

      printf("----- (Node %d): Encerrei o meu jobMPIMessageProcessing! -----\n\n", node->self);

   }

}

int main(int argc, char *argv[]) {

   int nodeRank, nodeCount;

   // Inicializando o ambiente MPI.
   MPI_Init(&argc, &argv);

   // Atribuindo à variável 'nodeRank' o id deste processo MPI.
   MPI_Comm_rank(MPI_COMM_WORLD, &nodeRank);

   // Atribuindo à variável 'nodeCount' o número de processos MPI.
   MPI_Comm_size(MPI_COMM_WORLD, &nodeCount);

   bool printingEvents = false, loggingEvents = false;

   // Obtendo argumentos passados ao programa (-p e -l).
   for (int argIndex = 0; argIndex < argc; argIndex++) {

      // Habilitar print.
      if (strcmp("-p", argv[argIndex]) == 0) {

         printingEvents = true;

      }

      // Habilitar log.
      if (strcmp("-l", argv[argIndex]) == 0) {

         loggingEvents = true;

      }

   }

   // Criando o node para este processo MPI.
   s_N *node = initialize_node();

   node = create_node(nodeRank, nodeCount, printingEvents, loggingEvents);

   // Inicializando o ambiente MPI_LOG.
   if (node->loggingEvents == true) {

      start_mpi_log_environment(node->logFile);

   }

   if (node->self == ELECTED_NODE) { // O node 0 foi eleito, inicialmente, como o TOKEN OWNER...

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 1 (TOKEN OWNER = true).
      sem_init(&g_TokenSemaphore, 0, 1);

   } else { // Atribuindo o node 0 como TOKEN OWNER dos demais nodes...

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 0 (TOKEN OWNER = false).
      sem_init(&g_TokenSemaphore, 0, 0);

   }

   // Log pós-execução do procedimento 'create_node' para este node.
   if (node->loggingEvents == true) {

      memset(node->logBuffer, 0, sizeof(node->logBuffer));

      char auxFather[5];
      sprintf(auxFather, "%d", node->father);

      char auxNext[5];
      sprintf(auxNext, "%d", node->next);

      sprintf(node->logBuffer, "(Node %d): Terminei de executar 'create_node' [node->father = %s | node->next = %s | node->requestingCS = %s | node->tokenPresent = %s | node->myState = %s]\n", node->self, node->father == -1 ? "NIL" : auxFather, node->next == -1 ? "NIL" : auxNext, node->requestingCS ? "true" : "false", node->tokenPresent ? "true" : "false", stateToString(node));

      write_mpi_log_event(node->logFile, node->logBuffer);

   }

   // Declaração da thread 'mpiMessageProcessingThread', responsável pelo processamento de mensagens MPI.
   pthread_t mpiMessageProcessingThread;

   // Criando os parâmetros para a thread 'mpiMessageProcessingThread'.
   s_MPIMPTP *mpiMessageProcessingThreadParameters = initialize_mpi_message_processing_thread_parameters();

   mpiMessageProcessingThreadParameters = create_mpi_message_processing_thread_parameters(node, nodeCount);

   // Inicializando o ambiente TIMER.
   initialize_timer_thread();

   // Criando a thread 'mpiMessageProcessingThread', passando o job (função callback) 'jobMPIMessageProcessing' e os parâmetros 'mpiMessageProcessingThreadParameters'.
   pthread_create(&mpiMessageProcessingThread, NULL, (const void *) jobMPIMessageProcessing, mpiMessageProcessingThreadParameters);

   // Testando falhas de nodes.

   int *arrayFailedNodes = (int *) malloc(sizeof(int) * nodeCount);

   for (int i = 0; i < nodeCount; i++) {

      arrayFailedNodes[i] = 0; // Default: todos os nodes estão ativos.

   }

   // Settando nodes falhos.

   arrayFailedNodes[0] = 1; // Node 0 falhou.

   if (arrayFailedNodes[node->self] == 1) {

      node->failed = true;

   }

   // Desalocando o array de failed nodes.
   free(arrayFailedNodes);

   // Este node está requisitando o acesso à CRITICAL SECTION.
   request_c_s(node);

   node->requestedTokenTime = MPI_Wtime(); // Início do wall-clock que contabiliza o tempo de espera para este node receber o TOKEN.

   // Tentando bloquear (Locking) o 'g_TokenSemaphore' (Obs: semaphoreLockedConfirmed == 0 significa sucesso na operação de bloqueio).
   int semaphoreLockedConfirmed = sem_wait(&g_TokenSemaphore);

   if (semaphoreLockedConfirmed == 0) {

      // Este node está simulando o acesso à CRITICAL SECTION.
      perform_c_s(node);

      // Este node está liberando o acesso à CRITICAL SECTION.
      release_c_s(node);

   }

   sem_post(&g_TokenSemaphore); // tokenSemaphore UNLOCK.

   // Disparando em broadcast para a thread 'mpiMessageProcessingThread'
   // que este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
   finalize_node(node, nodeCount);

   // Aguardando a finalização da thread 'mpiMessageProcessingThread' em cada node.
   pthread_join(mpiMessageProcessingThread, NULL);

   // Finalizando o ambiente MPI_LOG.
   if (node->loggingEvents == true) {

      close_mpi_log_environment(node->logFile);

   }

   // Finalizando o ambiente TIMER.
   finalize_timer_thread();

   // Desalocando o espaço ocupado na memória pelos parâmetros 'mpiMessageProcessingThreadParameters'.
   destroy_mpi_message_processing_thread_parameters(mpiMessageProcessingThreadParameters);

   // Desalocando o espaço ocupado na memória pelo node.
   destroy_node(node);

   // Finalizando o ambiente MPI.
   MPI_Finalize();

   // Destruindo o g_TokenSemaphore.
   sem_destroy(&g_TokenSemaphore);

   // Finalizando o programa.
   exit(0);

}
