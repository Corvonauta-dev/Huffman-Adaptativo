#ifndef HUFFMAN_FGK_H
#define HUFFMAN_FGK_H

#include <stdint.h>
#include <stdio.h>

/* -------------------------------
 * Definições de Constantes
 * ------------------------------- */
#define ALFABETO      256
#define MAX_NOS    (2*ALFABETO - 1)

/* -------------------------------
 * Estrutura do Nó da Árvore FGK
 * ------------------------------- */
typedef struct No {
    uint8_t simbolo;             // Símbolo do nó (válido só em folha)
    uint32_t peso;               // Peso/frequência do nó
    int32_t ordem;               // Número de ordem para manter a propriedade dos irmãos
    struct No *pai;              // Ponteiro para o nó pai
    struct No *esq, *dir;        // Ponteiros para filhos esquerdo e direito
} No;

/* -------------------------------
 * Enumeração para ordem dos bits
 * ------------------------------- */
typedef enum { MAIS_SIG, MENOS_SIG } OrdemBits;

/* -------------------------------
 * Funções públicas da FGK
 * ------------------------------- */

/**
 * Inicializa a árvore FGK (cria raiz e NYT).
 */
void fgk_inicializa(void);

/**
 * Libera toda a memória alocada pela árvore.
 */
void fgk_destroi(void);

/**
 * Comprime um arquivo usando o algoritmo FGK.
 * Retorna 0 em caso de sucesso, negativo em erro.
 */
int fgk_comprimir(FILE *entrada, FILE *saida);

/**
 * Descomprime um arquivo usando o algoritmo FGK.
 * Retorna 0 em caso de sucesso, negativo em erro.
 */
int fgk_descomprimir(FILE *entrada, FILE *saida);

#endif // HUFFMAN_FGK_H
