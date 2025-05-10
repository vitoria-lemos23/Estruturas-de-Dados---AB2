#include "algoritmo.h"

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
    printf("\nPressione ENTER para voltar ao menu...");
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
}

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