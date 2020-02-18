/**
* @file main.c
* @brief Main functions.
* @date 2014-2015
* @author Fabio Santos <ffsantos92@gmail.com>
* @author Eurico Sousa <2110133@my.ipleiria.pt>
*/
#include "common.h"
#include "decompress.h"
#include "compress.h"
#include <pthread.h>

/* External variables */
extern int got_signal;

/**
* Signal handling.
* @param signal value of signal
*/
static void signal_handler(int signal){
	if(signal == SIGINT){
		got_signal = 1;
	}
}

/**
* Main function.
* @param argc number of parameters
* @param arvg array with all parameters
*/
int main(int argc, char *argv[]){
	struct gengetopt_args_info args;

	struct sigaction act;
	act.sa_handler = &signal_handler;


	/**
	* The sa_mask field allows us to specify a set of signals that aren’t
	* permitted to interrupt execution of this handler
	*/
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	float output = 0;

	struct timeval tb, te;
	gettimeofday(&tb, NULL);

	TDictionary *dictionary = NULL;

	if (sigaction (SIGINT, &act, NULL) < 0){ /* -1 = error */
		ERROR(1, "sigaction - SIGINT");
	}

	/**
	* It takes the arguments that main receives and a pointer to such a struct,
	* that it will be filled
	*/
	if (cmdline_parser(argc, argv, &args) != 0) {
		exit(1);
	}
	/* Check for at least one parameter */
	if (argc > 1) {

		/* --decompress <file> */
		if (args.decompress_given) {
			decompress_resources_init(&dictionary);
			if ((output = decompress_file(&dictionary, args.decompress_arg)) < 0){
				get_error_msg(output, args.decompress_arg);
			}else{
				fprintf(stderr,"%.2f %%\n", output);
			}
		}

		/* --folder-decompress <folder> */
		else if (args.folder_decompress_given) {
			decompress_resources_init(&dictionary);
			decompress_folder(&dictionary, args.folder_decompress_arg);
		}

		/* --compress <file> */
		else if (args.compress_given) {
			if ((output = compress_file(args.compress_arg)) < 0){
				get_error_msg(output, args.compress_arg);
			}else{
				fprintf(stderr,"%.2f %%\n", output);
			}
		}

		/* --parallel-folder-compress <folder> --compress-max-threads <nthreads> */
		else if (args.parallel_folder_compress_given) {
			int max_threads = args.compress_max_threads_arg;

			parallel_folder_compress(args.parallel_folder_compress_arg, max_threads);
		}

		/**
		* --parallel-folder-decompress <folder> --decompress-max-threads <nthreads>
		*/
		else if (args.parallel_folder_decompress_given) {
			int max_threads = args.decompress_max_threads_arg;

			decompress_resources_init(&dictionary);
			parallel_folder_decompress(&dictionary,
															args.parallel_folder_decompress_arg, max_threads);
		}
		
		/* --about */
		else if (args.about_given) {
			printf("*************************************\n");
			printf("************* palz v2.0 *************\n");
			printf("*************************************\n");
			printf("Fabio Santos <ffsantos92@gmail.com>\n");
			printf("Eurico Sousa <2110133@my.ipleiria.pt>\n");
			printf("*************************************\n");
			exit(EXIT_SUCCESS);
		} else {
			printf("palz: unrecognized syntax. Try 'palz --help' for more info.\n");
			exit(EXIT_FAILURE);
		}
	} else {
		printf("palz: unrecognized syntax. Try 'palz --help' for more info\n");
		exit(EXIT_FAILURE);
	}

	if (got_signal){
		got_signal = 0;

		/* Release allocated memory */
		cmdline_parser_free(&args);

		/* Free decompress resources */
		if(dictionary != NULL) {
			decompress_resources_free(&dictionary);
		}

		/* Code based on strftime page from manual */
		time_t t;
		struct tm *tm_info;
		t = time(NULL);
		char aux[25];
		tm_info = localtime(&t);
		strftime(aux, 25, "%Y-%m-%d %Hh%M", tm_info); /* format: 2014-09-15 16h02 */

		fprintf(stderr, "Operation interrupted by user @%s\n", aux);
		exit(0);
	}

	/* Release allocated memory */
	cmdline_parser_free(&args);

	if(dictionary != NULL) {
		decompress_resources_free(&dictionary);
	}

	gettimeofday(&te, NULL);
	fprintf(stderr, "Execution time: %.2f s\n", (((te.tv_sec-tb.tv_sec) * 1000 +
																				(te.tv_usec-tb.tv_usec)/1000.0))*0.001);

	return 0;
}
