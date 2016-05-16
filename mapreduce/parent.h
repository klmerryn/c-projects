
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

/*
 * Close the unneeded (write) end of the pipe between parent and child,
 * which is an ls process, and call wait so that does not terminate before process. 
 */
void ls_parent_setup(int *fd_ls);

/*
 * Read from stdin and write information (filenames) to mapworke r pipes,
 * rotating through mapworker functions and writing one filename at a time
 * to each mapworker's pipe.
 * This approach ensures an approximately even distribution of filenames
 * to mapworkers.
 */
void parent_read_stdin(int pipe_arr[][2], int num_map);


/*
 * Read an instance of struct Pair one at a time from each child's pipe until it is empty.
 * Based on the key for that Pair, assign to a reduce worker (by writing to its pipe)
 * so that specific key ranges are reserved for specific reduce workers.  
 */
void parent_read_mapworker(int pipe_in[][2], int pipe_out[][2], int nmap, int nreduce);

/*
 * Error-checked write of Pair to reduceworker, exits with message if write failure.
 */
void parent_write_reduceworker(int infd, const Pair *pair);

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
 * workers are not assigned keys or are assigned very few, if for example the keys are 
 * exclusively alphanumeric characters, as occurs often in .txt files.
 * (As per Piazza)
 */
int assign_rworker(const Pair pair, int nworker);
