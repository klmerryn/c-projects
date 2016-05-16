
#include <sys/stat.h>
#include <fcntl.h> // ?
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include "helpers.h"
#include "mapreduce.h"
#include "linkedlist.h"

/*
 * Closes all 'which' ends specified (READ | WRITE) of pipes
 * in array of pipes from index start to index stop.
 * Checks for valid close and exits with error message if close fails.   
 */
void close_multi_pipes(int pipe[][2], int start, int stop, int which) {
	for (int i = start; i < stop; i++) {
		check_close(pipe[i][which]);
	}

}

/*
 * Parse optarg value (val) into int and check if valid input. 
 * Returns int on success or -1 on failure or invalid input. 
 */
int check_optarg(char *val, int *target) {
	char *end;
	int r = strtol(val, &end, 10);
	if (r > 0 && *end == '\0') {
		*target = r;
		return *target;
	}
	return -1;
}

/*
 * Check if close call to pipes executes without error.
 */
void check_close(int fd) {
	if (close(fd) == -1) {
		perror("close");
	}
}

/*
 * Error message and non-zero exit (1) if fork fails.
 */
void fork_check(int r) {
	if (r < 0) {
		perror("fork");
		exit(1);
	}
}

/*
 * Error message and non-zero exit (2) if pipe fails.
 */
void pipe_check(int result) {
	if (result != 0) {
		perror("pipe");
		exit(2);
	}
}

/*
 * Error message and non-zero exit (3) if wait fails.
 */
void wait_check(int *status) {
	if ((wait(status)) == -1) {
		perror("wait");
		exit(3);
	}
}

/*
 * Error message and non-zero exit (4) if dup2 fails.
 */
void dup2_check(int fd) {
	if (fd == -1) {
		perror("dup2");
		exit(4);
	}
}

/*
 * Check whether path specified points to a valid nonempty
 * directory.
 * Returns
 *    - 0 if path points correctly to nonempty directory
 *    - -1 if path points to empty directory
 */
int check_dir(char *path, char **dir) {
	// find a place to set dir name. return it pointer to it? or NULL?

	struct dirent *entry;
	
	DIR *d = opendir(path);
	if (!d) {
		fprintf(stderr,"Invalid directory error: exiting\n");
		exit(-1);
	}
	if ((entry = readdir(d))) { // means nonempty directory
		closedir(d);
		int len = strlen(path);
		
		// ensure trailing slash
		if (path[len-1] != '/') {
			len++; // make room for trailing '/' to add when malloc'ing space
			
		}
		*dir = malloc(sizeof(char) * len + 1);
		strncpy(*dir, path, len);
		strncat(*dir, "/", 1);
		return 0;
	}
	return -1;
}

/*
 * Print usage msesage and exit(1).
 * Used in parse_args when incorrect arguments have been found or args are missing.
 */
void print_usage() {
	fprintf(stderr, "Usage: -d directory [-r num_reduceworkers] [-m num_mapworkers ]\n");
	exit(-1);
}

/* 
 * Validate and parse commandline argments. Assign values to global variables num_maps and num_red
 */
void parse_args(int argc, char **argv, int *num_map, int *num_reduce, char **dirname) {
	if (argc < 3) {
		print_usage();
	}
	// Parse flags and check for associated values. 
	int opt;
	int r;
	int dflag = 0;
	while ((opt = getopt(argc, argv, "m:r:d:")) != -1) {
		switch(opt) {
		case 'm':
			if ((check_optarg(optarg, num_map)) != -1) { // if returns a valid non-zero int arg (optarg is 0 by default if unspecified)	
				break;
			}
			fprintf(stderr, "Invalid value for flag -m. Assigned %d\n", DEFAULT_NUMMAP);	
			break;
		case 'r':
			if ((check_optarg(optarg, num_reduce)) != -1) { // if returns a valid non-zero int arg
				break;
			}
			fprintf(stderr, "Invalid value for flag -r. Assigned %d\n", DEFAULT_NUMRED);
			break;
		case 'd':
			if ((r = check_dir(optarg, dirname)) != 0) {
				exit(r);
			}
			dflag = 1;
			break;
		default:  // Means getopt() returned '?'
			print_usage();
		}
	}
	if (!dflag) {
		print_usage();
	}
}

/* 
 * Setup and launch ls child, using dup2 to ensure stdout is directed to pipe.
 */
void ls_child_setup(int *fd_ls, const char *path) {
	close(fd_ls[0]); // read end closed
	dup2(fd_ls[1], fileno(stdout));
	close(fd_ls[1]);
	execl("/bin/ls", "ls", path, NULL);
	perror("exec"); // Should not reach here; exec returns only on error 
	exit(5);
}

/*
 * Calls wait however many times specified, and checks 
 * for non-erorr result.
 */
void multi_wait(int w) {
	for (int i = 0; i < w; i++) {
		wait_check(&w);
	}
}