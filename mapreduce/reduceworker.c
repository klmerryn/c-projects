
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
#include "reduceworker.h"

/*
 * Launches reduceworker and pass a file stream pointer to 
 * reduce_worker for reading.
 */
void reduce_worker_file(FILE *outfd, int infd) {
	Pair pair;
	LLKeyValues *llkeyval = malloc(sizeof(LLKeyValues));
	llkeyval = NULL; // so no confusing/garbage value

	while (read(infd, &pair, sizeof(Pair)) > 0) {
		insert_into_keys(&llkeyval, pair); // build a list of pairs
	}
	LLKeyValues *curr = llkeyval;  

	// call reduce for every key in linked list
	while (llkeyval) {
		Pair rpair = reduce(curr->key, curr->head_value);
		fwrite(&rpair, sizeof(Pair), 1, outfd);
		curr = curr->next;
	}
	fclose(outfd);
	free_key_values_list(llkeyval);
}

/*
 * Reads from a file stream, accumulating linked lists of Key/Value Pairs.
 * Calls reduce once on each key after all Pairs have been added to linked list.
 */
void launch_reduceworker(int pipes_in[][2], int pipes_out[][2], int curr) {
	char filename[MAX_FILENAME];
	snprintf(filename, MAX_FILENAME, "%d.out", getpid());
	FILE *file = fopen(filename, "w");
	if (!file) {
		perror("fopen");
		exit(1);
	}
	reduce_worker_file(file, pipes_in[curr][0]);
}