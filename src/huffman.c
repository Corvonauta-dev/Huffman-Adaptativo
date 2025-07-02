/*
 * Implementação do Huffman Adaptativo FGK
 */

#include "huffman.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------
 * Variáveis globais do FGK
 * ------------------------------- */
static No *raiz = NULL, *nyt = NULL, *folha[ALFABETO] = {0};
static struct { FILE* arq; uint8_t buf; int cnt; } bits;
static OrdemBits ordem_bits = MAIS_SIG;

/* ===============================
 * Controle de log detalhado
 * =============================== */
#ifdef FGK_DEBUG_LOG
static FILE *fgk_log = NULL;
static void fgk_log_abrir(void) { fgk_log = fopen("fgk_debug.log", "w"); }
static void fgk_log_fechar(void) { if (fgk_log) fclose(fgk_log); fgk_log = NULL; }
static const char* tipo_no(No *n) {
    return (n->esq == NULL && n->dir == NULL) ? "folha" : "interno";
}
static void print_no(FILE *log, No *n) {
    if (!n) return;
    fprintf(log, "ordem=%d peso=%u tipo=%s ", n->ordem, n->peso, tipo_no(n));
    if (n->esq == NULL && n->dir == NULL)
        fprintf(log, "simbolo='%c'(%d) ", (char)n->simbolo, n->simbolo);
    else
        fprintf(log, "end=%p ", (void*)n);
}
static void print_arvore_rec(FILE *log, No *n, int nivel) {
    if (!n) return;
    for (int i = 0; i < nivel; ++i) fprintf(log, "  ");
    print_no(log, n);
    fprintf(log, "\n");
    print_arvore_rec(log, n->esq, nivel+1);
    print_arvore_rec(log, n->dir, nivel+1);
}
static void print_arvore(FILE *log, No *raiz) {
    fprintf(log, "==== ESTADO ATUAL DA ARVORE ====\n");
    print_arvore_rec(log, raiz, 0);
    fprintf(log, "================================\n");
}
#else
static void fgk_log_abrir(void) {}
static void fgk_log_fechar(void) {}
#define print_arvore(a,b) ((void)0)
#define print_no(a,b)     ((void)0)
#endif

/* ===============================
 * Funções auxiliares de Endianess
 * =============================== */
static inline uint32_t para_little_endian(uint32_t x) {
    union { uint32_t u32; uint8_t b[4]; } teste = { .u32 = 1 };
    if (teste.b[0] == 1) return x;
    return ((x>>24)&0xFF) | ((x>>8)&0xFF00) | ((x<<8)&0xFF0000) | ((x<<24)&0xFF000000);
}
#define le32para_host(x) para_little_endian(x)
#define host_para_le32(x) para_little_endian(x)

/* ===============================
 * Manipulação de Bits (interno)
 * =============================== */
static void bits_iniciar_escrita(FILE*arq) { bits.arq=arq; bits.buf=0; bits.cnt=8; }
static void bits_escreve_bit(int bit) {
    bits.buf = (bits.buf<<1) | (bit&1);
    if (--bits.cnt == 0) { fputc(bits.buf, bits.arq); bits.buf=0; bits.cnt=8; }
}
static void bits_escreve_vetor(uint32_t v, int n) {
    for (int i = n-1; i >= 0; --i) bits_escreve_bit((v>>i)&1);
}
static void bits_flush(void) { while(bits.cnt != 8) bits_escreve_bit(0); }

static void bits_iniciar_leitura(FILE*arq) { bits.arq=arq; bits.buf=0; bits.cnt=0; }
static int bits_le_bit(void) {
    if (bits.cnt == 0) {
        int ch = fgetc(bits.arq); if (ch == EOF) return -1;
        bits.buf = (uint8_t)ch; bits.cnt = 8;
    }
    int bit;
    if (ordem_bits == MAIS_SIG) { bit = (bits.buf>>7)&1; bits.buf<<=1; }
    else { bit = bits.buf&1; bits.buf>>=1; }
    bits.cnt--; return bit;
}
static uint32_t bits_le_vetor(int n) {
    uint32_t v = 0; while (n--) v = (v<<1) | bits_le_bit(); return v;
}

/* ===============================
 * Árvore FGK – implementação
 * =============================== */

/* Cria um novo nó */
static No* novo_no(uint8_t simbolo, uint32_t peso, int32_t ordem) {
    No* n = (No*)calloc(1, sizeof(No));
    if (!n) { fprintf(stderr, "ERRO: Falha de alocação de nó.\n"); exit(1); }
    n->simbolo = simbolo;
    n->peso = peso;
    n->ordem = ordem;
    n->pai = n->esq = n->dir = NULL;
    return n;
}

/* Checa se 'sup' é pai de 'sub' */
static int eh_ancestral(No *sup, No *sub) {
    while (sub) {
        if (sub == sup) return 1;
        sub = sub->pai;
    }
    return 0;
}

/* Busca o nó de maior ordem com dado peso */
static No* no_maior_ordem_com_peso(uint32_t peso) {
    static No* pilha[MAX_NOS]; int t = 0; pilha[t++] = raiz; No* melhor = NULL;
    while (t) {
        No* c = pilha[--t];
        if (c->peso == peso && (!melhor || c->ordem > melhor->ordem)) melhor = c;
        if (c->esq) { pilha[t++] = c->esq; pilha[t++] = c->dir; }
    }
    return melhor;
}

/* Troca dois nós na árvore, nunca troca raiz, nunca troca ancestrais, nunca troca se iguais */
static void troca_nos(No*a, No*b) {
    if (!a || !b) { fprintf(stderr, "ERRO swap: nó nulo\n"); exit(3); }
    if (a == b) return;
    if (!a->pai || !b->pai) return; // Não troca raiz
    if (eh_ancestral(a, b) || eh_ancestral(b, a)) return;
#ifdef FGK_DEBUG_LOG
    if (fgk_log) {
        fprintf(fgk_log, "[SWAP] ");
        print_no(fgk_log, a); fprintf(fgk_log, "<-> "); print_no(fgk_log, b); fprintf(fgk_log, "\n");
    }
#endif
    No **ap = (a->pai->esq == a) ? &a->pai->esq : &a->pai->dir;
    No **bp = (b->pai->esq == b) ? &b->pai->esq : &b->pai->dir;
    *ap = b; *bp = a;
    No* temp = a->pai; a->pai = b->pai; b->pai = temp;
    int32_t ordem_temp = a->ordem; a->ordem = b->ordem; b->ordem = ordem_temp;
}

/* Atualiza a árvore após inserir/usar símbolo */
static void atualiza_arvore(No* n) {
    while (n) {
#ifdef FGK_DEBUG_LOG
        if (fgk_log) {
            fprintf(fgk_log, "[ATUALIZA] ");
            print_no(fgk_log, n); fprintf(fgk_log, "\n");
        }
#endif
        No* lider = no_maior_ordem_com_peso(n->peso);
        // Não pode fazer swap com o próprio nó, nem com pai
        if (lider && lider != n && !eh_ancestral(n, lider)) {
#ifdef FGK_DEBUG_LOG
            if (fgk_log) {
                fprintf(fgk_log, "  -> Lider para swap: ");
                print_no(fgk_log, lider); fprintf(fgk_log, "\n");
            }
#endif
            troca_nos(n, lider);
        }
        n->peso++;
        n = n->pai;
    }
#ifdef FGK_DEBUG_LOG
    if (fgk_log) print_arvore(fgk_log, raiz);
#endif
}

/* Divide o nó NYT para inserir novo símbolo */
static void divide_nyt(uint8_t simbolo) {
#ifdef FGK_DEBUG_LOG
    if (fgk_log) fprintf(fgk_log, "[DIVIDE_NYT] Inserindo novo símbolo '%c' (%d)\n", (char)simbolo, simbolo);
#endif
    No* interno = novo_no(0, 0, nyt->ordem-2);
    No* folha_nova = novo_no(simbolo, 0, nyt->ordem-1);
    nyt->esq = interno; interno->pai = nyt;
    nyt->dir = folha_nova; folha_nova->pai = nyt;
    folha[simbolo] = folha_nova; nyt = interno;
}

/* Escreve o código binário (caminho da raiz até o nó) */
static void escreve_codigo(No* n) {
    uint8_t pilha[512]; int t = 0;
    while (n != raiz) { pilha[t++] = (n == n->pai->dir); n = n->pai; }
    while (t) bits_escreve_bit(pilha[--t]);
}

/* ===============================
 * Inicialização/Destruição da Árvore
 * =============================== */
void fgk_destroi(void) {
    memset(folha, 0, sizeof(folha)); // Sempre limpe ANTES!
    if (!raiz) return;
    No* pilha[MAX_NOS]; int t = 0; pilha[t++] = raiz;
    while (t) {
        No* c = pilha[--t];
        if (c->esq) { pilha[t++] = c->esq; pilha[t++] = c->dir; }
        free(c);
    }
    raiz = nyt = NULL;
}
void fgk_inicializa(void) {
    fgk_destroi();
    raiz = novo_no(0, 0, MAX_NOS);
    nyt = raiz;
}

/* ===============================
 * Compressão
 * =============================== */
int fgk_comprimir(FILE*entrada, FILE*saida) {
    fgk_log_abrir();
    if (!entrada || !saida) {
        fprintf(stderr, "ERRO: Arquivo de entrada ou saída nulo.\n");
        fgk_log_fechar();
        return -1;
    }
    fgk_inicializa(); ordem_bits = MAIS_SIG;

    fseek(entrada, 0, SEEK_END); uint32_t tam = ftell(entrada); fseek(entrada, 0, SEEK_SET);

    /* Escreve cabeçalho little-endian */
    uint32_t le = host_para_le32(tam);
    if (fwrite(&le, 1, 4, saida) != 4) {
        fprintf(stderr, "ERRO: Falha ao escrever cabeçalho.\n");
        fgk_log_fechar();
        return -2;
    }

    bits_iniciar_escrita(saida);
    int ch;
    while ((ch = fgetc(entrada)) != EOF) {
        uint8_t simbolo = (uint8_t)ch; No* n;
        if (!folha[simbolo]) {
            escreve_codigo(nyt);
            bits_escreve_vetor(simbolo, 8);
            divide_nyt(simbolo);
            n = folha[simbolo];
        } else {
            n = folha[simbolo];
            escreve_codigo(n);
        }
        atualiza_arvore(n);
    }
    bits_flush();
#ifdef FGK_DEBUG_LOG
    if (fgk_log) print_arvore(fgk_log, raiz);
#endif
    fgk_log_fechar();
    return 0;
}

/* ===============================
 * Decodificação
 * =============================== */
static uint8_t le_u8(void) { return (uint8_t)bits_le_vetor(8); }

static int verifica_ordem(long pos, FILE*arq, OrdemBits ordem) {
    ordem_bits = ordem; bits_iniciar_leitura(arq); fseek(arq, pos, SEEK_SET);
    No* n = raiz; int prof = 0;
    while (n->esq && prof < 20) {
        int bit = bits_le_bit(); if (bit < 0) return 0;
        n = bit ? n->dir : n->esq; prof++;
    }
    return n == nyt;
}

int fgk_descomprimir(FILE*entrada, FILE*saida) {
    fgk_log_abrir();
    if (!entrada || !saida) {
        fprintf(stderr, "ERRO: Arquivo de entrada ou saída nulo.\n");
        fgk_log_fechar();
        return -1;
    }
    fgk_inicializa();

    uint32_t restante;
    if (fread(&restante, 4, 1, entrada) != 1) {
        fprintf(stderr, "ERRO: Falha ao ler cabeçalho.\n");
        fgk_log_fechar();
        return -2;
    }
    restante = le32para_host(restante);
    long apos_cabecalho = ftell(entrada);

    /* Detecta ordem de bits */
    if (!verifica_ordem(apos_cabecalho, entrada, MAIS_SIG) &&
        !verifica_ordem(apos_cabecalho, entrada, MENOS_SIG)) {
        fprintf(stderr, "ERRO: Ordem de bits não reconhecida.\n");
        fgk_log_fechar();
        return -3;
    }

    fgk_inicializa(); bits_iniciar_leitura(entrada); fseek(entrada, apos_cabecalho, SEEK_SET);

    while (restante) {
        No* n = raiz;
        while (n->esq) {
            int bit = bits_le_bit(); if (bit < 0) {
                fprintf(stderr, "ERRO: Leitura de bit inválida.\n");
                fgk_log_fechar();
                return -4;
            }
            n = bit ? n->dir : n->esq;
        }
        uint8_t simbolo;
        if (n == nyt) { simbolo = le_u8(); divide_nyt(simbolo); n = folha[simbolo]; }
        else simbolo = n->simbolo;
        fputc(simbolo, saida); atualiza_arvore(n); restante--;
    }
#ifdef FGK_DEBUG_LOG
    if (fgk_log) print_arvore(fgk_log, raiz);
#endif
    fgk_log_fechar();
    return 0;
}
