#define ELECTED_NODE 0 // Node eleito, inicialmente, como o TOKEN OWNER.
#define TAG_FIM 1 // Este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
#define TAG_REQUEST_TOKEN 500 // Este node solicitou acesso à CRITICAL SECTION.
#define TAG_TOKEN 1000 // Este node recebeu o TOKEN para acessar a CRITICAL SECTION.
