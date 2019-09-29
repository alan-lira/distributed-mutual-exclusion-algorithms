#include <mpi.h>
#include <semaphore.h>

#include "constants.h"
#include "node.h"
#include "thread-mpi-message-processing-parameters.h"

// Unnamed semaphore (thread-shared semaphore) 'g_TokenSemaphore'.
sem_t g_TokenSemaphore;

// Job de processamento de mensagens MPI.
void jobMPIMessageProcessing(const void *parameters) {

   s_TMPIMPP *threadParameters = (s_TMPIMPP*) parameters;

   s_N *node = threadParameters->node;

   int nodeCount = threadParameters->nodeCount;

   int valorMensagem = 0;

   int somadorMensagensFim = 0;

   int idNodeSolicitante = 0;

   while(somadorMensagensFim != nodeCount) {

      MPI_Status status;

      MPI_Recv(&valorMensagem, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int mpiTag = status.MPI_TAG;

      int mpiSource = status.MPI_SOURCE;

      switch(mpiTag) {

         case 1: // Mensagem da Main sinalizando que não vai mais acessar a CRITICAL SECTION.

            somadorMensagensFim += 1;

            break;

         case 500: // Existe um node Sj (valorMensagem) solicitando para mim (node) o TOKEN para acessar a CRITICAL SECTION.

	    idNodeSolicitante = valorMensagem;

            receive_request_cs(node, idNodeSolicitante);

            break;

         case 1000: // Eu (node) estou recebendo o TOKEN para acessar a CRITICAL SECTION.

            receive_token(node);

	    sem_post(&g_TokenSemaphore); // tokenSemaphore UNLOCK.

            break;

      }

   }

   printf("Nó %d: Encerrei o jobMPIMessageProcessing!\n\n", node->self);

}

int main(int argc, char *argv[]) {

   int rank, nodeCount;

   // Inicializando o ambiente MPI.
   MPI_Init(&argc, &argv);

   // Atribuindo à variável 'rank' o id deste processo MPI.
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   // Atribuindo à variável 'nodeCount' o número de processos MPI.
   MPI_Comm_size(MPI_COMM_WORLD, &nodeCount);

   // Criando o node para este processo MPI.
   s_N *node = initialize_node();
   node = create_node(rank);

   if (node->self == ELECTED_NODE) { // O node 0 foi eleito, inicialmente, como o TOKEN OWNER...

      node->token = true;

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 1 (TOKEN OWNER = true).
      sem_init(&g_TokenSemaphore, 0, 1);

   } else { // Atribuindo o node 0 como TOKEN OWNER dos demais nodes...

      node->owner = ELECTED_NODE;

      // Inicializando o semáforo 'g_TokenSemaphore' com o valor 0 (TOKEN OWNER = false).
      sem_init(&g_TokenSemaphore, 0, 0);

   }

   // Declaração da thread 'threadMPIMessageProcessing', responsável pelo processamento de mensagens MPI.
   pthread_t threadMPIMessageProcessing;

   // Criando os parâmetros para a thread 'threadMPIMessageProcessing'.
   s_TMPIMPP *threadParameters = initialize_thread_parameters();
   threadParameters = create_thread_parameters(node, nodeCount);

   // Criando a thread 'threadMPIMessageProcessing', passando o job (função callback) 'jobMPIMessageProcessing' e os parâmetros 'threadParameters'.
   pthread_create(&threadMPIMessageProcessing, NULL, (const void *) jobMPIMessageProcessing, threadParameters);

   // Este node está requisitando o acesso à CRITICAL SECTION.
   request_cs(node, nodeCount);

   // Tentando bloquear (Locking) o 'g_TokenSemaphore' (Obs: semaphoreLockedZero == 0 significa sucesso na operação de bloqueio).
   int semaphoreLockedZero = sem_wait(&g_TokenSemaphore);

   if (semaphoreLockedZero == 0) {

      // Este node está simulando o acesso à CRITICAL SECTION.
      srand(time(NULL));

      int duracao_acesso = rand() % 15;

      if (duracao_acesso == 0) {
         duracao_acesso = 1;
      }

      printf("(Nó %d): Acessando a CRITICAL SECTION por %d segundo(s)...\n\n", node->self, duracao_acesso);
      sleep(duracao_acesso);

      // Este node está liberando o acesso à CRITICAL SECTION.
      release_cs(node);

   }

   // Disparando em broadcast para a thread 'threadMPIMessageProcessing'
   // que este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
   finalize_node(node, nodeCount);

   // Aguardando a finalização da thread 'threadMPIMessageProcessing' em cada node.
   pthread_join(threadMPIMessageProcessing, NULL);

   // Desalocando o espaço ocupado na memória pelos parâmetros 'threadParameters'.
   destroy_thread_parameters(threadParameters);

   // Desalocando o espaço ocupado na memória pelo node.
   destroy_node(node);

   // Finalizando o ambiente MPI.
   MPI_Finalize();

   // Destruindo o g_TokenSemaphore.
   sem_destroy(&g_TokenSemaphore);

   // Finalizando o programa.
   exit(0);

}
