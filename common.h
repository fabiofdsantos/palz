/**
* @file common.h
* @brief The header file for common.c
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "debug.h"
#include "memory.h"
#include "cmdline.h"

#define MAGIC_PALZ                      "PALZ\n"
#define ERR_PALZEXTENSION               -1
#define ERR_PALZCORRUPTED               -2
#define ERR_PALZBIGDICTIONARY           -3
#define ERR_FOPEN                       -4
#define ERR_FSTATUS                     -5

#define C_ERRO_PTHREAD_CREATE           1
#define C_ERRO_PTHREAD_JOIN             2
#define C_ERRO_MUTEX_INIT               3
#define C_ERRO_MUTEX_DESTROY            4
#define C_ERRO_CONDITION_INIT           5
#define C_ERRO_CONDITION_DESTROY        6

#define DECOMPRESS_MODE                 1
#define COMPRESS_MODE                   0

typedef struct resources{
  /* flags */
  int flag_sourceFile;
  int flag_finalFile;
  int flag_tempFile;
  /* files */
  FILE *sourceFile;
  FILE *finalFile;
  FILE *tempFile;
}TResources;

typedef struct dictionary_element{
  int nElement;
  char *element;
}TElement;

typedef struct dictionary_data{
  int nElements;
  TElement *element;
}TDictionary;

typedef struct{
  char **buffer;
  int index_reading;
  int index_writing;
  int total;
  int stop;
  int max;
  int mode;
  TDictionary **dictionary;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
}PARAM_T;

void decompress_resources_free(TDictionary **dictionary);
void decompress_resources_init(TDictionary **dictionary);
void resource_add_file(FILE *file, char *type);
void resource_remove_flag(char *type);

void dictionary_init(TDictionary **dictionary);
void dictionary_add_element(TDictionary **dictionary, char **element, int size);
int dictionary_free(TDictionary **dictionary);
int dictionary_restart(TDictionary **dictionary);

int bytes_for_int(unsigned int max_value);
void get_error_msg(int id, char *filename);
int is_dot_palz(const char *source_filename);
int get_files_from_dir(char *source, char ***paths, int *amount, int isDotPalz);

float compress_ratio(float source_size, float final_size);
float get_size(char *filename);

void *consumer(void *args);
void write_on_buffer(char *path, void *args);

#endif
