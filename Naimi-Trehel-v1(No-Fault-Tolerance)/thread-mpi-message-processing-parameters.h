
// Estrutura de dados dos parâmetros da thread 'threadMPIMessageProcessing'.
typedef struct threadMPIMessageProcessingParameters {
   s_N *node;
   int nodeCount;
}s_TMPIMPP;

s_TMPIMPP *initialize_thread_parameters(void);

s_TMPIMPP *create_thread_parameters(s_N *node, int nodeCount);

void destroy_thread_parameters(s_TMPIMPP *threadParameters);
