#include <stddef.h>

#include "node.h"
#include "thread-mpi-message-processing-parameters.h"

s_TMPIMPP *initialize_thread_parameters(void) {
   return NULL;
}

s_TMPIMPP *create_thread_parameters(s_N *node, int nodeCount) {
   s_TMPIMPP *newThreadParameters = (s_TMPIMPP*) malloc(sizeof(s_TMPIMPP));
   newThreadParameters->node = node;
   newThreadParameters->nodeCount = nodeCount;
   return newThreadParameters;
}

void destroy_thread_parameters(s_TMPIMPP *threadParameters) {
   free(threadParameters);
}
