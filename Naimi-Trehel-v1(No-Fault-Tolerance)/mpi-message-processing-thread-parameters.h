
// Estrutura de dados dos parâmetros da thread 'mpiMessageProcessingThread'.
typedef struct mpiMessageProcessingThreadParameters {
   s_N *node;
   int nodeCount;
} s_MPIMPTP;

s_MPIMPTP *initialize_mpi_message_processing_thread_parameters(void);

s_MPIMPTP *create_mpi_message_processing_thread_parameters(s_N *node, int nodeCount);

void destroy_mpi_message_processing_thread_parameters(s_MPIMPTP *mpiMessageProcessingThreadParameters);
