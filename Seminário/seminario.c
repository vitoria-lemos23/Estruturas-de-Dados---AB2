#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1000
#define MAX_ARESTAS 10000

typedef struct {
    char tipo;        // 'U' para usuário ou 'F' para filme
    char nome[50];
} Vertex;

typedef struct {
    int origem;
    int destino;
} Aresta;

Vertex grafo[MAX];
Aresta arestas[MAX_ARESTAS];
int total_vertices = 0;
int total_arestas = 0;

int add_vertex(char tipo, char* nome) {
    grafo[total_vertices].tipo = tipo;
    strcpy(grafo[total_vertices].nome, nome);
    return total_vertices++;
}

void add_edge(int user, int movie) {
    arestas[total_arestas].origem = user;
    arestas[total_arestas].destino = movie;
    total_arestas++;
}

void recomendar_filmes(int user_id) {
    int vistos[MAX] = {0};
    int recomendados[MAX] = {0};

    for (int i = 0; i < total_arestas; i++) {
        if (arestas[i].origem == user_id && grafo[arestas[i].destino].tipo == 'F') {
            vistos[arestas[i].destino] = 1;
        }
        if (arestas[i].destino == user_id && grafo[arestas[i].origem].tipo == 'F') {
            vistos[arestas[i].origem] = 1;
        }
    }

    for (int i = 0; i < total_arestas; i++) {
        if ((arestas[i].origem == user_id && grafo[arestas[i].destino].tipo == 'F') ||
            (arestas[i].destino == user_id && grafo[arestas[i].origem].tipo == 'F')) {

            int filme_visto = (grafo[arestas[i].destino].tipo == 'F') ? arestas[i].destino : arestas[i].origem;

            for (int j = 0; j < total_arestas; j++) {
                if (arestas[j].destino == filme_visto && grafo[arestas[j].origem].tipo == 'U' && arestas[j].origem != user_id) {
                    int outro_usuario = arestas[j].origem;

                    for (int k = 0; k < total_arestas; k++) {
                        if (arestas[k].origem == outro_usuario && grafo[arestas[k].destino].tipo == 'F' && !vistos[arestas[k].destino]) {
                            recomendados[arestas[k].destino] = 1;
                        }
                        if (arestas[k].destino == outro_usuario && grafo[arestas[k].origem].tipo == 'F' && !vistos[arestas[k].origem]) {
                            recomendados[arestas[k].origem] = 1;
                        }
                    }
                }
            }
        }
    }

    printf("\nFilmes recomendados para %s:\n", grafo[user_id].nome);
    int encontrou = 0;
    for (int i = 0; i < total_vertices; i++) {
        if (recomendados[i]) {
            printf("- %s\n", grafo[i].nome);
            encontrou = 1;
        }
    }
    if (!encontrou) {
        printf("(Nenhuma recomendação encontrada)\n");
    }
}

void listar_vertices() {
    printf("\n=== Usuários ===\n");
    for (int i = 0; i < total_vertices; i++) {
        if (grafo[i].tipo == 'U') {
            printf("ID: %d - Nome: %s\n", i, grafo[i].nome);
        }
    }
    
    printf("\n=== Filmes ===\n");
    for (int i = 0; i < total_vertices; i++) {
        if (grafo[i].tipo == 'F') {
            printf("ID: %d - Nome: %s\n", i, grafo[i].nome);
        }
    }
}

void pausar() {
    int c;
    printf("\nPressione ENTER duas vezes para voltar ao menu...");
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
}

// Função que lê um arquivo e carrega usuários, filmes e assistidos
void carregar_dados(const char* nome_arquivo) {
    FILE* arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo '%s'\n", nome_arquivo);
        return;
    }

    char tipo;
    char nome[50];
    int user_id, movie_id;

    while (fscanf(arquivo, " %c", &tipo) != EOF) {
        if (tipo == 'U' || tipo == 'F') {
            fscanf(arquivo, " %[^\n]", nome); // Lê o nome até a quebra de linha
            add_vertex(tipo, nome);
        } else if (tipo == 'A') {
            fscanf(arquivo, "%d %d", &user_id, &movie_id);
            add_edge(user_id, movie_id);
        }
    }

    fclose(arquivo);
    printf("Dados carregados com sucesso do arquivo '%s'!\n", nome_arquivo);
    pausar();
}

void menu() {
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║      SISTEMA DE RECOMENDAÇÃO DE FILMES     ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  (1) Adicionar novo usuário                ║\n");
    printf("║  (2) Adicionar novo filme                  ║\n");
    printf("║  (3) Registrar filme assistido por usuário ║\n");
    printf("║  (4) Recomendar filme                      ║\n");
    printf("║  (5) Listar usuários e filmes              ║\n");
    printf("║  (6) Carregar dados de arquivo             ║\n");
    printf("║  (0) Sair                                  ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("Escolha uma opção: ");
}

int main() {
    int opcao;
    char nome[50];
    int user_id, movie_id;
    char nome_arquivo[100];

    do {
        menu();
        scanf("%d", &opcao);
        getchar();

        if (opcao < 0 || opcao > 6) {
            printf("Opção inválida detectada. Encerrando para segurança.\n");
            return 1;
        }
        
        switch (opcao) {
            case 1:
                printf("\nDigite o nome do usuário: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = 0;
                add_vertex('U', nome);
                printf("Usuário adicionado com sucesso!\n");
                pausar();
                break;

            case 2:
                printf("\nDigite o nome do filme: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = 0;
                add_vertex('F', nome);
                printf("Filme adicionado com sucesso!\n");
                pausar();
                break;

            case 3:
                listar_vertices();
                printf("\nDigite o ID do usuário: ");
                scanf("%d", &user_id);
                printf("Digite o ID do filme: ");
                scanf("%d", &movie_id);
                add_edge(user_id, movie_id);
                printf("Registro adicionado com sucesso!\n");
                pausar();
                break;

            case 4:
                listar_vertices();
                printf("\nDigite o ID do usuário para recomendações: ");
                scanf("%d", &user_id);
                recomendar_filmes(user_id);
                pausar();
                break;

            case 5:
                listar_vertices();
                pausar();
                break;

            case 6:
                printf("\nDigite o nome do arquivo para carregar dados: ");
                fgets(nome_arquivo, 100, stdin);
                nome_arquivo[strcspn(nome_arquivo, "\n")] = 0;
                carregar_dados(nome_arquivo);
                break;

            case 0:
                printf("\nSaindo...\n");
                break;
        }
    } while (opcao != 0);

    return 0;
}
