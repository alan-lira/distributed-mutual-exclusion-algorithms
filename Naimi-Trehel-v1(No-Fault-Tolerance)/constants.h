/* CONSTANTS */
#define NIL -1 // Atributo com valor indefinido para este node.
#define ELECTED_NODE 0 // Node eleito (inicialmente) como o TOKEN OWNER.
#define C_S_PASSAGE_DELAY 2 // Tempo fixo (2 segundos) de acesso à CRITICAL SECTION para todos os nodes.
#define MPI_LOG_FILE_NAME "mpi_log_file.txt" // Arquivo para gerar log de eventos dos nodes.

/* MPI MESSAGE TAGS */
#define TAG_IDLE 0 // Este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
#define TAG_REQUEST 1 // Este node solicitou acesso à CRITICAL SECTION.
#define TAG_TOKEN 2 // Este node recebeu o TOKEN para acessar a CRITICAL SECTION.
