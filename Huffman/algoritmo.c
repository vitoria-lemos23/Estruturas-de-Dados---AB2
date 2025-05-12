#include "algoritmo.h"

// ----------------------------------------------------
// Funções para gerenciamento da lista de prioridade
// ----------------------------------------------------

//INICIALIZA LISTA DE PRIORIDADE
void inicializarLista(ListaPrioridade *lista) {
    lista->inicio = NULL;
    lista->tamanho = 0;
}

//INSERE UM NÓ NA LISTA PARA MANTENDO A ORDENAÇÃO POR FREQUENCIA CRESCENTE
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

//REMOVE E RETORNA O PRIMEIRO NÓ (MENOR FREQUENCIA) DA LISTA
No *removerPrimeiro(ListaPrioridade *lista) {
    if (!lista->inicio) return NULL;
    No *removido = lista->inicio;
    lista->inicio = removido->proximo;
    lista->tamanho--;
    return removido;
}

//CONTA A FREQUENCIA DE CADA SÍMBOLO NO ARQUIVO
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

// ----------------------------------------------------
// Funções para construção da árvore de Huffman
// ----------------------------------------------------

//CRIA UM NÓ FOLHA COM O SÍMBOLO E FREQUENCIA INFORMADO
No *criarFolha(unsigned char simbolo, unsigned int frequencia) {
    No *novo = calloc(1, sizeof(No));
    novo->simbolo = simbolo;
    novo->frequencia = frequencia;
    return novo;
}

//CRIA UM NÓ INTERNO UNINDO DUAS SUBARVORES
No *criarNoInterno(No *esquerdo, No *direito) {
    No *novo = malloc(sizeof(No));
    novo->frequencia = esquerdo->frequencia + direito->frequencia;
    novo->esquerda = esquerdo;
    novo->direita = direito;
    return novo;
}

//CONSTROI A ÁRVORE DE HUFFMAN A A PARTIR DE UM VETOR DE FREQUENCIA
No *construirArvoreHuffman(unsigned int frequencias[]) {
    ListaPrioridade lista;
    inicializarLista(&lista);

    for (int i = 0; i < TAMANHO_TABELA; i++) {
        if (frequencias[i] > 0)
            inserirOrdenado(criarFolha((unsigned char)i, frequencias[i]), &lista);
    }

    while (lista.tamanho > 1) {
        No *esquerdo = removerPrimeiro(&lista);
        No *direito = removerPrimeiro(&lista);
        inserirOrdenado(criarNoInterno(esquerdo, direito), &lista);
    }
    return lista.inicio;
}

//PERCORRE A ÁRVORE E GERA UM CÓDIGO BINÁRIO PARA CADA SÍMBOLO
void gerarCodigos(No *no, char caminho[], int posicao, char tabelaCodigos[TAMANHO_TABELA][TAMANHO_TABELA]) {
    if (!no->esquerda && !no->direita) {
        caminho[posicao] = '\0';
        strcpy(tabelaCodigos[no->simbolo], caminho);
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

// ----------------------------------------------------
// Funções para escrita de bits no arquivo
// ----------------------------------------------------

//INICIALIZA O CONTROLADOR DE BITS PARA ESCRITA EM ARQUIVOS
void inicializarControlador(ControladorBits *controlador, FILE *arquivo) {
    controlador->arquivo = arquivo;
    controlador->buffer = 0;
    controlador->bitsEscritos = 0;
    controlador->totalBits = 0;
}

//ESCREVE UM ÚNICO BIT NO ARQUIVO USANDO BUFFER DE 8 BITS
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

//FINALIZA A ESCRITA PREENCHENDO O BUFFER COM ZERO SE NECESSÁRIO
void finalizarEscrita(ControladorBits *controlador) {
    if (controlador->bitsEscritos > 0) {
        controlador->buffer <<= (8 - controlador->bitsEscritos);
        fputc(controlador->buffer, controlador->arquivo);
    }
}

//SERIALIZA A ÁRVORE DE HUFFMAN EM ARQUIVO USANDO PERCURSO PRÉ-ORDEM
int escreverArvore(No *no, FILE *arquivo) {
    if (!no->esquerda && !no->direita) {
        if (no->simbolo == '*' || no->simbolo == '\\') {
            fputc('\\', arquivo);
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

// ----------------------------------------------------
// Funções principais de compactação e descompactação
// ----------------------------------------------------

//REALIZA A COMPACTAÇÃO DO ARQUIVO UTILIZANDO HUFFMAN
void compactarHuffman(const char *nomeEntrada, const char *nomeSaida) {
    unsigned int frequencias[TAMANHO_TABELA] = {0};
    contarFrequencias(nomeEntrada, frequencias);

    No *raiz = construirArvoreHuffman(frequencias);

    char tabelaCodigos[TAMANHO_TABELA][TAMANHO_TABELA] = {{0}};
    char caminhoAtual[TAMANHO_TABELA];
    gerarCodigos(raiz, caminhoAtual, 0, tabelaCodigos);

    FILE *saida = fopen(nomeSaida, "wb");
    if (!saida) return;

    fputc(0, saida);
    fputc(0, saida);

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
    int bitsLixo = (8 - (controlador.totalBits % 8)) % 8;
    unsigned short cabecalho = (bitsLixo << 13) | (tamanhoArvore & 0x1FFF);

    fseek(saida, 0, SEEK_SET);
    fputc((cabecalho >> 8) & 0xFF, saida);
    fputc(cabecalho & 0xFF, saida);

    fclose(entrada);
    fclose(saida);
}

//LÊ O CABEÇALHO DE UM ARQUIVO COMPACTADO, EXTRAINDO INFORMAÇÕES DO CONTROLE
void lerCabecalho(FILE *arquivo, int *bitsLixo, int *tamanhoArvore) {
    fseek(arquivo, 0, SEEK_SET);
    int byte1 = fgetc(arquivo);
    int byte2 = fgetc(arquivo);
    *bitsLixo = byte1 >> 5;
    *tamanhoArvore = (byte1 & 0x1F) << 8 | byte2;
}

//RECONSTROI A ÁRVORE A PARTIR DE ARQUIVOS SEREALIZADOS
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

//DECODIFICA DADOS COMPACTADOS EM BITS, ATRAVESSANDO A ÁRVORE DE HUFFMAN
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

// Descompacta um arquivo Huffman, lendo do arquivo de entrada e escrevendo no arquivo de saída
void descompactarHuffman(const char *nomeEntrada, const char *nomeSaida) {
    // Abre o arquivo compactado para leitura binária
    FILE *entrada = fopen(nomeEntrada, "rb");
    if (!entrada) 
        return;  // Se não conseguir abrir, aborta

    // Abre/Cria o arquivo de saída para escrita binária
    FILE *saida = fopen(nomeSaida, "wb");
    if (!saida) {
        fclose(entrada);
        return;  // Se falhar, garante fechar o de entrada antes de sair
    }

    // Lê os dois primeiros bytes e extrai:
    //  - bitsLixo: quantos zeros foram adicionados no último byte
    //  - tamanhoArvore: quantos bytes a serialização da árvore ocupa
    int bitsLixo, tamanhoArvore;
    lerCabecalho(entrada, &bitsLixo, &tamanhoArvore);

    // Reconstrói a árvore de Huffman a partir dos próximos bytes (percurso pré-ordem)
    No *raiz = lerNo(entrada);
    if (!raiz) {
        // Se não conseguir ler a árvore, aborta e fecha arquivos
        fclose(entrada);
        fclose(saida);
        return;
    }

    // Percorre o fluxo de bits restante, decodificando cada bit
    // e escrevendo símbolo a símbolo no arquivo de saída
    decodificarBits(entrada, saida, raiz, bitsLixo);

    // Fecha ambos os arquivos ao final da operação
    fclose(entrada);
    fclose(saida);
}

//GERA AUTOMATICAMENTE O NOME DE SAIDA .HUFF
void gerarNomeArquivoComExtensaoHuff(char *entrada, char *saida) {
    const char *ext = strrchr(entrada, '.');
    size_t len = ext ? (size_t)(ext - entrada) : strlen(entrada);
    memcpy(saida, entrada, len);
    strcpy(saida + len, ".huff");
}



