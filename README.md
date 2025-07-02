
# Huffman Adaptativo FGK: Testes Cruzados e Divergências de Implementação

Este repositório apresenta uma implementação em C do algoritmo de Huffman Adaptativo FGK (Faller-Gallager-Knuth), juntamente com um estudo experimental sobre a compatibilidade binária entre diferentes implementações. O projeto é resultado de experimentação prática na disciplina de Estrutura de Dados e foi fundamentado nos principais artigos clássicos da área.

## Visão Geral

O algoritmo de **Huffman Adaptativo** permite a compressão de dados sem conhecimento prévio da distribuição estatística dos símbolos, ajustando dinamicamente sua árvore binária de codificação à medida que lê os dados. Entretanto, pequenas variações nas regras de atualização e codificação podem inviabilizar a interoperabilidade de arquivos entre implementações distintas.

Este projeto inclui:
- Código-fonte modular e bem documentado.
- Logs detalhados para auditoria do funcionamento interno da árvore.
- Exemplos visuais e diagramas gerados a partir do log real da execução.
- Artigo acadêmico descrevendo as dificuldades encontradas em testes cruzados.

## Objetivo

Implementar, auditar e analisar a robustez do algoritmo FGK em testes cruzados, avaliando como pequenas diferenças de interpretação e implementação podem comprometer a compatibilidade entre soluções.

## Estrutura do Repositório

```bash
├── src/
│   ├── huffman.c        # Implementação do algoritmo FGK
│   ├── huffman.h        # Estrutura dos nós, protótipos e macros
│   └── main.c           # Programa principal (interface de linha de comando)
├── T3\_Estrutura\_de\_dados.pdf   # Artigo completo sobre o projeto
├── Makefile             # Compilação facilitada
└── README.md            # Este arquivo

````

## Estrutura dos Dados

A árvore binária utilizada é composta por nós contendo símbolo, peso, ordem (identificador único decrescente), ponteiros para pai e filhos esquerdo/direito. Um vetor de acesso rápido (`folha[ALFABETO]`) mantém o endereço das folhas existentes, acelerando a busca e atualização.

![fig1](https://github.com/user-attachments/assets/f6677a35-d493-4ca1-bf3a-442372e5dc4c)
*Figura 1: Estrutura do nó da árvore FGK.*

## Exemplos Visuais

Abaixo, a árvore real gerada pela implementação após processar a string `"Corvonauta"`:

![fig4](https://github.com/user-attachments/assets/ed7f5f65-9253-468d-826a-d11ac2f70fe3)
*Figura 2: Árvore binária gerada a partir do log da execução FGK.*

## Compilação

O projeto utiliza um `Makefile` para facilitar a compilação nos modos normal, debug e log.

### Compilar no modo normal (sem logs):

```sh
make clean
make
````

### Compilar no modo debug (com logs detalhados):

```sh
make clean
make LOG=1 DEBUG=1
```

* O arquivo de log `fgk_debug.log` será gerado automaticamente na raiz do projeto ao rodar o programa neste modo.
* O log mostra swaps, atualizações e snapshots da árvore a cada passo.
* `ATENÇÃO` usar os logs detalhados em arquivos grandes pode gerar um arquivo de log muito grande estrourando facil a escala de GB para arquivos de 500 MB

## Execução

O executável gerado é `huffbin`.

### Compressão de arquivos

```sh
./huffbin c arquivo_entrada arquivo_saida
```

Exemplo:

```sh
./huffbin c entrada.tar comprimido.huff
```

### Descompressão de arquivos

```sh
./huffbin d arquivo_entrada arquivo_saida
```

Exemplo:

```sh
./huffbin d comprimido.huff saida.tar
```

O programa aceita apenas os comandos `c` (compressão) ou `d` (descompressão), seguidos dos arquivos de entrada e saída.

> **Dica:** Sempre verifique se os arquivos descomprimidos são bit a bit idênticos ao original usando `diff`.

## Resultados Experimentais

O artigo associado detalha como pequenas diferenças de implementação (critérios de swap, ordem dos bits, endianess, manipulação do nó NYT) podem inviabilizar a interoperabilidade entre arquivos comprimidos por diferentes programas. Testes cruzados mostraram que cada escolha diferente nesses pontos dobra o número de variantes, reduzindo exponencialmente a chance de compatibilidade.

### Trecho do artigo

> "Os experimentos realizados mostraram que, mesmo com implementações baseadas nas mesmas referências, pequenas diferenças de interpretação e implementação podem inviabilizar a compatibilidade binária entre soluções. (...) Práticas como logs detalhados, documentação clara e validação sistemática por meio de testes cruzados são essenciais tanto para o desenvolvimento de software robusto quanto para a formação de uma cultura colaborativa em Ciência da Computação."

## Referências

* D. A. Huffman, “A Method for the Construction of Minimum-Redundancy Codes”, Proceedings of the IRE, 1952.
* D. E. Knuth, “Dynamic Huffman Coding”, Journal of Algorithms, 1985.
* J. S. Vitter, “Design and Analysis of Dynamic Huffman Codes”, J. ACM, 1987.
* N. Ziviani, *Projeto de Algoritmos: com implementações em Pascal e C*, 3a. ed., Elsevier, 2010.

---

**Autor:**
Luiz Guilherme Monteiro Padilha
Programa de Pós-Graduação em Ciência da Computação - UTFPR
E-mail: `luipad@alunos.utfpr.edu.br`

---

**Acesse o artigo completo:**
[T3\_Estrutura\_de\_dados.pdf](T3_Estrutura_de_dados.pdf)


