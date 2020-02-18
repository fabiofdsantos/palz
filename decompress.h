/**
* @file decompress.h
* @brief The header file for decompress.c
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#ifndef __DECOMPRESS_H__
#define __DECOMPRESS_H__

#include "common.h"

int is_header_PALZ(const char *header_first_row);
int is_valid_size(const char *size_str);
int decompress_folder(TDictionary **dictionary, const char *directory);
float decompress_file(TDictionary **dictionary, char *source_filename);
char* remove_dot_palz(const char *source_filename);

int parallel_folder_decompress(TDictionary **dictionary, char *directory, int max_threads);
#endif
