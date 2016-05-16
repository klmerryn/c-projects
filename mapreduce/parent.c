
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "helpers.h"
#include "mapreduce.h"
#include "linkedlist.h"
#include "reduceworker.h"
#include "mapworker.h"
#include "parent.h"

/*
 * Close the unneeded (write) end of the pipe between parent and child,
 * which is an ls process, and call wait so that does not terminate before process. 
 */
void ls_parent_setup(int *fd_ls) {
	close(fd_ls[1]); // write end closed
	dup2(fd_ls[0], fileno(stdin)); // parent's stdin will read from pipe
}

/*
 * Read from stdin and write information (filenames) to mapworke r pipes,
 * rotating through mapworker functions and writing one filename at a time
 * to each mapworker's pipe.
 * This approach ensures an approximately even distribution of filenames
 * to mapworkers.
 */
void parent_read_stdin(int pipe_arr[][2], int num_map) {
	char s[MAX_FILENAME];
	long int n = 0;
	while (scanf("%s", s) != EOF) { // read from stdin
		// Assign files by rotating through workers; only write to map worker pipes
		int which = n % num_map;
		int len = (int) strlen(s);
		s[len] = '\0';
		if (!(write(pipe_arr[which][1], s, MAX_FILENAME) > 0)) {
			fprintf(stderr, "write failure: %s to map_pipes[%d][1]\n", s, which);
		}
		n++;
	}
	// Finished reading; close pipes that were in use
	for (int i = 0; i < num_map; i++) {
		check_close(pipe_arr[i][1]);
	}
}

/*
 * Read an instance of struct Pair one at a time from each child's pipe until it is empty.
 * Based on the key for that Pair, assign to a reduce worker (by writing to its pipe)
 * so that specific key ranges are reserved for specific reduce workers.  
 */
void parent_read_mapworker(int pipe_in[][2], int pipe_out[][2], int nmap, int nreduce) {
	// read one at a time from each child's pipe until exhausted.
	Pair pair;
	for (int i = 0; i < nmap; i++) {
		while (read(pipe_in[i][0], &pair, sizeof(Pair)) > 0) {
			int ind = assign_rworker(pair, nreduce);
			if (ind >= 0) { //block non-key entries
				parent_write_reduceworker(pipe_out[ind + nmap][1], &pair);
			}
		}
		// close empty pipes
		check_close(pipe_in[i][0]);
	}
	for (int i = 0; i < nmap; i++) {
		check_close(pipe_out[i + nmap][1]); // close reduce worker pipes since done writing
	}
}

/*
 * Error-checked write of Pair to reduceworker, exits with message if write failure.
 */
void parent_write_reduceworker(int infd, const Pair *pair) {
	if (write(infd, pair, sizeof(Pair)) < 0) {
		perror("reduceworker write");
		exit(6);
	}
}

/*
 * Assign keys in a certain range to specific reduceworkers. Since 
 * user specifies number of workers, ranges are calculated at
 * runtime.
 * 
 * Returns an int specifying which reduceworker to assign to; the 
 * reduceworker's absolute position in the pipe array is given by
 * [this int + number of mapworkers.]
 *
 * ***NOTE***: Assignment is based on the first character of the key, which
 * could be any of 256 possible ANSI characters.
 * However, this means that, with a small number of workers, it may appear that a few
 * workers are not assigned keys or are assigned very few, if for example the actual keys are 
 * exclusively alphanumeric characters, as occurs in .txt files.
 * (As per Piazza)
 */
int assign_rworker(const Pair pair, int nworker) {
	int range = NCHAR_MAX / nworker; // int division
	for (int i = 0; i < nworker; i++) {
		if ((pair.key[0] < ((i + 1) * range)) && (pair.key[0] >= (i * range))) {
			return i;
		} 
	}
	// Assignment error
	//fprintf(stderr, "Assignment error (key %s)\n", pair.key);
	return -1;
}
