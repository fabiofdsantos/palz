/**
* @file decompress.c
* @brief Decompress functions.
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#include "decompress.h"
#include "common.h"

/* External variables */
extern int got_signal;

/**
* Decompress a given palz file.
* @param source_filename
* @param dictionary
* @return compress ratio
* @see compress_ratio()
*/
float decompress_file(TDictionary **dictionary, char *source_filename){
  TDictionary *aux = NULL;
  aux = *dictionary;
  size_t len = 0;
  char *final_filename = NULL;
  char *line = NULL;
  char *element = NULL;
  int read;
  int val = 0;
  int nbytes = 0;
  int bytesForInt = 0;
  unsigned int last_element = 0;
  float source_file_size = 0;
  float final_file_size = 0;
  FILE *fpTempFile = NULL;
  FILE *fpSourceFile = NULL;
  FILE *fpFinalFile = NULL;

  TDictionary *dictWords = NULL;
  dictWords = MALLOC(sizeof(TDictionary));
  dictWords->nElements = 0;

  /* Open .palz file */
  if ((fpSourceFile = fopen(source_filename, "r")) == NULL) {
    return ERR_FOPEN;
  }

  /* Get first line */
  if (getline(&line, &len, fpSourceFile) == -1) {
    return ERR_PALZCORRUPTED;
  }

  if (!is_header_PALZ(line)) {
    return ERR_PALZEXTENSION;
  }

  /* Get second line */
  if (getline(&line, &len, fpSourceFile) == -1) {
    return ERR_PALZCORRUPTED;
  }

  if ((val = is_valid_size(line)) == -1) {
    return ERR_PALZCORRUPTED;
  }

  if (bytes_for_int(val+14) == -1) {
    return ERR_PALZBIGDICTIONARY;
  }

  while (val != 0) {
    nbytes = getline(&line, &len, fpSourceFile);
    if ((element = malloc(sizeof(char)*nbytes)) == NULL) {
      return ERR_PALZCORRUPTED;
    }
    strncpy(element, line, nbytes-1);
    element[nbytes-1] = '\0';
    dictionary_add_element(&dictWords, &element, nbytes);
    FREE(element);
    val--;
  }
  FREE(line);

  bytesForInt = bytes_for_int(dictWords->nElements + 14);
  unsigned char elementN[bytesForInt];

  fpTempFile = tmpfile();

  while (!feof(fpSourceFile)) {
    while (fread(&elementN, 1, bytesForInt, fpSourceFile)) {

      /**
      * Check if number read from binary code is greater than dictionary entries
      * or less than zero.
      */
      if (*(int *)elementN > (dictWords->nElements + 14) ||
                                                         *(int *)elementN < 0) {
        return ERR_PALZCORRUPTED;
      }

      /* Check for repetition */
      if (*(int *)elementN == 0) {

        /**
        * We can't start with a repetition. So in this case,
        * we verify if last_element exists to validate the repetition.
        */
        if (last_element == 0) {
          return ERR_PALZCORRUPTED;
        }

        /* Check how many times the last_element must be repeated */
        if (fread(&elementN, 1, bytesForInt, fpSourceFile)
                                                 != (unsigned int)bytesForInt) {
          return ERR_PALZCORRUPTED;
        }

        /* last_element can't be repeated zero or less times */
        if(*(int *)elementN <= 0) {
          return ERR_PALZCORRUPTED;
        }

        /* Repeat for elementN times */
        while(*(int *)elementN != 0) {
          if(last_element < 15){
            fputs(aux->element[last_element-1].element, fpTempFile);
          }else{
            fputs(dictWords->element[last_element-15].element, fpTempFile);
          }
          *(int *)elementN -= 1;
        }
      }else{
        if(*(int *)elementN < 15){
          fputs(aux->element[*(int *)elementN-1].element, fpTempFile);
        }else{
          fputs(dictWords->element[*(int *)elementN - 15].element, fpTempFile);
        }
        last_element = *(int *)elementN;
      }
    }
  }

  fclose(fpSourceFile);

  int i;
  for(i = 0; i< dictWords->nElements; i++){
    FREE(dictWords->element[i].element);
  }

  FREE(dictWords->element);
  FREE(dictWords);

  if ((source_file_size = get_size(source_filename))==-1) {
    return ERR_FSTATUS;
  }
  /*
  * Sets the file position indicator for the stream
  * pointed to by stream to the beginning of the file.
  */
  rewind(fpTempFile);

  if (is_dot_palz(source_filename)) {
    final_filename = remove_dot_palz(source_filename);
  } else {
    final_filename = source_filename;
  }

  fpFinalFile = fopen(final_filename, "w");

  /* Copy data from tmpfile to destination file */
  while((read = fgetc(fpTempFile)) != EOF) {
    fputc(read, fpFinalFile);
  }

  fclose(fpTempFile);
  fclose(fpFinalFile);

  if ((final_file_size = get_size(final_filename))==-1) {
    return ERR_FSTATUS;
  }

  fprintf(stderr,"Compression ratio: %s ", final_filename);

  return compress_ratio(source_file_size, final_file_size);
}

/**
* Check if header_first_row contains "PALZ\n".
* @param header_first_row first row of file
* @return 1 if true, 0 if false
*/
int is_header_PALZ(const char *header_first_row){
  if (strcmp(header_first_row,MAGIC_PALZ) == 0) {
    return 1;
  }
  return 0;
}

/**
* Check if a given string is a valid integer. Based on strtol page from manual.
* @param size_str second row of file
* @return val value represented on string or -1 if string is a non-valid integer
*/
int is_valid_size(const char *size_str){
  int base = 10;
  char *first_failure;
  long val;

  errno = 0;
  val = strtol(size_str, &first_failure, base);

  /* Check for various possible errors */
  if (val < 0 || (errno != 0 && val == 0)) {
    return -1;
  }

  if (first_failure != NULL && (*first_failure != '\0')
                                                   & (*first_failure != '\n')) {
    return -1;
  }

  return val;
}

/**
* Search for palz files in a given folder and sub-folders. For every palz file
* found, call decompress_file() function.
* @param directory main directory where to start
* @param dictionary
* @return 0 at end
* @see decompress_file()
*/
int decompress_folder(TDictionary **dictionary, const char *directory){
  TDictionary *aux = NULL;
  aux = *dictionary;
  DIR *dir = NULL;
  struct dirent *dirent = NULL;
  char *nextDirent = NULL;
  float output = 0;

  /* Open directory */
  if ((dir = opendir(directory)) == NULL) {
    return ERR_FOPEN;
  }

  /* While readdir returns a pointer to a dirent structure */
  while (!got_signal && (dirent = readdir(dir)) != NULL) {
    nextDirent = MALLOC(sizeof(char)*(strlen(dirent->d_name)
    + strlen (directory) + 2));

    strcpy(nextDirent, directory);

    /* Appends d_name */
    strcat(nextDirent, dirent->d_name);

    /* DT_DIR = directory */
    if (dirent->d_type == DT_DIR && strcmp(dirent->d_name, ".") != 0
                                         && strcmp(dirent->d_name, "..") != 0) {

      strcat(nextDirent, "/");

      decompress_folder(&aux, nextDirent);
      /* DT_REG = regular file */
    } else if (dirent->d_type == DT_REG) {
      if (is_dot_palz(dirent->d_name)) {
        dictionary_restart(&aux);
        if ((output = decompress_file(&aux, nextDirent)) < 0) {
          get_error_msg(output, nextDirent);
        } else {
          fprintf(stderr,"%.2f %%\n", output);
        }
      }
    }
    FREE(nextDirent);
  }
  closedir(dir);

  return 0;
}

/**
* Search for .palz files in a given folder and sub-folders. For every .palz file
* found, call decompress_file() function using threads. Each file must be
* decompressed by one thread only.
* @param directory main directory where to start
* @param max_threads maximum number of threads
* @return 0 at end
* @see decompress_file()
*/
int parallel_folder_decompress(TDictionary **dictionary, char *directory,
                                                               int max_threads){
  char **files_to_decompress = NULL;
  int amount = 0;
  float output = 0;

  pthread_t thr[max_threads];

  PARAM_T param;

  /* Initialize the mutex */
  if ((errno = pthread_mutex_init(&param.mutex, NULL)) != 0) {
    ERROR(C_ERRO_MUTEX_INIT, "pthread_mutex_init() failed!");
  }

  /* Initialize the condition variable */
  if ((errno = pthread_cond_init(&param.cond, NULL)) != 0) {
    ERROR(C_ERRO_CONDITION_INIT, "pthread_cond_init() failed!");
  }

  /* Initialize other parameters */
  param.buffer = MALLOC(max_threads*sizeof(char*));
  param.index_reading = 0;
  param.index_writing = 0;
  param.total = 0;
  param.stop = 0;
  param.max = max_threads;
  param.mode = DECOMPRESS_MODE;
  param.dictionary = dictionary;

  int i;

  for(i=0; i<param.max; i++){
    param.buffer[i] = NULL;
  }

  /* Create the threads */
  for(i=0; i<max_threads; i++){
    if ((errno = pthread_create(&thr[i], NULL, consumer, &param) != 0)) {
      ERROR(C_ERRO_PTHREAD_CREATE, "pthread_create() failed!");
    }
  }

  /* Search for all non-palz files */
  if ((output = get_files_from_dir(directory, &files_to_decompress, &amount,
                                                        DECOMPRESS_MODE)) < 0) {
    get_error_msg(output, NULL);
  }

  /* Give all files to compress to write on buffer */
  for(i = 0; i<amount; i++){
    write_on_buffer(files_to_decompress[i], &param);
    FREE(files_to_decompress[i]);
  }
  FREE(files_to_decompress);

  /* Enters critical section */
  if ((errno = pthread_mutex_lock(&(param.mutex))) != 0) {
    WARNING("pthread_mutex_lock() failed\n");
    return 0;
  }

  /* Stop production */
  param.stop = 1;

  /* Notify all threads */
  if ((errno = pthread_cond_broadcast(&(param.cond))) != 0) {
    WARNING("pthread_cond_broadcast() failed");
    return 0;
  }

  /* Exits critical section */
  if ((errno = pthread_mutex_unlock(&(param.mutex))) != 0) {
    WARNING("pthread_mutex_unlock() failed");
    return 0;
  }

  /* Wait for the threads to finish */
  for(i=0; i<max_threads; i++) {
    if ((errno = pthread_join(thr[i], NULL)) != 0) {
      ERROR(C_ERRO_PTHREAD_JOIN, "pthread_join() failed!");
    }
  }

  /* Free the mutex */
  if ((errno = pthread_mutex_destroy(&param.mutex)) != 0) {
    ERROR(C_ERRO_MUTEX_DESTROY, "pthread_mutex_destroy() failed!");
  }

  /* Free consumer condition variable */
  if ((errno = pthread_cond_destroy(&param.cond)) != 0) {
    ERROR(C_ERRO_CONDITION_DESTROY, "pthread_cond_destroy() failed!");
  }

  /* Free buffer */
  for (i=0; i<param.max; i++){
    FREE(param.buffer[i]);
  }
  FREE(param.buffer);

  return 0;
}

/**
* Remove .palz from a certain filename.
* @param filename
* @return new_filename without .palz
*/
char* remove_dot_palz(const char *source_filename){
  char *new_filename = (char*)source_filename;

  /* replace dot with \0 */
  new_filename[strlen(new_filename)-5] = '\0';

  return new_filename;
}
