/* CONSTANTS */
#define ELECTED_NODE 0 // Node eleito, inicialmente, como o TOKEN OWNER.
#define TWAIT 2 // Estimativa (em segundos) de atraso máximo para a presunção de falha.
#define TELEC 4 // Estimativa (em segundos) de atraso máximo para a realização de broadcasting (envio de uma pergunta e recebimento de respostas).

/* MPI MESSAGE TAGS */
#define TAG_IDLE 0 // Este node não vai mais solicitar acesso à CRITICAL SECTION e deseja finalizar a sua execução.
#define TAG_REQUEST 1 // Este node solicitou acesso à CRITICAL SECTION.
#define TAG_TOKEN 2 // Este node recebeu o TOKEN para acessar a CRITICAL SECTION.
#define TAG_CONSULT 3 // Este node enviou, em broadcast, uma mensagem consult para verificar se é o next de algum node da rede.
#define TAG_QUIET 4 // Este node recebeu a confirmação da mensagem consult.
#define TAG_FAILURE 5 // Este node enviou, em broadcast, uma mensagem failure para verificar se o TOKEN está com algum node da rede.
#define TAG_PRESENT 6 // Este node recebeu a confirmação da mensagem failure.
#define TAG_ELECTION 7 // Este node enviou, em broadcast, uma mensagem election para indicar que é candidato a regenerar o TOKEN perdido da rede. 
#define TAG_CANDIDATE_ELECTED 8 // Um node i enviou, em broadcast, uma mensagem candidate_elected confirmando que foi eleito. Todos os outros nodes j da rede deverão receber esta mensagem.
