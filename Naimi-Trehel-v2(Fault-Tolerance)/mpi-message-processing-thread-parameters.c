#include <stddef.h>

#include "node.h"
#include "mpi-message-processing-thread-parameters.h"

s_MPIMPTP *initialize_mpi_message_processing_thread_parameters(void) {
   return NULL;
}

s_MPIMPTP *create_mpi_message_processing_thread_parameters(s_N *node, int nodeCount) {
   s_MPIMPTP *newThreadParameters = (s_MPIMPTP*) malloc(sizeof(s_MPIMPTP));
   newThreadParameters->node = node;
   newThreadParameters->nodeCount = nodeCount;
   return newThreadParameters;
}

void destroy_mpi_message_processing_thread_parameters(s_MPIMPTP *threadParameters) {
   free(threadParameters);
}
