#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Nó de uma árvore binária para representar decisões de atribuição
typedef struct NoArvore
{
    int indice_variavel;       // Índice da variável (0-based)
    struct NoArvore *esquerda; // Subárvore para valor 1
    struct NoArvore *direita;  // Subárvore para valor 0
} NoArvore;

// Estrutura para armazenar fórmula na forma CNF
typedef struct
{
    int **clausulas;      // Array de cláusulas (terminadas com 0)
    int num_clausulas;    // Quantidade total de cláusulas
    int num_variaveis;    // Número de variáveis na fórmula
} FormulaCNF;

// Cria árvore de decisão com todas as combinações possíveis
NoArvore *criar_arvore(int profundidade, int num_variaveis)
{
    if (profundidade >= num_variaveis)
        return NULL;

    NoArvore *no = malloc(sizeof(*no));
    no->indice_variavel = profundidade;
    no->esquerda = criar_arvore(profundidade + 1, num_variaveis); // Valor 1
    no->direita = criar_arvore(profundidade + 1, num_variaveis);  // Valor 0
    return no;
}

// Libera recursivamente a memória da árvore
void liberar_arvore(NoArvore *no)
{
    if (!no)
        return;
    liberar_arvore(no->esquerda);
    liberar_arvore(no->direita);
    free(no);
}

// Verifica se todas as cláusulas estão satisfeitas
bool formula_satisfeita(const FormulaCNF *formula, int atribuicao[])
{
    for (int indice_clausula = 0; indice_clausula < formula->num_clausulas; indice_clausula++)
    {
        int *clausula_atual = formula->clausulas[indice_clausula];
        bool clausula_satisfeita = false;

        // Verifica cada literal da cláusula
        for (int indice_literal = 0; clausula_atual[indice_literal] != 0; indice_literal++)
        {
            int literal = clausula_atual[indice_literal];
            int indice_variavel = abs(literal) - 1; // Converte para índice 0-based

            bool variavel_verdadeira = (atribuicao[indice_variavel] == 1);

            // Literal positivo: variável deve ser 1
            // Literal negativo: variável deve ser 0
            if ((literal > 0 && variavel_verdadeira) || (literal < 0 && !variavel_verdadeira))
            {
                clausula_satisfeita = true;
                break;
            }
        }

        if (!clausula_satisfeita)
            return false;
    }
    return true;
}

// Explora a árvore de decisão usando DFS (primeiro esquerda, depois direita)
bool gerar_com_arvore(
    NoArvore *no_atual,
    int atribuicao[],
    const FormulaCNF *formula,
    bool *solucao_encontrada)
{
    if (*solucao_encontrada)
        return true;

    // Folha da árvore: testa atribuição completa
    if (no_atual == NULL)
    {
        if (formula_satisfeita(formula, atribuicao))
            *solucao_encontrada = true;
        return *solucao_encontrada;
    }

    int indice_variavel = no_atual->indice_variavel;

    // Explora atribuição 1 (subárvore esquerda)
    atribuicao[indice_variavel] = 1;
    gerar_com_arvore(no_atual->esquerda, atribuicao, formula, solucao_encontrada);
    if (*solucao_encontrada)
        return true;

    // Explora atribuição 0 (subárvore direita)
    atribuicao[indice_variavel] = 0;
    gerar_com_arvore(no_atual->direita, atribuicao, formula, solucao_encontrada);

    return *solucao_encontrada;
}

// Lê arquivo CNF no formato DIMACS
FormulaCNF *ler_arquivo_cnf(const char *nome_arquivo)
{
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo)
    {
        perror("Erro ao abrir arquivo");
        return NULL;
    }

    FormulaCNF *formula = malloc(sizeof(*formula));
    formula->num_clausulas = 0;
    formula->num_variaveis = 0;
    formula->clausulas = NULL;

    char linha[256];
    int capacidade_clausulas = 0;
    int contador_clausulas = 0;

    while (fgets(linha, sizeof(linha), arquivo))
    {
        if (linha[0] == 'c') continue;

        if (linha[0] == 'p')
        {
            sscanf(linha, "p cnf %d %d", &formula->num_variaveis, &formula->num_clausulas);
            capacidade_clausulas = formula->num_clausulas;
            formula->clausulas = malloc(sizeof(int *) * capacidade_clausulas);
            continue;
        }

        if (contador_clausulas >= capacidade_clausulas)
        {
            capacidade_clausulas *= 2;
            formula->clausulas = realloc(formula->clausulas, sizeof(int *) * capacidade_clausulas);
        }

        int *literais_clausula = NULL;
        int tamanho_literais = 0;
        int capacidade_literais = 0;

        char *token = strtok(linha, " \n");
        while (token != NULL)
        {
            int literal = atoi(token);
            if (literal == 0) break;

            // Valida intervalo de variáveis
            if (abs(literal) > formula->num_variaveis)
            {
                printf("Erro: Literal %d excede número de variáveis (%d)\n", literal, formula->num_variaveis);
                fclose(arquivo);
                free(literais_clausula);
                free(formula->clausulas);
                free(formula);
                return NULL;
            }

            // Realoca buffer se necessário
            if (tamanho_literais >= capacidade_literais)
            {
                if (capacidade_literais == 0)
                {
                    capacidade_literais = 4;
                }
                else
                {
                    capacidade_literais = capacidade_literais * 2;
                }
                literais_clausula = realloc(literais_clausula, sizeof(int) * capacidade_literais);
            }

            literais_clausula[tamanho_literais++] = literal;
            token = strtok(NULL, " \n");
        }

        if (tamanho_literais > 0)
        {
            literais_clausula = realloc(literais_clausula, sizeof(int) * (tamanho_literais + 1));
            literais_clausula[tamanho_literais] = 0;
            formula->clausulas[contador_clausulas++] = literais_clausula;
        }
    }

    fclose(arquivo);
    formula->num_clausulas = contador_clausulas;
    return formula;
}

// Libera memória alocada para a fórmula
void liberar_formula(FormulaCNF *formula)
{
    for (int indice_clausula = 0; indice_clausula < formula->num_clausulas; indice_clausula++)
    {
        free(formula->clausulas[indice_clausula]);
    }
    free(formula->clausulas);
    free(formula);
}

// Ponto de entrada do programa
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <arquivo.cnf>\n", argv[0]);
        return 1;
    }

    FormulaCNF *formula = ler_arquivo_cnf(argv[1]);
    if (!formula)
        return 1;

    // Fórmula vazia é trivialmente satisfatível
    if (formula->num_clausulas == 0)
    {
        printf("SAT\n");
        liberar_formula(formula);
        return 0;
    }

    // Inicializa atribuições com valor indefinido (-1)
    int *atribuicoes = malloc(sizeof(int) * formula->num_variaveis);
    for (int indice_variavel = 0; indice_variavel < formula->num_variaveis; indice_variavel++)
    {
        atribuicoes[indice_variavel] = -1;
    }

    // Executa busca exaustiva
    NoArvore *arvore_raiz = criar_arvore(0, formula->num_variaveis);
    bool solucao_encontrada = false;
    gerar_com_arvore(arvore_raiz, atribuicoes, formula, &solucao_encontrada);
    liberar_arvore(arvore_raiz);

    // Exibe resultados
    if (solucao_encontrada)
    {
        for (int indice_variavel = 0; indice_variavel < formula->num_variaveis; indice_variavel++)
        {
            printf("x%d = %d\n", indice_variavel + 1, 
                (atribuicoes[indice_variavel] == -1) ? 0 : atribuicoes[indice_variavel]);
        }
        printf("SAT\n");
    }
    else
    {
        printf("UNSAT\n");
    }

    free(atribuicoes);
    liberar_formula(formula);
    return 0;
}