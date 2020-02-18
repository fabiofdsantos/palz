/**
 * @file hashtables.h
 * @brief Tabelas de hashing
 *
 * Conjunto de funcoes para acesso a tabelas de hashing genericas.
 * Esta implementacao e' uma adaptacao para C das aulas de P4.
 *
 * @Vitor Carreira
 * @date Abril 2004
 * @version 1
 */

#include "hashtables.h"
#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Funcao que devolve o primo mais proximo do valor dado*/
static int proximo_primo(int);

/* Funcao que devolve a posicao de insercao na tabela dada uma chave */
static int posicao_chave(HASHTABLE_T*, char*);

/* Funcao que efectua o re-hash da tabela */
static void rehash(HASHTABLE_T* );

/* Funcao que calcula o factor de carga */
static float factor_carga(HASHTABLE_T* );

/* Funcao de hashing para strings */
static unsigned int hashing_string(char* );

/* Funcao que cria e inicia o vector de entradas */
static ENTRADA_T** criar_vector_entradas(int);


/**
 * Funcao que cria uma hashtable
 * @param tamanho tamanho da hashtable
 * @param liberta_elem funcao para libertar a memoria de um elemento
 * @return ponteiro para a hashtable criada
 */
HASHTABLE_T* tabela_criar(int tamanho, LIBERTAR_FUNC liberta_elem) {
	HASHTABLE_T* tabela = (HASHTABLE_T*)malloc(sizeof(HASHTABLE_T));

	tabela->tamanho = proximo_primo(tamanho);
	tabela->entradas = criar_vector_entradas(tabela->tamanho);

	tabela->total_activos = tabela->total_inactivos = 0;
	tabela->liberta_elemento = liberta_elem;

	return tabela;
}

/**
 * Funcao que insere um elemento na tabela
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave utilizada para indexar o elemento
 * @param elem elemento a colocar na tabela (este elemento deve
 * ser alocado exteriormente)
 */
void tabela_inserir(HASHTABLE_T* tabela, char* chave, void* elem) {
	int i = posicao_chave(tabela, chave);
	ENTRADA_T* entrada = tabela->entradas[i];

	if (entrada != NULL && strcmp(entrada->chave, chave) == 0) {
		if (entrada->activo) {
			/* se o elemento ja' existe substitui o seu valor */
			if (tabela->liberta_elemento != NULL)
				tabela->liberta_elemento(entrada->elemento);
			entrada->elemento = elem;
			return;
		}

		entrada->elemento = elem;
		entrada->activo = 1;
		tabela->total_inactivos--;
	} else {
		entrada = (ENTRADA_T*)malloc(sizeof(ENTRADA_T));
		entrada->chave = (char*)malloc(strlen(chave)+1);
		strcpy(entrada->chave,chave);
		entrada->elemento = elem;
		entrada->activo = 1;
		tabela->entradas[i] = entrada;
	}
	tabela->total_activos++;
	if (factor_carga(tabela) >= 0.5)
		rehash(tabela);
}

/**
 * Funcao que remove um elemento da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave do elemento a remover
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso nao exista nenhum elemento associado 'a chave
 */
void* tabela_remover(HASHTABLE_T* tabela, char* chave) {
	int i = posicao_chave(tabela, chave);
	ENTRADA_T* entrada = tabela->entradas[i];

	if (entrada != NULL &&
		strcmp(entrada->chave, chave) == 0 &&
	    entrada->activo)
	{
		entrada->activo = 0;
		tabela->total_inactivos++;
		tabela->total_activos--;

		return entrada->elemento;
	}
	return NULL;
}

/**
 * Funcao que remove todos os elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 */
void tabela_remover_todos(HASHTABLE_T* tabela) {
	int i;
	ENTRADA_T* aux;

	for (i = 0; i < tabela->tamanho; i++) {
		aux = tabela->entradas[i];
		if (aux != NULL) {
			/* liberta a mem�ria alocada para a chave */
			free(aux->chave);
			/* liberta a mem�ria alocada para o elemento */
			if (aux->activo && tabela->liberta_elemento != NULL)
			tabela->liberta_elemento(aux->elemento);
			/* liberta a mem�ria alocada para a entrada */
			free(aux);
			tabela->entradas[i] = NULL;
		}
	}
	tabela->total_activos = tabela->total_inactivos = 0;
}



/**
 * Funcao que devolve o numero de elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return o numero de elementos na tabela
 */
int tabela_numero_elementos(HASHTABLE_T* tabela) {
	return tabela->total_activos;
}


/**
 * Funcao que destroi a tabela.
 * @param tabela ponteiro para a tabela de hash (passado por refer�ncia)
 */
void tabela_destruir(HASHTABLE_T** tabela) {
	tabela_remover_todos(*tabela);
	free((*tabela)->entradas);
	free(*tabela);
	//*tabela = NULL;
}

/**
 * Funcao que devolve o elemento associado 'a chave indicada.
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave do elemento a consultar
 * @return o ponteiro para o elemento caso exista; NULL caso contrario
 */
void* tabela_consultar(HASHTABLE_T* tabela, char* chave) {
	int i = posicao_chave(tabela, chave);
	ENTRADA_T* entrada = tabela->entradas[i];

	if (entrada != NULL && strcmp(entrada->chave, chave) == 0 && entrada->activo){
			if(entrada->elemento == NULL){
				return (int *)-1; /* if elemento not defined */
			} else {
				return entrada->elemento;
			}
	}
	return NULL;
}


/**
 * Funcao que devolve uma lista com as chaves da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return lista de chaves
 */
LISTA_GENERICA_T* tabela_criar_lista_chaves(HASHTABLE_T* tabela) {
	LISTA_GENERICA_T* lista = lista_criar(NULL);
	int i;
	ENTRADA_T* entrada;

	for (i=0;i<tabela->tamanho;i++) {
		entrada = tabela->entradas[i];
		if (entrada != NULL && entrada->activo)
			lista_inserir(lista, entrada->chave);
	}
	return lista;
}

/**
 * Funcao que devolve uma lista com os elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return lista de elementos
 */
LISTA_GENERICA_T* tabela_criar_lista_elementos(HASHTABLE_T* tabela){
	LISTA_GENERICA_T* lista = lista_criar(NULL);
	int i;
	ENTRADA_T* entrada;

	for (i=0;i<tabela->tamanho;i++) {
		entrada = tabela->entradas[i];
		if (entrada != NULL && entrada->activo)
			lista_inserir(lista, entrada->elemento);
	}
	return lista;
}


/* ---------------------------------------------------------- */
/* Fun��es locais                                             */
/* ---------------------------------------------------------- */

/* Funcao que devolve o primo mais proximo do valor dado*/
int proximo_primo(int n) {
	int i;
	if (n < 2)
		return 2;
	if (n % 2 == 0)
		++n;

	for (;;n += 2) {
		for (i = 3; i * i <= n && n % i != 0; i += 2)
			;
		if (i * i > n)
			return n;
	}
}


/* Funcao que devolve a posicao de insercao na tabela dada uma chave.
   Tratamento de colisoes: hashing quadratico
   (ver apontamentos de P4 para mais detalhes)
*/
int posicao_chave(HASHTABLE_T *t, char* chave) {
	int i = hashing_string(chave) % t->tamanho, pos = -1, inicial = i, inc = 1;

	while (t->entradas[i] != NULL && strcmp(t->entradas[i]->chave, chave) != 0) {
		if (!t->entradas[i]->activo) {
			pos = i;
			break;
		}
		i = (i + inc) % t->tamanho;
		inc += 2;
		if (i == inicial) {
			fprintf(stderr, "Sondagem circular");
			exit(1);
		}
	}
	if (pos != -1)
		do {
			i = (i + inc) % t->tamanho;
			inc += 2;
			if (i == inicial) {
				fprintf(stderr, "Sondagem circular");
				exit(1);
			}
		} while (t->entradas[i] != NULL && strcmp(t->entradas[i]->chave, chave) != 0);

	if (t->entradas[i] == NULL  &&  pos != -1)
		return pos;

	return i;
}

/* Funcao que efectua o re-hash da tabela */
static void rehash(HASHTABLE_T* tabela) {
		ENTRADA_T* entrada;
		int tamanho_antigo = tabela->tamanho;
		int tamanho_novo = proximo_primo(tabela->tamanho * 2);
		int i;
		ENTRADA_T** entradas_antigas = tabela->entradas;

		tabela->tamanho = tamanho_novo;
		tabela->entradas = criar_vector_entradas(tabela->tamanho);
		tabela->total_activos = tabela->total_inactivos = 0;

		for (i = 0; i < tamanho_antigo; i++) {
			entrada = entradas_antigas[i];
			if (entrada != NULL && entrada->activo) {
				tabela_inserir(tabela, entrada->chave, entrada->elemento);
				free(entrada->chave);
				free(entrada);
			}
		}
		free(entradas_antigas);
}

/* Funcao que calcula o factor de carga */
float factor_carga(HASHTABLE_T * tabela) {
	return (tabela->total_activos + tabela->total_inactivos) / (float) tabela->tamanho;
}

/**
 * Funcao de hashing para strings
 */
unsigned int hashing_string(char* str) {
    int len = strlen(str), i;
    unsigned int hash = 0;
	for (i = 0; i < len; i++) {
    	hash = 31 * hash + (unsigned char)str[i];
    }
	return hash;
}

/* Funcao que cria e inicia o vector de entradas */
static ENTRADA_T** criar_vector_entradas(int tamanho) {
	ENTRADA_T ** entradas = (ENTRADA_T**)MALLOC(sizeof(ENTRADA_T*)*tamanho);
	int i;
	for (i=0;i<tamanho;i++)
		entradas[i] = NULL;
	return entradas;
}
