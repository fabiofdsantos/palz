/**
* @file compress.c
* @brief Compress functions.
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#include "compress.h"

/* Global vars */
const char *separators = "\n\t\r ?!.;,:+-*/";

/**
* Compress a given text file using an algorithm similar to the LZ77/LZ78.
* @param source_filename file to compress
* @return compress_ratio()
* @see write_binary()
*/
int compress_file(char *source_filename){
	size_t len = 0;
	HASHTABLE_T *table;
	FILE *fpSource = NULL;
	FILE *fpFinal = NULL;
	char *final_filename = MALLOC(sizeof(char)*(strlen(source_filename)+6));
	char *line = NULL;
	char *word = NULL;
	char *last_word = NULL;
	char **array = NULL;
	int *value;
	int bytes;
	int count = 0;
	float source_file_size = 0;
	float final_file_size = 0;

	/* Create hastable */
	table = tabela_criar(101, free);

	fpSource = fopen(source_filename, "r");

	/* Read and save distinct words */
	while(getline(&line, &len, fpSource) > 0) {
		last_word = NULL;
		word = strtok_r(line, separators, &last_word);

		while(word){

			/* Check for a dictionary out of bounds */
			if(count == 16777216){
				/* Free resources and return error */
				fclose(fpSource);
				FREE(final_filename);
				tabela_destruir(&table);
				return ERR_PALZBIGDICTIONARY;
			}

			/* Check if word already exist on table */
			if(tabela_consultar(table, word) == NULL){

				tabela_inserir(table, word, malloc(sizeof(int)));

				array = (char **) realloc(array, (count+1)*sizeof(char*));
				array[count] = malloc(strlen(word)+1);
				strcpy(array[count], word);

				count++;
			}
			word = strtok_r(NULL, separators, &last_word);
		}
	}
	FREE(line);

	/* Get the number of bytes */
	int tmp = count+14;
	bytes = bytes_for_int(tmp);

	/* Check for a dictionary out of bounds */
	if(bytes == -1){
		/* Free resources and return error */
		fclose(fpSource);
		FREE(final_filename);
		tabela_destruir(&table);
		return ERR_PALZBIGDICTIONARY;
	}

	/* Sort an array of distinct words */
	qsort(&array[0], count, sizeof(char *), cmpstringp);

	/* Add .palz extension */
	strcpy(final_filename, source_filename);
	strcat(final_filename, ".palz");

	fpFinal = fopen(final_filename, "wb");

	/* Write header (PALZ and dictionary size) */
	fprintf(fpFinal,MAGIC_PALZ);
	fprintf(fpFinal,"%d\n", tabela_numero_elementos(table));

	/* Write header (list of distinct words) and update word values */
	for(tmp=0; tmp<tabela_numero_elementos(table); tmp++){
		fprintf(fpFinal,"%s\n", array[tmp]);

		value = MALLOC(sizeof(int));
		*value = tmp + 15;
		tabela_inserir(table, array[tmp], value);

		free(array[tmp]);
	}
	free(array);

	/* Add separators to table */
	add_separators(&table);

	/* Write binary */
	write_binary(&table, &fpSource, &fpFinal, bytes);

	fclose(fpSource);
	fclose(fpFinal);
	tabela_destruir(&table);

	if ((source_file_size = get_size(source_filename)) == -1) {
		return ERR_FSTATUS;
	}

	if ((final_file_size = get_size(final_filename)) == -1) {
		return ERR_FSTATUS;
	}

	fprintf(stderr,"Compression ratio: %s ", source_filename);

	FREE(final_filename);

	return compress_ratio(final_file_size, source_file_size);
}

/**
* Write binary code in the .palz file.
* @param table hashtable with distinct words and separators
* @param fpSource source file
* @param fpFinal final file
* @param bytes number of bytes
* @return 0 if write binary was successful
*/
int write_binary(HASHTABLE_T **table, FILE **fpSource, FILE **fpFinal,
																																		 int bytes){
	int *result = NULL;
	FILE *srcFile = NULL;
	FILE *finalFile = NULL;
	srcFile = *fpSource;
	finalFile = *fpFinal;
	char *separator = MALLOC(sizeof(char)*2); /* separator plus \0 */
	char *word = NULL;
	int noc = 0; /* number of characters */
	int nor = 0; /* number of repetitions */
	int nor_temp = 0; /* temporary number of repetitions */
	int last_separator = -1; /* -1 because fgetc() returns an unsigned integer */
	int read;
	int repetition_mark = 0;

	rewind(srcFile);

	while((read = fgetc(srcFile))){
		/* (noc+1) because at the beginning noc=0 */
		word = realloc(word, sizeof(char)*(noc+1));

		/* Detect a separator or the end of file (EOF) */
		if(strchr(separators, read) || (read == EOF)) {

			/* Check for a separator's repetition */
			if((read == last_separator) && (read != EOF)) {

				/* Write the separator */
				if(nor == 0){
					fwrite(&repetition_mark, bytes, 1, finalFile);
				}
				nor++;

			} else {

				/* Write the number of repetitions for a certain separator */
				if(nor != 0){

					/* Write with 1 byte */
					if(bytes == 1){
						while(nor > 0){
							if(nor > 255){
								nor_temp = 255;
								fwrite(&nor_temp, 1, bytes, finalFile);
								fwrite(&repetition_mark, 1, bytes, finalFile);
								nor -= 255;
							} else {
								fwrite(&nor, 1, bytes, finalFile);
								nor = 0;
							}
						}
					}

					/* Write with 2 bytes */
					if(bytes == 2){
						while(nor > 0){
							if(nor > 65535){
								nor_temp = 65535;
								fwrite(&nor_temp, 1, bytes, finalFile);
								fwrite(&repetition_mark, 1, bytes, finalFile);
								nor -= 65535;
							} else {
								fwrite(&nor, 1, bytes, finalFile);
								nor = 0;
							}
						}
					}

					/* Write with 3 bytes */
					if(bytes == 3){
						while(nor > 0){
							if(nor > 16777215){
								nor_temp = 16777215;
								fwrite(&nor_temp, 1, bytes, finalFile);
								fwrite(&repetition_mark, 1, bytes, finalFile);
								nor -= 16777215;
							} else {
								fwrite(&nor, 1, bytes, finalFile);
								nor = 0;
							}
						}
					}
					last_separator = -1; /* Reset last separator */
				}

				/*
				* At this time we have found the first separator after a word. So, now
				* we must write the word read before.
				*/
				if (noc != 0){
					word[noc] = '\0';
					result = tabela_consultar(*table, word);
					if(result){
						fwrite(result, bytes, 1, finalFile);
					}
					noc = 0;
				}

				/* Now it's time to write the separator */
				if(read != EOF){
					separator[0] = read;
					separator[1] = '\0';
					result = tabela_consultar(*table, separator);
					fwrite(result, bytes, 1, finalFile);
					last_separator = read;

				} else { /* Read EOF */
					FREE(word);
					FREE(separator);
					return 0;
				}
			}

			/* Fill word with char read */
		} else {
			last_separator = -1;
			word[noc] = read;
			noc++;
		}
	}
	FREE(word);
	FREE(separator);
	return 0;
}

/**
* Compare two words. This function was copied from qsort's man page.
* @param p1 first word to compare
* @param p2 second word to compare
* @return the result of strcmp()
*/
int cmpstringp(const void *p1, const void *p2){
	/* The actual arguments to this function are "pointers to
	pointers to char", but strcmp(3) arguments are "pointers
	to char", hence the following cast plus dereference */
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}

/**
* Insert separators in the hashtable.
* @param table hashtable to fill
*/
void add_separators(HASHTABLE_T **table){
	HASHTABLE_T *aux = NULL;
	aux = *table;

	int *number = NULL;

	number = MALLOC(sizeof(int));
	*number = 1;
	tabela_inserir(aux, "\n", number);

	number = MALLOC(sizeof(int));
	*number = 2;
	tabela_inserir(aux, "\t", number);

	number = MALLOC(sizeof(int));
	*number = 3;
	tabela_inserir(aux, "\r", number);

	number = MALLOC(sizeof(int));
	*number = 4;
	tabela_inserir(aux, " ", number);

	number = MALLOC(sizeof(int));
	*number = 5;
	tabela_inserir(aux, "?", number);

	number = MALLOC(sizeof(int));
	*number = 6;
	tabela_inserir(aux, "!", number);

	number = MALLOC(sizeof(int));
	*number = 7;
	tabela_inserir(aux, ".", number);

	number = MALLOC(sizeof(int));
	*number = 8;
	tabela_inserir(aux, ";", number);

	number = MALLOC(sizeof(int));
	*number = 9;
	tabela_inserir(aux, ",", number);

	number = MALLOC(sizeof(int));
	*number = 10;
	tabela_inserir(aux, ":", number);

	number = MALLOC(sizeof(int));
	*number = 11;
	tabela_inserir(aux, "+", number);

	number = MALLOC(sizeof(int));
	*number = 12;
	tabela_inserir(aux, "-", number);

	number = MALLOC(sizeof(int));
	*number = 13;
	tabela_inserir(aux, "*", number);

	number = MALLOC(sizeof(int));
	*number = 14;
	tabela_inserir(aux, "/", number);
}

/**
* Search for non-palz files in a given folder and sub-folders. For every
* non-palz file found, call compress_file() function using threads. Each file
* must be compressed by one thread only.
* @param directory main directory where to start
* @param max_threads maximum number of threads
* @return 0 at end
* @see compress_file()
*/
int parallel_folder_compress(char *directory, int max_threads){
	char **files_to_compress = NULL;
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
	param.mode = COMPRESS_MODE;
	param.dictionary = NULL;

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
	if ((output = get_files_from_dir(directory, &files_to_compress, &amount,
																													COMPRESS_MODE)) < 0) {
		get_error_msg(output, NULL);
	}

	/* Give all files to compress to write on buffer */
	for(i = 0; i<amount; i++){
		write_on_buffer(files_to_compress[i], &param);
		FREE(files_to_compress[i]);
	}
	FREE(files_to_compress);

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
