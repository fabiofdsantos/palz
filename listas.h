/**
 * @file listas.h
 * @brief Listas genericas
 *
 * Conjunto de funcoes para acesso a listas genericas. Consultar exemplos fornecidos.
 *
 * @Vitor Carreira
 * @date Abril 2004
 * @version 1.3 (fix lista_inserir_inicio)
 */
#ifndef _LISTAS_H
#define _LISTAS_H

/**
 * Declaracao do tipo que representa o ponteiro para uma funcao que se aplica a um elemento de uma lista
 */
typedef void (*APLICAR_FUNC) (void* A);

typedef APLICAR_FUNC LIBERTAR_FUNC;


/**
 * Declaracao do tipo que representa o ponteiro para uma funcao de comparacao. Recebe 2 elementos de uma lista
 * e devolve:
 *   < 0 se A < B
 *   0   se A = B
 *   > 0 se A > B
 */
typedef int (*COMPARAR_FUNC) (void* A, void* B);

typedef struct no {
	void* elem;
	struct no *ant, *prox;
} NO_T;

typedef struct lg {
	NO_T* base;
	int numero_elementos;
	APLICAR_FUNC liberta_memoria;
} LISTA_GENERICA_T;


typedef struct iterador {
	NO_T* base;
	NO_T* actual;
} ITERADOR_T;



/**
 * Funcao que cria uma lista generica
 * @param liberta_elem ponteiro para uma funcao que liberta a memoria de um elemento da lista
 * @return ponteiro para a lista criada
 */
LISTA_GENERICA_T* lista_criar(LIBERTAR_FUNC liberta_elem);

/**
 * Funcao que insere um elemento na lista. O elemento e' inserido no final da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir(LISTA_GENERICA_T* lista, void* elem);

/**
 * Funcao que insere um elemento no inicio da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir_inicio(LISTA_GENERICA_T* lista, void* elem);

/**
 * Funcao que insere um elemento no final da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir_fim(LISTA_GENERICA_T* lista, void* elem);


/**
 * Funcao que remove um elemento da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a remover
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso o elemento nao existe
 */
void* lista_remover(LISTA_GENERICA_T* lista, void* elem);

/**
 * Funcao que remove o elemento que se encontra no inicio da lista.
 * @param lista ponteiro para a lista generica
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso a lista se encontre vazia
 */
void* lista_remover_inicio(LISTA_GENERICA_T* lista);

/**
 * Funcao que remove o elemento que se encontra no fim da lista.
 * @param lista ponteiro para a lista generica
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso a lista se encontre vazia
 */
void* lista_remover_fim(LISTA_GENERICA_T* lista);

/**
 * Funcao que remove todos os elementos da lista.
 * @param lista ponteiro para a lista generica
 */
void lista_remover_todos(LISTA_GENERICA_T* lista);


/**
 * Funcao que devolve o numero de elementos da lista.
 * @param lista ponteiro para a lista generica
 * @return o numero de elementos na lista
 */
int lista_numero_elementos(LISTA_GENERICA_T* lista);


/**
 * Funcao que destroi a lista.
 * @param lista ponteiro para a lista generica (passado por refer�ncia)
 */
void lista_destruir(LISTA_GENERICA_T** lista);

/**
 * Funcao que pesquisa a lista 'a procura de um elemento.
 * @param lista ponteiro para a lista generica
 * @param elem elemento a procurar (apenas os campos utilizados pela funcao de
 * pesquisa devem estar preenchidos)
 * @param compara_elem funcao de comparacao
 * @return ponteiro para o elemento caso este exista; NULL caso contrario
 */
void* lista_pesquisar(LISTA_GENERICA_T* lista, void* elem, COMPARAR_FUNC compara_elem);

/**
 * Funcao que aplica uma funcao a todos os elementos da lista.
 * @param lista ponteiro para a lista generica
 * @param aplica_elem funcao a chamar para cada elemento da lista
 */
void lista_aplicar_todos(LISTA_GENERICA_T* lista, APLICAR_FUNC aplica_elem);

/**
 * Funcao que devolve um iterador para a lista.
 * @param lista ponteiro para a lista generica
 * @return iterador para a lista
 */
ITERADOR_T* lista_criar_iterador(LISTA_GENERICA_T* lista);

/**
 * Funcao que devolve um iterador para uma versao ordenada da lista.
 * @param lista ponteiro para a lista generica
 * @param compara_elem funcao de comparacao para ordenar a lista
 * @return iterador para uma versao ordenada da lista
 */
ITERADOR_T* lista_criar_iterador_ordenado(LISTA_GENERICA_T* lista, COMPARAR_FUNC compara_elem);


/**
 * Funcao que devolve o proximo elemento do iterador.
 * @param iterador ponteiro para o iterador
 * @return ponteiro para o proximo elemento;NULL caso tenha chegado ao fim do iterador
 */
void* iterador_proximo_elemento(ITERADOR_T* iterador);

/**
 * Funcao que destroi o iterador.
 * @param iterador ponteiro para o iterador passado por referencia
 */
void iterador_destruir(ITERADOR_T** iterador);

#endif
