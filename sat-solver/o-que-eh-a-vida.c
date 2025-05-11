#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct NoArvore
{
    int indice_variavel;      
    struct NoArvore *esquerda;
    struct NoArvore *direita;  
} NoArvore;

// FormulaCNF: armazena a lista de cláusulas e parâmetros da fórmula
typedef struct
{
    int **clausulas;   // vetor de cláusulas
    int num_clausulas; // número de cláusulas lidas
    int num_variaveis; // número de variáveis na fórmula
} FormulaCNF;

// --- Cria árvore binária de profundidade 'num_variaveis' ---
// Cada nível corresponde a uma variável, ramificando em verdadeiro (esquerda) e falso (direita)
NoArvore *criar_arvore(int profundidade, int num_variaveis)
{
    // Se já definimos todas as variáveis, não criamos mais nós
    if (profundidade >= num_variaveis)
        return NULL;
    // Aloca um novo nó e define seu índice de variável
    NoArvore *no = malloc(sizeof(*no));
    no->indice_variavel = profundidade;
    // Cria recursivamente as subárvores:
    // esquerda = atribuir valor 1 à variável atual
    no->esquerda = criar_arvore(profundidade + 1, num_variaveis);
    // direita  = atribuir valor 0 à variável atual
    no->direita = criar_arvore(profundidade + 1, num_variaveis);
    return no;
}

// --- Libera memória da árvore ---
void liberar_arvore(NoArvore *no)
{
    if (!no)
        return;
    liberar_arvore(no->esquerda);
    liberar_arvore(no->direita);
    free(no);
}

// --- Verifica se a fórmula inteira está satisfeita ---
// Para cada cláusula, testa se existe um literal que seja verdadeiro na atribuição atual
bool formula_satisfeita(const FormulaCNF *formula, int atribuicao[])
{

    for (int indice_clausula = 0; indice_clausula < formula->num_clausulas; indice_clausula++)
    {
        int *clausula_atual = formula->clausulas[indice_clausula];
        bool clausula_satisfeita = false;

        // Percorre todos os literais até encontrar o marcador de fim (0)
        for (int indice_literal = 0; clausula_atual[indice_literal] != 0; indice_literal++)
        {
            int literal = clausula_atual[indice_literal];
            int indice_variavel = abs(literal) - 1; // Converte literal para índice 0-based

            // Verifica se a variável correspondente ao literal está atribuída como 1 (verdadeira).
            bool variavel_verdadeira = false;
            if (atribuicao[indice_variavel] == 1)
            {
                variavel_verdadeira = true;
            }

            // Verifica se o literal está satisfeito:
            // - Literal positivo: variável deve ser 1
            // - Literal negativo: variável deve ser 0
            if ((literal > 0 && variavel_verdadeira) || (literal < 0 && !variavel_verdadeira))
            {
                clausula_satisfeita = true;
                break; // Cláusula satisfeita, passa para a próxima
            }
        }

        // Se a cláusula atual não foi satisfeita, a fórmula é falsa
        if (!clausula_satisfeita)
            return false;
    }
    return true; // Todas as cláusulas foram satisfeitas
}

// --- Gera atribuições usando a árvore para recursão implícita ---
// Caminha a árvore, atribuindo valor 1 (esquerda) antes de 0 (direita) em cada variável
bool gerar_com_arvore(
    NoArvore *no_atual,
    int atribuicao[],
    const FormulaCNF *formula,
    bool *solucao_encontrada)
{
    // Interrompe apenas se já encontrou uma solução
    if (*solucao_encontrada)
    {
        return *solucao_encontrada;
    }

    // Se chegou a um nó nulo, todas as variáveis foram atribuídas
    if (no_atual == NULL)
    {
        // Verifica se a atribuição atual satisfaz a fórmula completa
        if (formula_satisfeita(formula, atribuicao))
        {
            *solucao_encontrada = true;
        }
        return *solucao_encontrada;
    }

    int indice_variavel = no_atual->indice_variavel;

    // Explora subárvore esquerda (atribui valor 1 à variável atual)
    atribuicao[indice_variavel] = 1;
    gerar_com_arvore(no_atual->esquerda, atribuicao, formula, solucao_encontrada);
    if (*solucao_encontrada)
        return true;

    // Explora subárvore direita (atribui valor 0 à variável atual)
    atribuicao[indice_variavel] = 0;
    gerar_com_arvore(no_atual->direita, atribuicao, formula, solucao_encontrada);

    return *solucao_encontrada;
}

// --- Lê arquivo DIMACS CNF e constrói FormulaCNF ---
// Ignora linhas de comentário 'c', processa header 'p cnf V C', depois cada cláusula terminada em 0
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
    int capacidade_clausulas = 0; // Capacidade inicial do vetor de cláusulas
    int contador_clausulas = 0;   // Número real de cláusulas lidas

    while (fgets(linha, sizeof(linha), arquivo))
    {
        if (linha[0] == 'c')
            continue; // Ignora linhas de comentário

        if (linha[0] == 'p')
        {
            // Lê o cabeçalho 'p cnf' com número de variáveis e cláusulas
            sscanf(linha, "p cnf %d %d", &formula->num_variaveis, &formula->num_clausulas);
            capacidade_clausulas = formula->num_clausulas;
            formula->clausulas = malloc(sizeof(int *) * capacidade_clausulas);
            continue;
        }

        // Realoca o vetor de cláusulas se necessário
        if (contador_clausulas >= capacidade_clausulas)
        {
            capacidade_clausulas *= 2;
            formula->clausulas = realloc(formula->clausulas, sizeof(int *) * capacidade_clausulas);
        }

        // Buffer para armazenar os literais da cláusula atual
        int *literais_clausula = NULL;
        int tamanho_literais = 0;    // Número de literais lidos
        int capacidade_literais = 0; // Capacidade atual do buffer

        // Divide a linha em tokens (literais)
        char *token = strtok(linha, " \n");
        while (token != NULL)
        {
            int literal = atoi(token);
            if (literal == 0)
                break; // Fim da cláusula

            // Validação do literal
            if (abs(literal) > formula->num_variaveis)
            {
                printf("Erro: Literal %d fora do intervalo (max var: %d)\n", literal, formula->num_variaveis);
                fclose(arquivo);
                free(literais_clausula); // Libera o buffer antes de retornar
                free(formula->clausulas);
                free(formula);
                return NULL;
            }

            if (tamanho_literais >= capacidade_literais)
            {
                // Substituição do ternário por if/else
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

        // Adiciona a cláusula à fórmula (se não estiver vazia)
        if (tamanho_literais > 0)
        {
            // Adiciona o marcador de fim (0) e ajusta o tamanho
            literais_clausula = realloc(literais_clausula, sizeof(int) * (tamanho_literais + 1));
            literais_clausula[tamanho_literais] = 0; // Terminador
            formula->clausulas[contador_clausulas++] = literais_clausula;
        }
    }

    fclose(arquivo);
    formula->num_clausulas = contador_clausulas; // Atualiza com o número real
    return formula;
}

// --- Libera memória da fórmula ---
// Libera cada cláusula e o vetor de ponteiros, depois a própria estrutura
void liberar_formula(FormulaCNF *formula)
{
    for (int indice_clausula = 0; indice_clausula < formula->num_clausulas; indice_clausula++)
    {
        free(formula->clausulas[indice_clausula]);
    }
    free(formula->clausulas);
    free(formula);
}

// --- Função principal ---//
int main(int argc, char *argv[])
{

    // Verifica se o nome do arquivo foi fornecido
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <arquivo.cnf>\n", argv[0]);
        return 1;
    }

    // Lê CNF do arquivo "teste2.cnf"
    FormulaCNF *formula = ler_arquivo_cnf(argv[1]);
    if (!formula)
        return 1;

    // Caso SAT trivial: fórmula sem cláusulas é sempre satisfatível
    if (formula->num_clausulas == 0)
    {
        printf("SAT\n");
        liberar_formula(formula);
        return 0;
    }

    // Inicializa vetor de atribuições com -1 (não atribuído)
    int *atribuicoes = malloc(sizeof(int) * formula->num_variaveis);
    for (int indice_variavel = 0; indice_variavel < formula->num_variaveis; indice_variavel++)
    {
        atribuicoes[indice_variavel] = -1;
    }

    // Cria árvore de decisão para explorar combinações
    NoArvore *arvore_raiz = criar_arvore(0, formula->num_variaveis);
    bool solucao_encontrada = false;

    // Realiza busca exaustiva usando a árvore
    gerar_com_arvore(arvore_raiz, atribuicoes, formula, &solucao_encontrada);
    liberar_arvore(arvore_raiz);

    // Exibe resultados
    if (solucao_encontrada)
    {
        for (int indice_variavel = 0; indice_variavel < formula->num_variaveis; indice_variavel++)
        {
            printf("x%d = %d\n",
                   indice_variavel + 1,
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