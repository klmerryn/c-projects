#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "mapreduce.h"
#include "linkedlist.h"


/*
 * Launches reduceworker and pass a file stream pointer to reduce_worker for reading.
 */
void launch_reduceworker(int pipes_in[][2], int pipes_out[][2], int curr);

/*
 * Reads from a file stream, accumulating linked lists of Key/Value Pairs.
 * Calls reduce once on each key after all Pairs have been added to linked list.
 */
void reduce_worker_file(FILE *outfd, int infd);