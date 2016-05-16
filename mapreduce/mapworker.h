#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "mapreduce.h"
#include "reduceworker.h"

/*
 * Launch function to open filename and pass a pointer to the filename 
 * to mapworker for reading, chunking, and mapping.
 * 
 */
void launch_mapworker(int pipes_in[][2], int pipes_out[][2], int curr, const char *dirname);

/*
 * Reads READSIZE bytes at a time from stream FILE *infd and passes 
 * a string of those bytes to the mapworker.
 */
void map_worker_fread(int outfd, FILE *infd);