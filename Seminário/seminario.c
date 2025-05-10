#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1000

// Estrutura que representa um nó da lista de adjacência.
// Cada vértice do grafo terá uma lista de vértices conectados a ele.
typedef struct adj_list {
    int item;
    struct adj_list* next;
} adj_list;

// Estrutura do grafo principal.
typedef struct {
    adj_list* vertices[MAX];  // Lista de adjacência para cada vértice
    short visited[MAX];       // Vetor de visitados (pode ser usado em BFS/DFS)
    char tipo[MAX];           // Tipo do vértice: 'U' para usuário, 'F' para filme
    char nome[MAX][50];       // Nome associado a cada vértice
    int total_vertices;       // Quantidade atual de vértices
} graph;

// Função que aloca e inicializa um grafo vazio
graph* create_graph() {
    graph* g = malloc(sizeof(graph));

    // Inicializa todos os ponteiros da lista de adjacência como NULL
    // E o vetor de visitados como 0
    for (int i = 0; i < MAX; i++) {
        g->vertices[i] = NULL;
        g->visited[i] = 0;
    }

    g->total_vertices = 0;
    return g;
}

// Adiciona um novo vértice (usuário ou filme) ao grafo
int add_vertex(graph* g, char tipo, const char* nome) {
    int id = g->total_vertices++; // ID será o índice atual (quantidade de vertices já existentes, depois incrementa )
    g->tipo[id] = tipo;           // Define o tipo do vértice no indice correspondente ao mesmo no vetor de tipos do grafo
    strcpy(g->nome[id], nome);    // Copia o nome para o vetor de nomes do grafo, na posição correspondente ao indice
    return id;                    // Retorna o ID do novo vértice
}

// Cria uma aresta entre dois vértices (ligação bidirecional entre usuário e filme)
// É chamada quando o usuario adiciona um registro de filme assistido
void add_edge(graph* g, int origem, int destino) {
    adj_list* novo = malloc(sizeof(adj_list)); // Cria novo nó da lista
    novo->item = destino;                      // O item desse nó vai ser o id do filme assistido
    novo->next = g->vertices[origem];          // Se a lista estiver vazia, novo->next aponta para null, caso não, novo->next vai ser a cabeça da lista
    g->vertices[origem] = novo; // Agora o vetor com o indice do usuário, aponta para o novo nó
}
// OBS.: o mesmo processo é feito para origem e destino trocando de posição, para que o grafo seja bidirecional

// Lista os vértices do grafo, separando usuários e filmes
void listar_vertices(graph* g) {
    printf("\n=== Usuários ===\n");

    // Percorre todos os vértices e imprime os que são usuários
    for (int i = 0; i < g->total_vertices; i++) {
        if (g->tipo[i] == 'U')
            printf("ID: %d - Nome: %s\n", i, g->nome[i]);
    }

    printf("\n=== Filmes ===\n");

    // Percorre todos os vértices e imprime os que são filmes
    for (int i = 0; i < g->total_vertices; i++) {
        if (g->tipo[i] == 'F')
            printf("ID: %d - Nome: %s\n", i, g->nome[i]);
    }
}

// Gera recomendações de filmes para um usuário com base em outros usuários
void recomendar_filmes(graph* g, int user_id) {
    short vistos[MAX] = {0};        // Vetor de filmes já assistidos pelo usuário
    short recomendados[MAX] = {0};  // Vetor de filmes a recomendar

    // Identifica os filmes que o usuário já assistiu 
    for (adj_list* a = g->vertices[user_id]; a != NULL; a = a->next) {
        // Percorre todos os nós da lista desse usuário
        if (g->tipo[a->item] == 'F') {
            // Se o tipo do nó for um filme, adiciona no vetor de vistos
            vistos[a->item] = 1;
        }
    }

    // Verificar cada filme que o usuário viu
    for (int i = 0; i < g->total_vertices; i++) {
        // Percorre todos os vértices do grafo
        if (vistos[i]) {
            // Se esse vertice está no vetor de vistos, procurar todos os usuários que também viram esse filme
            for (adj_list* a = g->vertices[i]; a != NULL; a = a->next) {
                // Percorre a lista ligada a esse filme
                int outro_usuario = a->item;
                // Os itens dessa lista (do vertice filme) serão os outros usuarios que viram o filme

                // Verifica se é um usuário diferente do atual
                if (g->tipo[outro_usuario] == 'U' && outro_usuario != user_id) {

                    // Para cada filme que esse outro usuário assistiu, percorre a lista desse outro usuario
                    for (adj_list* b = g->vertices[outro_usuario]; b != NULL; b = b->next) {
                        int possivel = b->item;
                        //Os itens vão ser as outras possibilidades de filme

                        // Se for filme e ainda não foi visto, marcar como recomendação
                        if (g->tipo[possivel] == 'F' && !vistos[possivel]) {
                            recomendados[possivel] = 1;
                        }
                    }
                }
            }
        }
    }

    // Imprimir as recomendações
    printf("\nFilmes recomendados para %s:\n", g->nome[user_id]);
    int encontrou = 0;

    // Percorre todos os vértices e imprime os marcados como recomendados
    for (int i = 0; i < g->total_vertices; i++) {
        if (recomendados[i]) {
            printf("- %s\n", g->nome[i]);
            encontrou = 1;
        }
    }

    // Caso nenhuma recomendação tenha sido encontrada
    if (!encontrou)
        printf("(Nenhuma recomendação encontrada)\n");
}

// Carrega dados iniciais de um arquivo
void carregar_dados(graph* g, const char* nome_arquivo) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo '%s'\n", nome_arquivo);
        return;
    }

    char tipo;
    char nome[50];
    int user_id, movie_id;

    // Lê o arquivo linha por linha
    while (fscanf(arquivo, " %c", &tipo) != EOF) {
        if (tipo == 'U' || tipo == 'F') {
            fscanf(arquivo, " %[^\n]", nome); // Lê nome com espaços
            add_vertex(g, tipo, nome);        // Adiciona ao grafo
        } else if (tipo == 'A') {
            fscanf(arquivo, "%d %d", &user_id, &movie_id);
            add_edge(g, user_id, movie_id);   // Ligação usuário-filme
            add_edge(g, movie_id, user_id);   // Ligação filme-usuário
        }
    }

    fclose(arquivo);
    printf("Dados carregados com sucesso!\n");
}

// Função auxiliar para pausar a execução
void pausar() {
    int c;
    printf("\nPressione ENTER duas vezes para voltar ao menu...");
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar(); // Aguarda segunda tecla ENTER
}

// Menu do sistema
void menu() {
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║      SISTEMA DE RECOMENDAÇÃO DE FILMES     ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  (1) Adicionar novo usuário                ║\n");
    printf("║  (2) Adicionar novo filme                  ║\n");
    printf("║  (3) Registrar filme assistido             ║\n");
    printf("║  (4) Recomendar filmes                     ║\n");
    printf("║  (5) Listar usuários e filmes              ║\n");
    printf("║  (6) Carregar dados de arquivo             ║\n");
    printf("║  (0) Sair                                  ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("Escolha uma opção: ");
}

// Função principal do programa
int main() {
    graph* g = create_graph(); // Cria o grafo principal
    int opcao;
    char nome[50];
    int user_id, movie_id;
    char nome_arquivo[100];

    do {
        menu();            // Exibe o menu
        scanf("%d", &opcao);
        getchar();      

        switch (opcao) {
            case 1:
                // Adiciona novo usuário
                printf("\nNome do usuário: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = 0;
                add_vertex(g, 'U', nome);
                printf("Usuário adicionado com sucesso!\n");
                pausar();
                break;

            case 2:
                // Adiciona novo filme
                printf("\nNome do filme: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = 0;
                add_vertex(g, 'F', nome);
                printf("Filme adicionado com sucesso!\n");
                pausar();
                break;

            case 3:
                // Registra que um usuário assistiu um filme
                listar_vertices(g);
                printf("\nID do usuário: ");
                scanf("%d", &user_id);
                printf("ID do filme: ");
                scanf("%d", &movie_id);
                add_edge(g, user_id, movie_id);
                add_edge(g, movie_id, user_id);
                printf("Registro realizado!\n");
                pausar();
                break;

            case 4:
                // Recomenda filmes para um usuário
                listar_vertices(g);
                printf("\nID do usuário para recomendação: ");
                scanf("%d", &user_id);
                recomendar_filmes(g, user_id);
                pausar();
                break;

            case 5:
                // Lista todos os usuários e filmes
                listar_vertices(g);
                pausar();
                break;

            case 6:
                // Carrega dados de um arquivo texto
                printf("\nNome do arquivo: ");
                fgets(nome_arquivo, 100, stdin);
                nome_arquivo[strcspn(nome_arquivo, "\n")] = 0;
                carregar_dados(g, nome_arquivo);
                pausar();
                break;

            case 0:
                printf("\nSaindo...\n");
                break;

            default:
                printf("Opção inválida.\n");
        }
    } while (opcao != 0); // Continua até o usuário digitar 0 (sair)

    return 0;
}
