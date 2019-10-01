#define ELECTED_NODE 0 // Node eleito, inicialmente, como o TOKEN OWNER.
#define TWAIT 2 // Estimativa (em segundos) do atraso máximo para a presunção de falha.
#define TELEC 4 // Estimativa (em segundos) do atraso máximo para realizar broadcasting (envio de uma pergunta e recebimento de respostas).
#define TAG_IDLE 0 // Este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
#define TAG_REQUEST 1 // Este node solicitou acesso à CRITICAL SECTION.
#define TAG_TOKEN 2 // Este node recebeu o TOKEN para acessar a CRITICAL SECTION.
