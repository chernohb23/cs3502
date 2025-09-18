#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
	int pipe1 [2]; // Parent to child
	int pipe2 [2]; // Child to parent
	pid_t pid;

	// TODO : Create both pipes
	// if ( pipe ( pipe1 ) == -1) { /* error handling */ }

	if(pipe(pipe1) == -1){
		perror("pipe1"); // Print message on error
		return 1;
	}
	if(pipe(pipe2) == -1){
		perror("pipe2"); // Print message on error
		return 1;
	}

	// TODO : Fork process

	pid = fork(); // (PID > 0) == Parent, (PID == 0) == Child
	if(pid == -1) {
		perror("fork"); // Print message on error
		return 1;
	}

	if (pid == 0) {
		// Child process
		// TODO : Close unused pipe ends
		// close (pipe1 [1]) ; // Close write end of pipe1
		// close (pipe2 [0]) ; // Close read end of pipe2

		close(pipe1[1]); // Child doesn't write to pipe1
		close(pipe2[0]); // Child doesn't read from pipe2

		// Turns FDs into FILE* for easier I/O
		FILE *from_parent = fdopen(pipe1[0], "r");
		FILE *to_parent = fdopen(pipe2[1], "w");
		if(!from_parent || !to_parent) {
			perror("fdopen child"); // Print message on error
			return 1;
		}

		// TODO : Read from parent , send responses

		char *line = NULL;
		size_t cap = 0;
		ssize_t len;
		while((len = getline(&line, &cap, from_parent)) != -1) { // Child repeatedly reads lines from parent
			fprintf(to_parent, "Child (pid %d) received: %s", getpid(), line); // Sends reply back through pipe2
			fflush(to_parent); // Pushes data quickly
		}
		free(line); // Free allocated memory
		fclose(from_parent); // Closes pipes
		fclose(to_parent);
		return 0;
	} else {
		// Parent process
		// TODO : Close unused pipe ends
		// close (pipe1 [0]) ; // Close read end of pipe1
		// close (pipe2 [1]) ; // Close write end of pipe2

		close(pipe1[0]); // Parent doesn't read from pipe1
		close(pipe2[1]); // Parent doesn't write to pipe2

		// Turns FDs into FILE* for easier I/O
		FILE *to_child = fdopen(pipe1[1], "w");
		FILE *from_child = fdopen(pipe2[0], "r");
		if(!to_child || !from_child) {
			perror("fdopen parent"); // Print message on error
			return 1;
		}

		// TODO : Send messages , read responses
		// TODO: wait () for child

		char *line = NULL;
		size_t cap = 0;
		ssize_t len;
		fprintf(stderr, "Parent: type message to send to child. Type 'exit' to quit.\n"); // Prompts user on stderr for input because stdout is reserved for child's replies
		while(1) {
			fprintf(stderr, ">");
			fflush(stderr);
			if((len = getline(&line, &cap, stdin)) == -1){ // Reads line from user
				break;
			}
			// Send input to child
			if(fwrite(line, 1, (size_t)len, to_child) != (size_t)len) {
				perror("write to child");
				break;
			}
			fflush(to_child); // Pushes data quickly

			// Exit condition
			if(strncmp(line, "exit", 4) == 0) {
				break;
			}

			// Read child's response
			char respbuf[4096];
			if(fgets(respbuf, sizeof(respbuf), from_child) != NULL) { // Gets child's response
				printf("%s", respbuf); // Prints child's response to stdout
				fflush(stdout); // Pushes data quickly
			} else {
				break;
			}
		}

		free(line); // Free allocated memory
		fclose(to_child); // Closes pipes
		fclose(from_child);
		wait(NULL); // Wait for child to finish
	}

	return 0; // Signal success
}
