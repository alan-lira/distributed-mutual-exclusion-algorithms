
void start_mpi_log_environment(MPI_File *logFile);

void write_mpi_log_event(MPI_File *logFile, char *logBuffer);

void close_mpi_log_environment(MPI_File *logFile);

int log_file_line_counter();
