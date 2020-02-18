/**
 * @file hashtables.h
 * @brief Tabelas de hashing
 *
 * Conjunto de funcoes para acesso a tabelas de hashing genericas. Consultar exemplo 3.
 * Por uma questao de simplicidade, apenas sao permitidas chaves do tipo string
 *
 * @Vitor Carreira
 * @date Abril 2004
 * @version 1
 */
#ifndef _HASHTABLES_H
#define _HASHTABLES_H

#include "listas.h"

typedef struct entrada {
	char* chave;
	void* elemento;
	int activo;
} ENTRADA_T;

typedef struct hashtable {
	ENTRADA_T** entradas;
	int total_activos, total_inactivos, tamanho;
	LIBERTAR_FUNC liberta_elemento;
} HASHTABLE_T;


/**
 * Funcao que cria uma hashtable
 * @param tamanho tamanho da hashtable
 * @param liberta_elem funcao para libertar a memoria de um elemento
 * @return ponteiro para a hashtable criada
 */
HASHTABLE_T* tabela_criar(int tamanho, LIBERTAR_FUNC liberta_elem);

/**
 * Funcao que insere um elemento na tabela
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave utilizada para indexar o elemento
 * @param elem elemento a colocar na tabela (este elemento deve
 * ser alocado exteriormente)
 */
void tabela_inserir(HASHTABLE_T* tabela, char* chave, void* elem);

/**
 * Funcao que remove um elemento da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave do elemento a remover
 * @return o ponteiro para o elemento que foi removido (depois deste
 * ponteiro nao ser necessario, nao esquecer de libertar a memoria).
 * Devolve NULL caso nao exista nenhum elemento associado 'a chave
 */
void* tabela_remover(HASHTABLE_T* tabela, char* chave);

/**
 * Funcao que remove todos os elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 */
void tabela_remover_todos(HASHTABLE_T* tabela);



/**
 * Funcao que devolve o numero de elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return o numero de elementos na tabela
 */
int tabela_numero_elementos(HASHTABLE_T* tabela);


/**
 * Funcao que destroi a tabela.
 * @param tabela ponteiro para a tabela de hash (passado por refer�ncia)
 */
void tabela_destruir(HASHTABLE_T** tabela);

/**
 * Funcao que devolve o elemento associado 'a chave indicada.
 * @param tabela ponteiro para a tabela de hash
 * @param chave chave do elemento a consultar
 * @return o ponteiro para o elemento caso exista; NULL caso contrario
 */
void* tabela_consultar(HASHTABLE_T* tabela, char* chave);


/**
 * Funcao que devolve uma lista com as chaves da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return lista de chaves
 */
LISTA_GENERICA_T* tabela_criar_lista_chaves(HASHTABLE_T* tabela);

/**
 * Funcao que devolve uma lista com os elementos da tabela.
 * @param tabela ponteiro para a tabela de hash
 * @return lista de elementos
 */
LISTA_GENERICA_T* tabela_criar_lista_elementos(HASHTABLE_T* tabela);

#endif
