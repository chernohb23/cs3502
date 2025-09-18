#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

// Global flags for signal handlers
volatile sig_atomic_t shutdown_flag = 0; // Set to 1 when SIGINT
volatile sig_atomic_t stats_flag = 0; // Set to 1 when SIGUSR1

// Handler for SIGINT
void handle_sigint(int sig) {
    (void)sig;
    shutdown_flag = 1; // Request shutdown
}

// Handler for SIGUSR1
void handle_sigusr1(int sig) {
    (void)sig;
    stats_flag = 1; // Request stats
}

int main(int argc, char *argv[]) {
    // Producer.c stuff
    FILE *input = stdin;
    int buffer_size = 4096;
    char opt;

    char *filename = NULL;
    int getopt_ret;
    char *buffer = NULL;
    size_t nread;

    // Stat counters
    long long bytes_processed = 0;
    long long lines_processed = 0;

    // Setup signal handlers (SIGINT and SIGUSR1)
    struct sigaction sa_int, sa_usr1;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction(SIGINT)");
        // Not fatal; continue
    }

    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction(SIGUSR1)");
        // Not fatal; continue
    }

    // Start time measurement
    clock_t start_clock = clock();

    // Producer.c stuff
    while((getopt_ret = getopt(argc, argv, "f:b:vh")) != -1) {
        opt = (char)getopt_ret;
        switch(opt) {
            case 'f':
                filename = optarg;
                break;
            case 'b':
                buffer_size = atoi(optarg);
                if(buffer_size <= 0){
                    fprintf(stderr, "Invalid buffer size: %s\n", optarg);
                    return 1;
                }
                break;
            case 'v':
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-f file] [-b size]\n", argv[0]);
                return 1;
        }
    }

    if(filename != NULL) {
        input = fopen(filename, "rb");
        if(input == NULL) {
            perror("fopen");
            return 1;
        }
    }

    buffer = (char*)malloc((size_t)buffer_size);
    if(buffer == NULL) {
        perror("malloc");
        if(input != stdin){
            fclose(input);
        }
        return 1;
    }

    while((nread = fread(buffer, 1, (size_t)buffer_size, input)) > 0) {

        // If a shutdown was requested, break out gracefully (stop producing)
        if (shutdown_flag) {
            break;
        }

        // Count bytes
        bytes_processed += (long long)nread;

        // Count lines in this buffer
        for(size_t i = 0; i < nread; ++i) {
            if(buffer[i] == '\n') lines_processed++;
        }

        size_t nwritten = fwrite(buffer, 1, nread, stdout);
        if(nwritten != nread) {

            // fwrite may be interrupted by signal; treat as error
            if (ferror(stdout)) {
                perror("fwrite");
                free(buffer);
                if(input != stdin){
                    fclose(input);
                }
                return 1;
            }
        }

        // If user requested stats print (SIGUSR1), print current stats to stderr
        if (stats_flag) {
            clock_t now = clock();
            double elapsed = ((double)(now - start_clock)) / CLOCKS_PER_SEC;
            if (elapsed <= 0.0) elapsed = 1e-9; // Avoid divide by zero
            double mbps = (double)bytes_processed / 1024.0 / 1024.0 / elapsed;
            fprintf(stderr, "[producer SIGUSR1] bytes: %lld lines: %lld time(s): %.6f MB/s: %.6f\n",
                    bytes_processed, lines_processed, elapsed, mbps);
            stats_flag = 0;
        }
    }

    // Ensure stdout is flushed before printing final stats
    fflush(stdout);

    // Stop time and compute final stats
    clock_t end_clock = clock();
    double elapsed = ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC;
    if (elapsed <= 0.0) elapsed = 1e-9;
    double mbps = (double)bytes_processed / 1024.0 / 1024.0 / elapsed;

    // Print final stats to stderr (also triggered by SIGINT behavior)
    fprintf(stderr, "Producer final stats:\n");
    fprintf(stderr, "Bytes processed: %lld\n", bytes_processed);
    fprintf(stderr, "Lines processed: %lld\n", lines_processed);
    fprintf(stderr, "Time (sec): %.6f\n", elapsed);
    fprintf(stderr, "Throughput (MB/s): %.6f\n", mbps);

    free(buffer);
    if(input != stdin) fclose(input);
    return 0; // Signal success
}
