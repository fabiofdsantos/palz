/**
* @file compress.h
* @brief The header file for compress.c
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "common.h"
#include "decompress.h"
#include "listas.h"
#include "hashtables.h"

/* Compress file */
int compress_file(char *source_filename);
int write_binary(HASHTABLE_T** tabela, FILE **fpSource, FILE **fpFinal, int bytes);

int cmpstringp(const void *p1, const void *p2);
void add_separators(HASHTABLE_T **table);

int parallel_folder_compress(char *directory, int max_threads);
#endif
