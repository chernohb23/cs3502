#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
  FILE *input = stdin;
	int buffer_size = 4096;
	char opt;

	// Extra varibles
	char *filename = NULL;
	int getopt_ret;
	char *buffer = NULL;
	size_t nread;

	// TODO : Parse command line arguments
	// -f filename ( optional )
	// -b buffer_size ( optional )

	while((getopt_ret = getopt(argc, argv, "f:b:vh")) != -1) {
		opt = (char)getopt_ret;
		switch(opt) {
			case 'f': // Input file
				filename = optarg;
				break;
			case 'b': // Buffer size
				buffer_size = atoi(optarg);
				if(buffer_size <= 0){
					fprintf(stderr, "Invalid buffer size: %s\n", optarg);
					return 1;
				}
				break;
			case 'v': // Verbose but it's not used here
				break;
			case 'h': // Help
			default:
				fprintf(stderr, "Usage: %s [-f file] [-b size]\n", argv[0]);
				return 1;
		}
	}

	// TODO : Open file if -f provided

	if(filename != NULL) {
		input = fopen(filename, "rb");
		if(input == NULL) {
			perror("fopen");
			return 1;
		}
	}

	// TODO : Allocate buffer

	buffer = (char*)malloc((size_t)buffer_size); // Buffer is allocated
	if(buffer == NULL) { // If allocation fails then:
		perror("malloc"); // Print error and close file if opened
		if(input != stdin){
			fclose(input);
		}
		return 1;
	}

	// TODO : Read from input and write to stdout

	while((nread = fread(buffer, 1, (size_t)buffer_size, input)) > 0) { // Repeatedly read data from input file
		size_t nwritten = fwrite(buffer, 1, nread, stdout); // Write bytes into standard output
		if(nwritten != nread) { // If fewer bytes were written than read then:
			perror("fwrite"); // Read then print error and exit
			free(buffer);
			if(input != stdin){
				fclose(input);
			}
			return 1;
		}
	}

	fflush(stdout);

	// TODO : Cleanup

	free(buffer); // Free allocated memory
	if(input != stdin) fclose(input); // Close input file if opened
	return 0; // Signal success
}
