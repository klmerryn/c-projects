/*NAME
	hogs - resource-hogging identifier

SYNPOSIS
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

DESCRIPTION 
	Using formatted input of currently-active processes and an optional flag
	to indicate filter by CPU or memory, print to stdout the PID, uUsage, and command. 
	If no matching processes, no output printed.

AUTHORS
	c5devere, 2016-02-03

RETURNS
	Zero if process executes and finds a 'hog', or if no processes match a given user.
	Returns an int representing (argc + 1) if the wrong number of command arguments
	are specified. Returns -1 if an invalid flag is given.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* Constants */
#define MEMU 'm'
#define CPUU 'c'
#define MAXLEN 127

int pid;
float usage = 0;
char command[MAXLEN + 1];
int *pid_ptr = &pid;
float *usage_ptr = &usage;


/* Compare and assign values for process id, usage stats, and commands.*/
void compare_hogs(int *new_pid, float *new_usage, char **new_command) {


	if (!pid) { // No previous processes have been found for this user 
		*pid_ptr = *new_pid;
		*usage_ptr = *new_usage;
		strncpy(command, *new_command, MAXLEN);
	}

	else if (*new_usage > usage) {
		*pid_ptr = *new_pid;
		*usage_ptr = *new_usage;
		strncpy(command, *new_command, MAXLEN);
	}

	// lexographic ordering of 'command' to break tie
	else if (*new_usage == usage) {
		if (strcasecmp(*new_command, command) < 0) {
			*pid_ptr = *new_pid;
			*usage_ptr = *new_usage;	
			strncpy(command, *new_command, MAXLEN);
		}
	}
}
 
/* Scans from formatted input of current processes. If process owner
id matches specified user id, calls compare_hogs to determine whether
process is more resource-consumptive.*/
int find_hogs(char **username, const char c) {

	char new_uid[MAXLEN + 1];
	int new_pid;
	float new_usage_cpu;
	float new_usage_mem;
	char new_command[MAXLEN + 1];
	char *new_command_ptr = new_command;
	char ch;

	scanf("%*[^\n]"); // Skip first line of headers

	while (scanf("%s %d %f %f %*d %*d %*s %*s %*s %*s %127[^\n]%c",
		new_uid, &new_pid, &new_usage_cpu, &new_usage_mem, new_command_ptr, &ch) != EOF) {

		// Match username to process owner
		if (strcmp(*username, new_uid) == 0) { 

			// Comparison to current resource hog
			if (c == MEMU) {
				compare_hogs(&new_pid, &new_usage_mem, &new_command_ptr);
			}
			else if (c == CPUU) { 
				compare_hogs(&new_pid, &new_usage_cpu, &new_command_ptr);
			}

			else { // Not supposed to be reached since A1 handout assumes valid flag
				fprintf(stderr, "Usage: hogs [-c | -m] username\n");
				return -1;
			}
		}
	}
	return 0;
} 


/* Determine the most usage-intense process for specified user by either 
memory or CPU cycles.*/
int main(int argc, char **argv){

	char user[MAXLEN + 1];
	char *username = user;
	
	// Too few or too many arguments; print to sterror, nonzero exit
	if (argc > 3 || argc < 2) {
		fprintf(stderr, "Usage: hogs [-c | -m] username\n");
		return (1 + argc);
	}

	// Only username provided
	else if (argc == 2) {
		strncpy(user, argv[1], MAXLEN);
		user[MAXLEN + 1] = '\0';
		find_hogs(&username, CPUU);
	}

	// Two arguments provided (flag then username according to A1 handout)
	else {
		strncpy(user, argv[2], MAXLEN);
		user[MAXLEN + 1] = '\0';

		// Valid flag is '-c' or '-m'
		if (argv[1][0] != '-' || (argv[1][1] != 'c' && argv[1][1] != 'm')) {
			fprintf(stderr, "Usage: hogs [-c | -m] username\n");
			return -1;
		}
		const char c = argv[1][1];
		find_hogs(&username, c);
	}

	// According to instructors, print '\n' if no matching processes
	if (!pid) {
		printf("\n");
	}
	else {
		printf("%d\t%.1f\t%s\n", pid, usage, command);
	}
	return 0;
}

