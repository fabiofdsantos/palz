/**
* @file common.c
* @brief Functions used by compression and decompression.
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#include "common.h"
#include "compress.h"

/* Global vars */
int got_signal = 0;

/**
* Free resources used on decompress functions.
* @param dictionary
*/
void decompress_resources_free(TDictionary **dictionary){
  TDictionary *aux = NULL;
  aux = *dictionary;

  dictionary_free(&aux);
}

/**
* Initialize decompress resources.
* @param dictionary
*/
void decompress_resources_init(TDictionary **dictionary){
  dictionary_init(dictionary);
}

/**
* Initialize dictionary with 14 elements (separators).
* @param dictionary
*/
void dictionary_init(TDictionary **dictionary){
  TDictionary *aux = MALLOC(sizeof(TDictionary));

  aux->element = MALLOC(sizeof(TElement) * 14);

  aux->element[0].nElement = 1;
  aux->element[0].element = "\n";

  aux->element[1].nElement = 2;
  aux->element[1].element = "\t";

  aux->element[2].nElement = 3;
  aux->element[2].element = "\r";

  aux->element[3].nElement = 4;
  aux->element[3].element = " ";

  aux->element[4].nElement = 5;
  aux->element[4].element = "?";

  aux->element[5].nElement = 6;
  aux->element[5].element = "!";

  aux->element[6].nElement = 7;
  aux->element[6].element = ".";

  aux->element[7].nElement = 8;
  aux->element[7].element = ";";

  aux->element[8].nElement = 9;
  aux->element[8].element = ",";

  aux->element[9].nElement = 10;
  aux->element[9].element = ":";

  aux->element[10].nElement = 11;
  aux->element[10].element = "+";

  aux->element[11].nElement = 12;
  aux->element[11].element = "-";

  aux->element[12].nElement = 13;
  aux->element[12].element = "*";

  aux->element[13].nElement = 14;
  aux->element[13].element = "/";

  aux->nElements = 14;
  *dictionary = aux;
}

/**
* Add a new element to dictionary.
* @param dictionary
* @param element
* @param size
*/
void dictionary_add_element(TDictionary **dictionary, char **element, int size){
  TDictionary *aux = NULL;
  char *auxElement = NULL;

  aux = *dictionary;
  auxElement = *element;
  if(aux->nElements == 0)
    aux->element = MALLOC(sizeof(TElement));
  else
	aux->element = realloc(aux->element, sizeof(TElement) * (aux->nElements + 1));
  aux->nElements += 1;
  aux->element[aux->nElements-1].nElement = aux->nElements;
  aux->element[aux->nElements-1].element = MALLOC(sizeof(char)*size);
  strcpy(aux->element[aux->nElements-1].element, auxElement);
}

/**
* Free dictionary.
* @param dictionary
* @return 0
*/
int dictionary_free(TDictionary **dictionary){
  TDictionary *aux = NULL;
  aux = *dictionary;

  if (aux->nElements > 14){
    dictionary_restart(&aux);
  }

  FREE(aux->element);
  FREE(aux);

  return 0;
}

/**
* Remove extra dictionary elements (all non-separators).
* @param dictionary
* @return 0
*/
int dictionary_restart(TDictionary **dictionary){
  TDictionary *aux = NULL;
  aux = *dictionary;

  int i = 0;

  for (i=aux->nElements; i>14; i--) {
    FREE(aux->element[i-1].element);
  }

  aux->element = realloc(aux->element, sizeof(TElement)*14);
  aux->nElements = 14;

  return 0;
}

/**
* Check how many bytes are necessary to represent a given number.
* @param max_value integer number
* @return number of bytes or -1 if max_value must be represented with 4 bytes
*/
int bytes_for_int(unsigned int max_value){
  /* 1 byte - 255 values */
  if (max_value < 256) {
    return 1;
  }

  /* 2 bytes - 65535 values */
  if (max_value < 65536) {
    return 2;
  }

  /* 3 bytes - 16777215 values */
  if (max_value < 16777216) {
    return 3;
  }

  /* Dictionary is too big */
  return -1;
}

/**
* Get error messages.
* @param id error code
* @param filename
*/
void get_error_msg(int id, char *filename){
  switch(id) {
    case ERR_PALZEXTENSION:
    fprintf(stderr, "Failed: %s is not a valid .palz file\n", filename);
    break;

    case ERR_PALZCORRUPTED:
    fprintf(stderr, "Failed: %s is corrupted\n", filename);
    break;

    case ERR_PALZBIGDICTIONARY:
    fprintf(stderr, "Failed: %s dictionary is too big\n", filename);
    break;

    case ERR_FOPEN:
    fprintf(stderr, "opendir() failed\n");
    break;

    case ERR_FSTATUS:
    fprintf(stderr, "stat() failed\n");
    break;
  }
}

/**
* Check if filename have .palz.
* @param filename
* @return 1 if true, 0 if false
*/
int is_dot_palz(const char *filename){
  char *dot = strrchr(filename, '.');

  /* filename without dot */
  if(!dot){
    return 0;
  }

  /* dot+1 = extension */
  if(strcasecmp((dot+1),"palz") == 0) {
    return 1;
  }
  return 0;
}

/**
* Search for palz or text files in a given folder and sub-folders. For every
* palz or text file found, save its path in an array.
* @param source main directory where to start
* @param paths array of paths
* @param amount amount of files
* @param mode compress or decompress mode
* @return 0 at end
*/
int get_files_from_dir(char *source, char ***paths, int *amount, int mode){
  DIR *dir = NULL;
  struct dirent *dirent = NULL;
  char *nextDirent = NULL;
  char **auxPaths = *paths;
  float output = 0;

  /* Open source directory */
  if ((dir = opendir(source)) == NULL) {
    return ERR_FOPEN;
  }

  /* While readdir returns a pointer to a dirent structure */
  while ((dirent = readdir(dir)) != NULL) {

    nextDirent = MALLOC(sizeof(char)*(strlen(dirent->d_name)
                                                     + strlen (source) + 2));
    strcpy(nextDirent, source);
    strcat(nextDirent, dirent->d_name);

    /* DT_DIR = directory */
    if (dirent->d_type == DT_DIR && strcmp(dirent->d_name, ".") != 0
                                         && strcmp(dirent->d_name, "..") != 0) {

      strcat(nextDirent, "/");

      if ((output =
            get_files_from_dir(nextDirent, &auxPaths, amount, mode)) < 0) {

        get_error_msg(output, NULL);
      }

      FREE(nextDirent);

      /* DT_REG = regular file */
    } else if (dirent->d_type == DT_REG) {

      if (is_dot_palz(dirent->d_name) == mode) {

        auxPaths = realloc(auxPaths, sizeof(char*)*((*amount)+1));
        auxPaths[*amount] = nextDirent;
        *amount += 1;

      } else {
        FREE(nextDirent);
      }
    } else {
      FREE(nextDirent);
    }
  }

  closedir(dir);
  *paths = auxPaths;

  return 0;
}

/**
* Compression ratio calculator.
* @param source_size source filesize
* @param final_size final filesize
* @return ratio
*/
float compress_ratio(float source_size, float final_size){
  float ratio = 0;
  if ((ratio = (1-(source_size / final_size))*100) > 0.00) {
    return ratio;
  } else /* prevent negative ratio */ {
    return 0;
  }
}

/**
* Get file size for a given filename.
* @param filename
* @return filesize or -1 in case of error.
*/
float get_size(char *filename){
  struct stat st;
  if (stat(filename, &st) == 0) {
    return (float)st.st_size;
  }
  return -1; /* stat() failed */
}

/**
* Consumer function used on a producer consumer problem using Ptheads.
* Based on exercise 3 from "Ficha 4" and some examples found on the Internet.
* @param args
*/
void *consumer(void *args) {
  PARAM_T *p = args;

  char *path = NULL;
  float output = 0;

  while (!got_signal){

    /* Enters critical section */
    if ((errno = pthread_mutex_lock(&(p->mutex))) != 0) {
      WARNING("pthread_mutex_lock() failed\n");
      got_signal = 1;
    }

    /* Waits for buffer data */
    while (p->total == 0 && p->stop == 0 && !got_signal) {
      if ((errno = pthread_cond_wait(&(p->cond), &(p->mutex))) != 0) {
        WARNING("pthread_cond_wait() failed");
        got_signal = 1;
      }
    }

    if(p->total == 0){

      /* Exits critical section */
      if ((errno = pthread_mutex_unlock(&(p->mutex))) != 0) {
        WARNING("pthread_mutex_unlock() failed");
        got_signal = 1;
      }

      break;
    }

    /* Read path from the buffer */
    path = realloc(path, strlen(p->buffer[p->index_reading])*sizeof(char)+1);
    strcpy(path, p->buffer[p->index_reading]);
    p->index_reading = (p->index_reading + 1) % p->max;
    p->total--;

    /* Notifies waiting producer */
    if (p->total == p->max-1) {
      if ((errno = pthread_cond_signal(&(p->cond))) != 0) {
        WARNING("pthread_cond_signal() failed");
        got_signal = 1;
      }
    }

    /* Exits critical section */
    if ((errno = pthread_mutex_unlock(&(p->mutex))) != 0) {
      WARNING("pthread_mutex_unlock() failed");
      got_signal = 1;
    }

    /* Ok, now it's time to compress the given file */
    if ((p->mode) == COMPRESS_MODE) {
      output = compress_file(path);
    } else {
      output = decompress_file(p->dictionary, path);
    }

    /* Print compression ratio */
    fprintf(stderr,"%.2f %%\n", output);

    //For test purposes
    //sleep(4);
  }

  FREE(path);
  return 0;
}

/**
* Function to write full path to a certain file in buffer.
* @param path full path to a certain file
* @param args
*/
void write_on_buffer(char *path, void *args){

  PARAM_T *p = args;

  /* Enters critical section */
  if ((errno = pthread_mutex_lock(&(p->mutex))) != 0) {
    WARNING("pthread_mutex_lock() failed\n");
    got_signal = 1;
  }

  /* Waits for space to write new data on buffer */
  while (p->total == p->max && !got_signal) {
    if ((errno = pthread_cond_wait(&(p->cond), &(p->mutex))) != 0) {
      WARNING("pthread_cond_wait() failed");
      got_signal = 1;
    }
  }

  /* Write data on buffer */
  p->buffer[p->index_writing] = realloc(p->buffer[p->index_writing],
                                                   strlen(path)*sizeof(char)+1);
  strcpy(p->buffer[p->index_writing], path);
  p->index_writing = (p->index_writing + 1) % p->max;
  p->total++;

  /* Notifies waiting threads */
  if (p->total == 1) {
    if ((errno = pthread_cond_broadcast(&(p->cond))) != 0) {
      WARNING("pthread_cond_broadcast() failed");
      got_signal = 1;
    }
  }

  /* Exits critical section */
  if ((errno = pthread_mutex_unlock(&(p->mutex))) != 0) {
    WARNING("pthread_mutex_unlock() failed");
    got_signal = 1;
  }
}
