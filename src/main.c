/*
 * Compilar no modo normal (sem logs): make
 * Compilar no modo debug (com logs detalhados): make LOG=1 DEBUG=1
 * ATENÇÃO usar os logs detalhados em arquivos grandes pode gerar um arquivo de log muito grande estrourando facil a escala de GB para arquivos de 500 MB
 * Programa principal para compressão/descompressão usando FGK.
 * Uso: ./huffbin [c|d] <entrada> <saida>
 * Exempolo: ./huffbin c entrada.tar comprimido.huff
 *           ./huffbin d comprimido.huff saida.tar
 * Para informações mais completas e detalhadas acesse: https://github.com/Corvonauta-dev/Huffman-Adaptativo
 */
#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>

static void uso(const char *prog) {
    fprintf(stderr, "Uso: %s c|d arquivo_entrada arquivo_saida\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc != 4) { uso(argv[0]); return 1; }
    const char *modo = argv[1];
    FILE *entrada  = fopen(argv[2], "rb");
    FILE *saida = fopen(argv[3], "wb");
    if (!entrada || !saida) { perror("fopen"); return 1; }

    int ret = 0;
    if (modo[0] == 'c') {
        ret = fgk_comprimir(entrada, saida);
    } else if (modo[0] == 'd') {
        ret = fgk_descomprimir(entrada, saida);
    } else {
        uso(argv[0]); ret = 1;
    }

    fclose(entrada); fclose(saida);
    fgk_destroi();
    return ret;
}
