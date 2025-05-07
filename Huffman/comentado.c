#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Constantes
#define TAMANHO_TABELA 256      // Quantidade de símbolos possíveis (ASCII estendido)
#define TAMANHO_BUFFER 4096     // Tamanho do buffer de leitura

// ============================
// ESTRUTURAS DE DADOS
// ============================

// Nó da árvore de Huffman
typedef struct No {
    unsigned char simbolo;           // Símbolo armazenado (usado apenas nas folhas)
    unsigned int frequencia;         // Frequência do símbolo
    struct No *esquerda, *direita;   // Ponteiros para os filhos
    struct No *proximo;              // Usado na lista de prioridade
} No;

// Lista de prioridade (implementada como lista encadeada ordenada)
typedef struct {
    No *inicio;                      // Início da lista
    int tamanho;                     // Quantidade de nós
} ListaPrioridade;

// Estrutura para controle da escrita bit a bit
typedef struct {
    FILE *arquivo;                   // Ponteiro para arquivo de saída
    uint8_t buffer;                  // Buffer para armazenar bits temporários
    int bitsEscritos;                // Quantos bits foram armazenados no buffer
    int totalBits;                   // Total de bits já escritos
} ControladorBits;

// ============================
// FUNÇÕES DE SUPORTE À ÁRVORE
// ============================

// Inicializa lista de prioridade
void inicializarLista(ListaPrioridade *lista) {
    lista->inicio = NULL;
    lista->tamanho = 0;
}

// Insere um nó na lista ordenadamente por frequência
void inserirOrdenado(No *no, ListaPrioridade *lista) {
    if (lista->inicio == NULL || no->frequencia < lista->inicio->frequencia) {
        no->proximo = lista->inicio;
        lista->inicio = no;
    } else {
        No *atual = lista->inicio;
        while (atual->proximo && atual->proximo->frequencia <= no->frequencia)
            atual = atual->proximo;
        no->proximo = atual->proximo;
        atual->proximo = no;
    }
    lista->tamanho++;
}

// Remove e retorna o primeiro nó (menor frequência)
No *removerPrimeiro(ListaPrioridade *lista) {
    if (!lista->inicio) return NULL;
    No *removido = lista->inicio;
    lista->inicio = removido->proximo;
    lista->tamanho--;
    return removido;
}

// Cria um nó folha com símbolo e frequência
No *criarFolha(unsigned char simbolo, unsigned int frequencia) {
    No *novo = calloc(1, sizeof(No));
    novo->simbolo = simbolo;
    novo->frequencia = frequencia;
    return novo;
}

// Cria um nó interno unindo dois nós
No *criarNoInterno(No *esquerdo, No *direito) {
    No *novo = malloc(sizeof(No));
    novo->frequencia = esquerdo->frequencia + direito->frequencia;
    novo->esquerda = esquerdo;
    novo->direita = direito;
    return novo;
}

// Conta a frequência de cada byte no arquivo de entrada
void contarFrequencias(const char *nomeArquivo, unsigned int frequencias[]) {
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if (!arquivo) return;

    unsigned char buffer[TAMANHO_BUFFER];
    size_t bytesLidos;

    memset(frequencias, 0, TAMANHO_TABELA * sizeof(unsigned int));

    while ((bytesLidos = fread(buffer, 1, TAMANHO_BUFFER, arquivo)) > 0) {
        for (size_t i = 0; i < bytesLidos; i++)
            frequencias[buffer[i]]++;
    }
    fclose(arquivo);
}

// Constrói a árvore de Huffman
No *construirArvoreHuffman(unsigned int frequencias[]) {
    ListaPrioridade lista;
    inicializarLista(&lista);

    // Insere os nós folha
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        if (frequencias[i] > 0)
            inserirOrdenado(criarFolha((unsigned char)i, frequencias[i]), &lista);
    }

    // Combina nós até restar só a raiz
    while (lista.tamanho > 1) {
        No *esquerdo = removerPrimeiro(&lista);
        No *direito = removerPrimeiro(&lista);
        inserirOrdenado(criarNoInterno(esquerdo, direito), &lista);
    }

    return lista.inicio;
}

// ============================
// GERAÇÃO DE CÓDIGOS DE HUFFMAN
// ============================

// Gera os códigos de Huffman recursivamente (caminho = código)
void gerarCodigos(No *no, char caminho[], int posicao, char tabelaCodigos[TAMANHO_TABELA][TAMANHO_TABELA]) {
    if (!no->esquerda && !no->direita) {
        caminho[posicao] = '\0'; // finaliza a string do código
        strcpy(tabelaCodigos[no->simbolo], caminho); // salva na tabela
        return;
    }
    if (no->esquerda) {
        caminho[posicao] = '0';
        gerarCodigos(no->esquerda, caminho, posicao + 1, tabelaCodigos);
    }
    if (no->direita) {
        caminho[posicao] = '1';
        gerarCodigos(no->direita, caminho, posicao + 1, tabelaCodigos);
    }
}

// ============================
// CONTROLE DE BITS
// ============================

// Inicializa estrutura de controle bit a bit
void inicializarControlador(ControladorBits *controlador, FILE *arquivo) {
    controlador->arquivo = arquivo;
    controlador->buffer = 0;
    controlador->bitsEscritos = 0;
    controlador->totalBits = 0;
}

// Escreve 1 bit no buffer
void escreverBit(ControladorBits *controlador, int bit) {
    controlador->buffer = (controlador->buffer << 1) | (bit & 1);
    controlador->bitsEscritos++;
    controlador->totalBits++;

    if (controlador->bitsEscritos == 8) {
        fputc(controlador->buffer, controlador->arquivo);
        controlador->buffer = 0;
        controlador->bitsEscritos = 0;
    }
}

// Finaliza escrita (preenche o último byte com 0s)
void finalizarEscrita(ControladorBits *controlador) {
    if (controlador->bitsEscritos > 0) {
        controlador->buffer <<= (8 - controlador->bitsEscritos);
        fputc(controlador->buffer, controlador->arquivo);
    }
}

// ============================
// ESCRITA DA ÁRVORE NO ARQUIVO
// ============================

// Escreve árvore usando pré-ordem
int escreverArvore(No *no, FILE *arquivo) {
    if (!no->esquerda && !no->direita) {
        if (no->simbolo == '*' || no->simbolo == '\\') {
            fputc('\\', arquivo); // escape de símbolos especiais
            fputc(no->simbolo, arquivo);
            return 2;
        } else {
            fputc(no->simbolo, arquivo);
            return 1;
        }
    }
    fputc('*', arquivo);
    int tamEsquerdo = escreverArvore(no->esquerda, arquivo);
    int tamDireito = escreverArvore(no->direita, arquivo);
    return 1 + tamEsquerdo + tamDireito;
}

// ============================
// COMPACTAÇÃO PRINCIPAL
// ============================

void compactarHuffman(const char *nomeEntrada, const char *nomeSaida) {
    unsigned int frequencias[TAMANHO_TABELA] = {0};
    contarFrequencias(nomeEntrada, frequencias);
    No *raiz = construirArvoreHuffman(frequencias);

    char tabelaCodigos[TAMANHO_TABELA][TAMANHO_TABELA] = {{0}};
    char caminhoAtual[TAMANHO_TABELA];
    gerarCodigos(raiz, caminhoAtual, 0, tabelaCodigos);

    FILE *saida = fopen(nomeSaida, "wb");
    if (!saida) return;

    // Reserva dois bytes para o cabeçalho (preenchido depois)
    fputc(0, saida);
    fputc(0, saida);

    // Escreve árvore e obtém seu tamanho
    int tamanhoArvore = escreverArvore(raiz, saida);

    FILE *entrada = fopen(nomeEntrada, "rb");
    if (!entrada) {
        fclose(saida);
        return;
    }

    ControladorBits controlador;
    inicializarControlador(&controlador, saida);

    int caractere;
    while ((caractere = fgetc(entrada)) != EOF) {
        char *codigo = tabelaCodigos[caractere];
        for (int i = 0; codigo[i] != '\0'; i++)
            escreverBit(&controlador, codigo[i] - '0');
    }

    finalizarEscrita(&controlador);

    // Calcula bits de lixo
    int bitsLixo = (8 - (controlador.totalBits % 8)) % 8;

    // Cria o cabeçalho: 3 bits de lixo, 13 bits para o tamanho da árvore
    unsigned short cabecalho = (bitsLixo << 13) | (tamanhoArvore & 0x1FFF);
    fseek(saida, 0, SEEK_SET);
    fputc((cabecalho >> 8) & 0xFF, saida);
    fputc(cabecalho & 0xFF, saida);

    fclose(entrada);
    fclose(saida);
}

// ============================
// DESCOMPACTAÇÃO
// ============================

// Lê cabeçalho: extrai lixo e tamanho da árvore
void lerCabecalho(FILE *arquivo, int *bitsLixo, int *tamanhoArvore) {
    fseek(arquivo, 0, SEEK_SET);
    int byte1 = fgetc(arquivo);
    int byte2 = fgetc(arquivo);
    *bitsLixo = byte1 >> 5;
    *tamanhoArvore = ((byte1 & 0x1F) << 8) | byte2;
}

// Lê a árvore de Huffman em pré-ordem
No *lerNo(FILE *arquivo) {
    int caractere = fgetc(arquivo);
    if (caractere == EOF) return NULL;

    if (caractere == '\\') {
        return criarFolha(fgetc(arquivo), 0);
    } else if (caractere == '*') {
        No *no = malloc(sizeof(No));
        no->esquerda = lerNo(arquivo);
        no->direita = lerNo(arquivo);
        return no;
    } else {
        return criarFolha(caractere, 0);
    }
}

// Lê os bits e decodifica os símbolos
void decodificarBits(FILE *entrada, FILE *saida, No *raiz, int bitsLixo) {
    No *atual = raiz;
    uint8_t byteAtual, proximoByte;
    int finalizado = 0;

    size_t leitura = fread(&byteAtual, 1, 1, entrada);
    if (leitura != 1) return;

    while (!finalizado) {
        size_t leituraProx = fread(&proximoByte, 1, 1, entrada);
        int bitsRestantes = (leituraProx == 1) ? 8 : 8 - bitsLixo;
        finalizado = (leituraProx != 1);

        for (int i = 7; i >= 8 - bitsRestantes; i--) {
            int bit = (byteAtual >> i) & 1;
            atual = (bit == 0) ? atual->esquerda : atual->direita;

            if (!atual->esquerda && !atual->direita) {
                fputc(atual->simbolo, saida);
                atual = raiz;
            }
        }
        byteAtual = proximoByte;
    }
}

// Função principal de descompactação
void descompactarHuffman(const char *nomeEntrada, const char *nomeSaida) {
    FILE *entrada = fopen(nomeEntrada, "rb");
    if (!entrada) return;

    FILE *saida = fopen(nomeSaida, "wb");
    if (!saida) {
        fclose(entrada);
        return;
    }

    int bitsLixo, tamanhoArvore;
    lerCabecalho(entrada, &bitsLixo, &tamanhoArvore);
    No *raiz = lerNo(entrada);
    if (!raiz) {
        fclose(entrada);
        fclose(saida);
        return;
    }

    decodificarBits(entrada, saida, raiz, bitsLixo);

    fclose(entrada);
    fclose(saida);
}

// ============================
// INTERFACE DO USUÁRIO
// ============================

void gerarNomeArquivoComExtensaoHuff(char *arquivoEntrada, char *arquivoSaida) {
    strcpy(arquivoSaida, arquivoEntrada);
    char *extensao = strrchr(arquivoSaida, '.');
    if (extensao) strcpy(extensao, ".huff");
    else strcat(arquivoSaida, ".huff");
}

void exibirInterface() {
    printf("╔════════════════════════════════════════════╗\n");
    printf("║      FERRAMENTA DE COMPACTAÇÃO HUFFMAN     ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  (C) Compactar arquivo                     ║\n");
    printf("║  (D) Descompactar arquivo                  ║\n");
    printf("║  (S) Sair                                  ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("Escolha uma opção: ");
}

void pausar() {
    int c;
    printf("\nPressione ENTER para continuar...");
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
}

// ============================
// FUNÇÃO PRINCIPAL
// ============================

int main() {
    char opcao, arquivoEntrada[256], arquivoSaida[256];
    do {
        system("clear || cls");
        exibirInterface();
        scanf(" %c", &opcao);

        if (opcao == 'C' || opcao == 'c') {
            printf("\nNome do arquivo para compactar: ");
            scanf("%255s", arquivoEntrada);
            gerarNomeArquivoComExtensaoHuff(arquivoEntrada, arquivoSaida);
            compactarHuffman(arquivoEntrada, arquivoSaida);
            printf("\nArquivo compactado como '%s'!\n", arquivoSaida);
            pausar();
        } else if (opcao == 'D' || opcao == 'd') {
            printf("\nNome do arquivo para descompactar: ");
            scanf("%255s", arquivoEntrada);
            printf("Nome do arquivo de saída: ");
            scanf("%255s", arquivoSaida);
            descompactarHuffman(arquivoEntrada, arquivoSaida);
            printf("\nArquivo descompactado como '%s'!\n", arquivoSaida);
            pausar();
        } else if (opcao != 'S' && opcao != 's') {
            printf("\nOpção inválida!\n");
            pausar();
        }
    } while (opcao != 'S' && opcao != 's');

    return 0;
}
