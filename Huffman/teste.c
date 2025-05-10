#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "algoritmo.h"

// Protótipo da função de geração de nome com extensão .huff (do main.c)
void gerarNomeArquivoComExtensaoHuff(char *arquivoEntrada, char *arquivoSaida);

//TESTA A CRIAÇÃO DAS FOLHAS
static void test_criarFolha() {
    No *folha = criarFolha('X', 5);
    assert(folha != NULL);
    assert(folha->simbolo == (unsigned char)'X');
    assert(folha->frequencia == 5);
    assert(folha->esquerda == NULL && folha->direita == NULL);
    free(folha);
}

//TESTA A CRIAÇÃO DE NÓS INTERNOS
static void test_criarNoInterno() {
    No *esq = criarFolha('A', 2);
    No *dir = criarFolha('B', 3);
    No *interno = criarNoInterno(esq, dir);
    assert(interno != NULL);
    assert(interno->frequencia == 5);
    assert(interno->esquerda == esq);
    assert(interno->direita == dir);
    free(interno);
    // As folhas esq e dir serão liberadas no teste de criarFolha
}

//TESTA A CONTAGEM DE FREQUENCIA A PARTIR DE ARQUIVO TEMPORÁRIO
static void test_contarFrequencias() {
    const char *nome = "freq_test.txt";
    FILE *f = fopen(nome, "wb");
    assert(f != NULL);
    fputs("ABA", f);
    fclose(f);

    unsigned int frequencias[TAMANHO_TABELA] = {0};
    contarFrequencias(nome, frequencias);
    assert(frequencias[(unsigned char)'A'] == 2);
    assert(frequencias[(unsigned char)'B'] == 1);
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        if (i != 'A' && i != 'B') assert(frequencias[i] == 0);
    }
    remove(nome);
}

//TESTA A CONSTRUÇÃO DA ARVORE E GERAÇÃO DE CÓDIGOS PARA 2 SÍMBOLOS
static void test_construirArvoreHuffman_e_gerarCodigos() {
    unsigned int freq[TAMANHO_TABELA] = {0};
    freq[(unsigned char)'A'] = 2;
    freq[(unsigned char)'B'] = 3;
    No *raiz = construirArvoreHuffman(freq);
    assert(raiz != NULL);
    assert(raiz->frequencia == 5);
    assert(raiz->esquerda->simbolo == (unsigned char)'A');
    assert(raiz->direita->simbolo == (unsigned char)'B');

    char tabela[TAMANHO_TABELA][TAMANHO_TABELA] = {{0}};
    char caminho[256] = {0};
    gerarCodigos(raiz, caminho, 0, tabela);
    assert(strcmp(tabela[(unsigned char)'A'], "0") == 0);
    assert(strcmp(tabela[(unsigned char)'B'], "1") == 0);

    // Liberação simplificada da árvore
    free(raiz->esquerda);
    free(raiz->direita);
    free(raiz);
}

//TESTA A SERIALIZAÇÃO PRÉ ORDEM DÁ ÁRVORE
static void test_escreverArvore() {
    unsigned int freq[TAMANHO_TABELA] = {0};
    freq[(unsigned char)'A'] = 1;
    freq[(unsigned char)'B'] = 1;
    No *raiz = construirArvoreHuffman(freq);

    const char *nome = "arvore_test.bin";
    FILE *f = fopen(nome, "wb");
    assert(f != NULL);
    int tamanho = escreverArvore(raiz, f);
    fclose(f);

    FILE *g = fopen(nome, "rb");
    assert(g != NULL);
    char buffer[4] = {0};
    size_t lidos = fread(buffer, 1, tamanho, g);
    fclose(g);
    assert(lidos == (size_t)tamanho);
    assert(buffer[0] == '*');
    // Verifica sequência de folhas A e B em qualquer ordem
    assert((buffer[1] == 'A' && buffer[2] == 'B') || (buffer[1] == 'B' && buffer[2] == 'A'));
    remove(nome);

    free(raiz->esquerda);
    free(raiz->direita);
    free(raiz);
}

//TESTA A GERAÇÃO DE NOME DE ARQUIVO COM .HUFF
static void test_gerarNomeArquivoComExtensaoHuff() {
    char entrada1[256] = "arquivo.txt";
    char saida1[256] = {0};
    gerarNomeArquivoComExtensaoHuff(entrada1, saida1);
    assert(strcmp(saida1, "arquivo.huff") == 0);

    char entrada2[256] = "arquivo";
    char saida2[256] = {0};
    gerarNomeArquivoComExtensaoHuff(entrada2, saida2);
    assert(strcmp(saida2, "arquivo.huff") == 0);

    char entrada3[256] = "a.b.c";
    char saida3[256] = {0};
    gerarNomeArquivoComExtensaoHuff(entrada3, saida3);
    assert(strcmp(saida3, "a.b.huff") == 0);
}

int main() {
    test_criarFolha();
    test_criarNoInterno();
    test_contarFrequencias();
    test_construirArvoreHuffman_e_gerarCodigos();
    test_escreverArvore();
    test_gerarNomeArquivoComExtensaoHuff();
    printf("Todos os testes passaram!\n");
    return 0;
}
