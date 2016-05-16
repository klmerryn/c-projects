#ifndef HELPERS_H
#define HELPERS_H

#include <sys/stat.h>
#include <fcntl.h>
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

#define DEFAULT_NUMMAP 2
#define DEFAULT_NUMRED 2
#define NUM_WORKER (num_map + num_reduce)
#define NCHAR_MAX 256 // range of ANSI/ASCII to espect
#define READ 0
#define WRITE 1 

/* 
 * Validate and parse commandline argments, assigning
 * values to num_map, num_reduce, and dir (directory).
 * Exit values for failed system calls or invalid input are 
 * spcified in specific calls listed in helpers.c.
 */ 
void parse_args(int arg, char **argv, int *num_map, int *num_reduce, char **dir);

/*
 * Check whether path specified points to a valid nonempty
 * directory.
 * Returns
 *    - 0 if path points correctly to nonempty directory
 *    - -1 if path points to empty directory
 */
int check_dir(char *path, char **dir);

/*
 * Parse optarg value (val) into int and check if valid input. 
 * Returns int on success or -1 on failure or invalid input. 
 */
int check_optarg(char *val, int *target);

/*
 * Closes all 'which' ends specified (READ | WRITE) of pipes
 * in array of pipes from index start to index stop.
 * Checks for valid close and exits with error message if close fails.   
 */
void close_multi_pipes(int pipe[][2], int start, int stop, int which);

void ls_parent_setup(int *fd_ls);

void ls_child_setup(int *fd_ls, const char *path);

/*
 * Calls wait however many times specified, and checks 
 * for non-erorr result.
 */
void multi_wait(int w);

/*
 * Print usage msesage and exit(1).
 * Used in parse_args when incorrect arguments have been found or args are missing.
 */
void print_usage();

/* Error-checked system calls: see helpers.c file for details on exit return values. */
void pipe_check(int p);
void fork_check(int r);
void wait_check(int *status);
void check_close(int r);


#endif