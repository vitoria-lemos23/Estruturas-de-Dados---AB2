#ifndef ALGORITMO_H
#define ALGORITMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TAMANHO_TABELA 256
#define TAMANHO_BUFFER 4096

typedef struct No {
    unsigned char simbolo;
    unsigned int frequencia;
    struct No *esquerda, *direita, *proximo;
} No;

typedef struct {
    No *inicio;
    int tamanho;
} ListaPrioridade;

typedef struct {
    FILE *arquivo;
    uint8_t buffer;
    int bitsEscritos;
    int totalBits;
} ControladorBits;

// ----------------------------------------------------
// Funções para gerenciamento da lista de prioridade
// ----------------------------------------------------

void inicializarLista(ListaPrioridade *lista);
void inserirOrdenado(No *no, ListaPrioridade *lista);
No *removerPrimeiro(ListaPrioridade *lista);
void contarFrequencias(const char *nomeArquivo, unsigned int frequencias[]);

// ----------------------------------------------------
// Funções para construção da árvore de Huffman
// ----------------------------------------------------

No *criarFolha(unsigned char simbolo, unsigned int frequencia);
No *criarNoInterno(No *esquerdo, No *direito);
No *construirArvoreHuffman(unsigned int frequencias[]);
void gerarCodigos(No *no, char caminho[], int posicao, char tabelaCodigos[TAMANHO_TABELA][TAMANHO_TABELA]);

// ----------------------------------------------------
// Funções para escrita de bits no arquivo
// ----------------------------------------------------

void inicializarControlador(ControladorBits *controlador, FILE *arquivo);
void escreverBit(ControladorBits *controlador, int bit);
void finalizarEscrita(ControladorBits *controlador);
int escreverArvore(No *no, FILE *arquivo);

// ----------------------------------------------------
// Funções principais de compactação e descompactação
// ----------------------------------------------------

void compactarHuffman(const char *nomeEntrada, const char *nomeSaida);
void lerCabecalho(FILE *arquivo, int *bitsLixo, int *tamanhoArvore);
No *lerNo(FILE *arquivo);
void decodificarBits(FILE *entrada, FILE *saida, No *raiz, int bitsLixo);
void descompactarHuffman(const char *nomeEntrada, const char *nomeSaida);
void gerarNomeArquivoComExtensaoHuff(char *entrada, char *saida);

#endif