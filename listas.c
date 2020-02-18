/**
 * @file listas.c
 * @brief Listas genericas
 *
 * Conjunto de funcoes para acesso a listas genericas
 * @Vitor Carreira
 * @date Abril 2004
 * @version 1
 */

#include "listas.h"
#include "memory.h"
#include <stdlib.h>


/**
 * Funcao interna que insere os elementos numa lista de forma ordenada
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir
 * @param compara_elem funcao de comparacao
 */
static void lista_inserir_ordenado(LISTA_GENERICA_T* lista, void* elem, COMPARAR_FUNC compara_elem);


/**
 * Funcao que cria uma lista generica
 * @param liberta_elem ponteiro para uma funcao que liberta a memoria de um elemento da lista
 * @return ponteiro para a lista criada
 */
LISTA_GENERICA_T* lista_criar(LIBERTAR_FUNC liberta_elem) {
	LISTA_GENERICA_T* lista = (LISTA_GENERICA_T*)MALLOC(sizeof(LISTA_GENERICA_T));

	lista->base = (NO_T*)MALLOC(sizeof(NO_T));
	lista->base->prox = lista->base->ant = lista->base;
	lista->base->elem = NULL;
	lista->liberta_memoria = liberta_elem;

	lista->numero_elementos = 0;

	return lista;
}

/**
 * Funcao que insere um elemento na lista. O elemento e' inserido no final da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir(LISTA_GENERICA_T* lista, void* elem) {
	lista_inserir_fim(lista, elem);
}

/**
 * Funcao que insere um elemento no inicio da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir_inicio(LISTA_GENERICA_T* lista, void* elem) {
	NO_T* aux = (NO_T*)malloc(sizeof(NO_T));

	aux->prox = lista->base->prox;
	aux->prox->ant = aux;
	aux->ant = lista->base;
	aux->elem = elem;
	lista->base->prox = aux;

	lista->numero_elementos++;
}

/**
 * Funcao que insere um elemento no final da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir (este elemento deve
 * ser alocado exteriormente)
 *
 */
void lista_inserir_fim(LISTA_GENERICA_T* lista, void* elem) {
	NO_T* aux = (NO_T*)MALLOC(sizeof(NO_T));
	NO_T* previo = lista->base->ant;

	aux->elem = elem;
	aux->prox = lista->base;
	aux->ant = previo;
	previo->prox = aux;
	lista->base->ant = aux;


	lista->numero_elementos++;

}


/**
 * Funcao que remove um elemento da lista.
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a remover
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso o elemento nao existe
 */
void* lista_remover(LISTA_GENERICA_T* lista, void* elem) {
	NO_T* aux = lista->base->prox;
	void* elemento;

	while (aux != lista->base) {
		if (aux->elem == elem) {
			elemento = aux->elem;
			aux->ant->prox = aux->prox;
			aux->prox->ant = aux->ant;
			free(aux);
			lista->numero_elementos--;
			return elemento;
		}
		aux = aux->prox;
	}
	return NULL;
}

/**
 * Funcao que remove o elemento que se encontra no inicio da lista.
 * @param lista ponteiro para a lista generica
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso a lista se encontre vazia
 */
void* lista_remover_inicio(LISTA_GENERICA_T* lista) {
	NO_T* aux = lista->base->prox;
	void* elemento;

	if (aux == lista->base)
		return NULL;

	elemento = aux->elem;
	aux->ant->prox = aux->prox;
	aux->prox->ant = aux->ant;
	free(aux);
	lista->numero_elementos--;
	return elemento;
}

/**
 * Funcao que remove o elemento que se encontra no fim da lista.
 * @param lista ponteiro para a lista generica
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso a lista se encontre vazia
 */
void* lista_remover_fim(LISTA_GENERICA_T* lista) {
	NO_T* aux = lista->base->ant;
	void* elemento;

	if (aux == lista->base)
		return NULL;

	elemento = aux->elem;
	aux->ant->prox = aux->prox;
	aux->prox->ant = aux->ant;
	free(aux);
	lista->numero_elementos--;
	return elemento;
}

/**
 * Funcao que remove todos os elementos da lista.
 * @param lista ponteiro para a lista generica
 */
void lista_remover_todos(LISTA_GENERICA_T* lista) {
	NO_T* proximo = lista->base->prox;
	NO_T* aux;

	while (proximo != lista->base) {
		aux = proximo;
		proximo = proximo->prox;
		if (lista->liberta_memoria != NULL)
			lista->liberta_memoria(aux->elem);
		free(aux);
	}
	lista->base->prox = lista->base->ant = lista->base;
	lista->numero_elementos = 0;
}



/**
 * Funcao que devolve o numero de elementos da lista.
 * @param lista ponteiro para a lista generica
 * @return o numero de elementos na lista
 */
int lista_numero_elementos(LISTA_GENERICA_T* lista) {
	return lista->numero_elementos;
}


/**
 * Funcao que destroi a lista.
 * @param lista ponteiro para a lista generica (passado por refer�ncia)
 */
void lista_destruir(LISTA_GENERICA_T** lista) {
	lista_remover_todos(*lista);
	free((*lista)->base);
	free(*lista);
	*lista = NULL;
}

/**
 * Funcao que pesquisa a lista 'a procura de um elemento.
 * @param lista ponteiro para a lista generica
 * @param elem elemento a procurar (apenas os campos utilizados pela funcao de
 * pesquisa devem estar preenchidos)
 * @param compara_elem funcao de comparacao
 * @return ponteiro para o elemento caso este exista; NULL caso contrario
 */
void* lista_pesquisar(LISTA_GENERICA_T* lista, void* elem, COMPARAR_FUNC compara_elem) {
	NO_T* aux = lista->base->prox;

	while (aux != lista->base) {
		if (compara_elem(elem, aux->elem) == 0)
			return aux->elem;
		aux = aux->prox;
	}
	return NULL;
}

/**
 * Funcao que aplica uma funcao a todos os elementos da lista.
 * @param lista ponteiro para a lista generica
 * @param aplica_elem funcao a chamar para cada elemento da lista
 */
void lista_aplicar_todos(LISTA_GENERICA_T* lista, APLICAR_FUNC aplica_elem) {
	NO_T* aux = lista->base->prox;

	while (aux != lista->base) {
		aplica_elem(aux->elem);
		aux = aux->prox;
	}
}

/**
 * Funcao que devolve um iterador para a lista.
 * @param lista ponteiro para a lista generica
 * @return iterador para a lista
 */
ITERADOR_T* lista_criar_iterador(LISTA_GENERICA_T* lista) {
	ITERADOR_T* iterador = (ITERADOR_T*)malloc(sizeof(ITERADOR_T));

	/* Cria uma copia da lista */
	LISTA_GENERICA_T * nova_lista = lista_criar(NULL);
	NO_T* aux = lista->base->prox;

	while (aux != lista->base) {
		lista_inserir_fim(nova_lista, aux->elem);
		aux = aux->prox;
	}

	iterador->base = iterador->actual = nova_lista->base;

	free(nova_lista);

	return iterador;
}

/**
 * Funcao que devolve um iterador para uma versao ordenada da lista.
 * @param lista ponteiro para a lista generica
 * @param compara_elem funcao de comparacao para ordenar a lista
 * @return iterador para uma versao ordenada da lista
 */
ITERADOR_T* lista_criar_iterador_ordenado(LISTA_GENERICA_T* lista, COMPARAR_FUNC compara_elem) {
	ITERADOR_T* iterador = (ITERADOR_T*)MALLOC(sizeof(ITERADOR_T));

	/* Cria uma versao ordenada da lista */
	LISTA_GENERICA_T * lista_ordenada = lista_criar(NULL);
	NO_T* aux = lista->base->prox;

	while (aux != lista->base) {
		lista_inserir_ordenado(lista_ordenada, aux->elem, compara_elem);
		aux = aux->prox;
	}

	iterador->base = iterador->actual = lista_ordenada->base;

	free(lista_ordenada);

	return iterador;
}


/**
 * Funcao que devolve o proximo elemento do iterador.
 * @param iterador ponteiro para o iterador
 * @return ponteiro para o proximo elemento;NULL caso tenha chegado ao fim do iterador
 */
void* iterador_proximo_elemento(ITERADOR_T* iterador) {
	iterador->actual = iterador->actual->prox;
	if (iterador->actual == iterador->base)
		return NULL;
	return iterador->actual->elem;
}

/**
 * Funcao que destroi o iterador.
 * @param iterador ponteiro para o iterador passado por referencia
 */
void iterador_destruir(ITERADOR_T** iterador) {
	ITERADOR_T *it = *iterador;
	NO_T* aux = it->base->prox;
	NO_T* liberta;

	while (aux != it->base) {
		liberta = aux;
		aux = aux->prox;
		free(liberta);
	}
	free(it->base);
	free(*iterador);
	*iterador = NULL;
}



/**
 * Funcao interna que insere os elementos numa lista de forma ordenada
 * @param lista ponteiro para a lista generica
 * @param elem ponteiro para o elemento a inserir
 * @param compara_elem funcao de comparacao
 */
void lista_inserir_ordenado(LISTA_GENERICA_T* lista, void* elem, COMPARAR_FUNC compara_elem) {
	NO_T* previo = lista->base;
	NO_T* aux = (NO_T*)MALLOC(sizeof(NO_T));

	while (previo->prox != lista->base && compara_elem(elem, previo->prox->elem) > 0) {
		previo = previo->prox;
	}

	aux->prox = previo->prox;
	aux->ant = previo;
	aux->elem = elem;
	previo->prox = aux;
}
