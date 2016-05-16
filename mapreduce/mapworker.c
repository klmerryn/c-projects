
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "mapworker.h"
#include "mapreduce.h"
#include "helpers.h"

/*
 * Launch function to open filename and pass a pointer to the filename 
 * to mapworker for reading, chunking, and mapping.
 * 
 */
void launch_mapworker(int pipes_in[][2], int pipes_out[][2], int curr, const char *dirname) {
	char buff[MAX_FILENAME];
	while (read(pipes_in[curr][0], buff, MAX_FILENAME) == (size_t) MAX_FILENAME) { // see
		char filename[MAX_FILENAME];
		snprintf(filename, MAX_FILENAME, "%s%s", dirname, buff);
		filename[MAX_FILENAME - 1] = '\0'; // safety
		
		//Open the file's stream for use by mapworker
		FILE *filefd = fopen(filename, "r");
		if (!filefd) {
			fprintf(stderr, "problem with %s\n", filename);
			perror("open");
			exit(1);
		}
		// Call map_worker wrapper; it closes filefd when finished
		map_worker_fread(pipes_out[curr][1], filefd);	
	}
	close(pipes_in[curr][0]);
}


/*
 * Reads READSIZE bytes at a time from stream FILE *infd and passes 
 * a string of those bytes to the mapworker.
 */
void map_worker_fread(int outfd, FILE *infd) {
	char buff[READSIZE + 1];
	int q;
	while ((q = fread(buff, sizeof(char), READSIZE, infd)) > 0 ) {
		buff[READSIZE] = '\0'; // null-terminate
	 	map(buff, outfd);
	}
	//finished reading from this particular file
	fclose(infd);
}


