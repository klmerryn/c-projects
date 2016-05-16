
/*
DESCRIPTION

Master processes controlling MapReduce program. Assigns work to user-
specified number of child processes of three separate types:
a first function (ls) to list files in a user-specified directory;
map processes, which support user-specified map implementation, 
information processing in master/parent, and finally, user-supported reduce
process implementation.

ADDITIONAL INFORMATION
- Nonzero exit values and their meanings:
 -1: wrong number of commandline args (with usage message)
 1: fork error. 
 2: Pipe error. 
 3: wait error. 
 4: dup2 error  
 5: close/open error

DATE
2016-03-22
c5devere

*/
#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "helpers.h"
#include "parent.h"
#include "mapworker.h"
#include "reduceworker.h"


int num_map = DEFAULT_NUMMAP;
int num_reduce = DEFAULT_NUMRED;
char *dirname;
 
int main(int argc, char **argv) {

	// Set num_map and num_reduce
	parse_args(argc, argv, &num_map, &num_reduce, &dirname);
	
	// Create pipe and child ls
	int fd_ls[2];
	pipe(fd_ls);
	int pid = fork();
	fork_check(pid); 

	if (pid == 0) { // child; fork_safe guarantees pid !< 0
		ls_child_setup(fd_ls, dirname);
	}

	else if (pid > 0) { // parent
		ls_parent_setup(fd_ls);

		// create worker pipes
		int pipes_masterwrite[NUM_WORKER][2]; // array of pipes: master writes, child reads.
		int pipes_masterread[NUM_WORKER][2]; // array of pipes: master reads, child writes


		for (int i = 0; i < NUM_WORKER; i++) {
			pipe_check(pipe(pipes_masterwrite[i]));
			pipe_check(pipe(pipes_masterread[i]));
		}

		// fork worker processes (all)
		for (int f = 0; f < NUM_WORKER; f++) {
			int result = fork();
			fork_check(result); // Safety: guarantees >= 0

			if (result == 0) { // child

				// close all non appropriate pipes
				close_multi_pipes(pipes_masterwrite, 0, NUM_WORKER, WRITE);
				close_multi_pipes(pipes_masterread, 0, NUM_WORKER, READ);

				
				if (f < num_map) { // a map worker
					launch_mapworker(pipes_masterwrite, pipes_masterread, f, dirname);
					check_close(pipes_masterread[f][WRITE]); //good alone

				}

				else { // a reduce worker
					check_close(pipes_masterread[f][WRITE]); // never writes to pipe //good alone
					launch_reduceworker(pipes_masterwrite, pipes_masterread, f); // close AND hang error
					check_close(pipes_masterwrite[f][READ]); //good alone
				}
				exit(0);
			}

			else if (result > 0) { // parent stuff
				check_close(pipes_masterwrite[f][READ]);
				check_close(pipes_masterread[f][WRITE]);			
			}
		}

		// Only parent gets here
		// Parent reads from stdin, writes to mapworkers via masterwrite pipe
		// Parent closes unneeded pipes in parent_read_stdin (parent.c)
		parent_read_stdin(pipes_masterwrite, num_map);
		
		// Parent reads from map workers one at a time till pipes empty, assigning
		// to reduceworkers' pipes by key ranges
		parent_read_mapworker(pipes_masterread, pipes_masterwrite, num_map, num_reduce);
		
		// Free dynamically-allocated memory. LLKeyVal are freed in reduceworker.
		free(dirname);	
		
	}
	
	// wait for all processes
	for (int j = 0; j < NUM_WORKER + 1; j++) {
		wait(&j);
	}
	return 0;
}