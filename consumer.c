#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

int main (int argc, char *argv[]){
	int max_lines = -1;
	int verbose = 0;

	//Extra variables
	int getopt_ret;
	char opt;
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	long long line_count = 0;
	long long char_count = 0;

	// TODO : Parse arguments ( - n max_lines , -v verbose )

	while((getopt_ret = getopt(argc, argv, "n:vh")) != -1) {
		opt = (char)getopt_ret;
		switch(opt){
			case 'n': // Max lines
				max_lines = atoi(optarg);
				if(max_lines < 0){
					max_lines = -1;
				}
				break;
			case 'v': // Verbose
				verbose = 1;
				break;
			case 'h': // Help
			default:
				fprintf(stderr, "Usage: %s [-n max_lines] [-v]\n", argv[0]);
				return 1;
		}
	}

	// TODO : Read from stdin line by line
	// Count lines and characters

	while((linelen = getline(&line, &linecap, stdin)) != -1) { // Repeatedly reads from stdin into line
		line_count++; // Count line
		char_count += linelen; // Add line's character count to total

		// If verbose, echo lines to stdout

		if(verbose) {
			ssize_t w = fwrite(line, 1, (size_t)linelen, stdout); // Write line back to standard output with fwrite
			(void)w;
			fflush(stdout); // Ensures output appears quickly
		}

		// Stop if reached max lines
		if(max_lines != -1 && line_count >= max_lines){
			break;
		}
	}

	// TODO : Print statistics to stderr
	// fprintf ( stderr , " Lines : % d \ n " , line_count );

	fprintf(stderr, "Lines: %lld\n", line_count);
        fprintf(stderr, "Characters (bytes): %lld\n", char_count);

	free(line); // Free allocated memory
	return 0; // Signal success
}
