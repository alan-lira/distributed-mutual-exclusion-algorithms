#include <mpi.h>
#include <stdio.h>
#include "constants.h"

void start_mpi_log_environment(MPI_File *logFile) {

   MPI_File_open(MPI_COMM_WORLD, MPI_LOG_FILE_NAME, MPI_MODE_WRONLY | MPI_MODE_CREATE | MPI_MODE_APPEND, MPI_INFO_NULL, logFile);

}

void write_mpi_log_event(MPI_File *logFile, char *logBuffer) {

   char *aux1LogBuffer = logBuffer;

   char aux2LogBuffer[2048];

   int log_event_index = log_file_line_counter() + 1;

   sprintf(aux2LogBuffer, "[EVENTO %d] ===> ", log_event_index);

   char *logBufferIndexed = malloc(sizeof(char) * (strlen(aux1LogBuffer) + strlen(aux2LogBuffer) + 10));

   strcpy(logBufferIndexed, aux2LogBuffer);
   strcat(logBufferIndexed, aux1LogBuffer);

   MPI_File log = *logFile;

   //MPI_File_write_shared is a blocking routine that uses the shared file pointer to write files. The order of serialization is not deterministic for this noncollective routine.
   MPI_File_write_shared(log, logBufferIndexed, strlen(logBufferIndexed), MPI_CHAR, MPI_STATUS_IGNORE);

   if (logBufferIndexed) {

      free(logBufferIndexed);

   }

}

void close_mpi_log_environment(MPI_File *logFile) {

   MPI_File_close(logFile);

}

int log_file_line_counter() {

   FILE *fp = NULL;

   int lineCounter = 0;

   char filename[100], c;

   fp = fopen(MPI_LOG_FILE_NAME, "r");

   if (fp == NULL) {

      return 0;

   }

   for (c = getc(fp); c != EOF; c = getc(fp)) {

      if (c == '\n') {

         lineCounter = lineCounter + 1;

      }

   }

   fclose(fp);

   return lineCounter;

}
