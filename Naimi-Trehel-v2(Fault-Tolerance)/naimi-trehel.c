#include <mpi.h>
#include <semaphore.h>

#include "constants.h"
#include "node.h"
#include "mpi-message-processing-thread-parameters.h"

// Unnamed semaphore (thread-shared semaphore) 'g_TokenSemaphore'.
sem_t g_TokenSemaphore;

// Job de processamento de mensagens MPI.
void jobMPIMessageProcessing(const void *parameters) {

   s_MPIMPTP *mpiMessageProcessingThreadParameters = (s_MPIMPTP*) parameters;

   s_N *node = mpiMessageProcessingThreadParameters->node;

   int nodeCount = mpiMessageProcessingThreadParameters->nodeCount;

   int messageContent = 0;

   int idleNodeCount = 0;

   int requestingNode = 0;

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

	    requestingNode = messageContent;

            received_request_message(node, requestingNode);

            break;

         case TAG_TOKEN: // Eu (node) estou recebendo o TOKEN para acessar a CRITICAL SECTION.

            received_token_message(node);

	    sem_post(&g_TokenSemaphore); // tokenSemaphore UNLOCK.

            break;

      }

   }

   printf("----- (Node %d): Encerrei o meu jobMPIMessageProcessing! -----\n\n", node->self);

}

int main(int argc, char *argv[]) {

   int nodeRank, nodeCount;

   // Inicializando o ambiente MPI.
   MPI_Init(&argc, &argv);

   // Atribuindo à variável 'nodeRank' o id deste processo MPI.
   MPI_Comm_rank(MPI_COMM_WORLD, &nodeRank);

   // Atribuindo à variável 'nodeCount' o número de processos MPI.
   MPI_Comm_size(MPI_COMM_WORLD, &nodeCount);

   // Criando o node para este processo MPI.
   s_N *node = initialize_node();

   node = create_node(nodeRank, nodeCount);

   if (node->self == ELECTED_NODE) { // O node 0 foi eleito, inicialmente, como o TOKEN OWNER...

      received_token_message(node);

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 1 (TOKEN OWNER = true).
      sem_init(&g_TokenSemaphore, 0, 1);

   } else { // Atribuindo o node 0 como TOKEN OWNER dos demais nodes...

      node->last = ELECTED_NODE;

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 0 (TOKEN OWNER = false).
      sem_init(&g_TokenSemaphore, 0, 0);

   }

   // Declaração da thread 'mpiMessageProcessingThread', responsável pelo processamento de mensagens MPI.
   pthread_t mpiMessageProcessingThread;

   // Criando os parâmetros para a thread 'mpiMessageProcessingThread'.
   s_MPIMPTP *mpiMessageProcessingThreadParameters = initialize_mpi_message_processing_thread_parameters();

   mpiMessageProcessingThreadParameters = create_mpi_message_processing_thread_parameters(node, nodeCount);

   // Criando a thread 'mpiMessageProcessingThread', passando o job (função callback) 'jobMPIMessageProcessing' e os parâmetros 'mpiMessageProcessingThreadParameters'.
   pthread_create(&mpiMessageProcessingThread, NULL, (const void *) jobMPIMessageProcessing, mpiMessageProcessingThreadParameters);

   // Este node está requisitando o acesso à CRITICAL SECTION.
   request_c_s(node);

   // Tentando bloquear (Locking) o 'g_TokenSemaphore' (Obs: semaphoreLockedConfirmed == 0 significa sucesso na operação de bloqueio).
   int semaphoreLockedConfirmed = sem_wait(&g_TokenSemaphore);

   if (semaphoreLockedConfirmed == 0) {

      // Este node está simulando o acesso à CRITICAL SECTION.
      perform_c_s(node);

      // Este node está liberando o acesso à CRITICAL SECTION.
      release_c_s(node);

   }

   // Disparando em broadcast para a thread 'mpiMessageProcessingThread'
   // que este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
   finalize_node(node, nodeCount);

   // Aguardando a finalização da thread 'mpiMessageProcessingThread' em cada node.
   pthread_join(mpiMessageProcessingThread, NULL);

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
