#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

// Global flags for signal handlers
volatile sig_atomic_t shutdown_flag = 0; // Set to 1 when SIGINT receieved
volatile sig_atomic_t stats_flag = 0; // Set to 1 when SIGUSR1 receieved

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

// consumer.c stuff
int main (int argc, char *argv[]){
    int max_lines = -1;
    int verbose = 0;

    int getopt_ret;
    char opt;
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    long long line_count = 0;
    long long char_count = 0;

    // Setup signal handlers (SIGINT and SIGUSR1)
    struct sigaction sa_int, sa_usr1;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction(SIGINT)");
        // Continue
    }

    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction(SIGUSR1)");
        // Continue
    }

    // Start time measurement
    clock_t start_clock = clock();

    // consumer.c stuff
    while((getopt_ret = getopt(argc, argv, "n:vh")) != -1) {
        opt = (char)getopt_ret;
        switch(opt){
            case 'n':
                max_lines = atoi(optarg);
                if(max_lines < 0){
                    max_lines = -1;
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n max_lines] [-v]\n", argv[0]);
                return 1;
        }
    }

    while((linelen = getline(&line, &linecap, stdin)) != -1) {

        // If a shutdown was requested, break out gracefully
        if (shutdown_flag) {
            break;
        }

        line_count++;
        char_count += linelen;

        // If verbose, echo lines to stdout
        if(verbose) {
            ssize_t w = fwrite(line, 1, (size_t)linelen, stdout);
            (void)w;
            fflush(stdout);
        }

        if(max_lines != -1 && line_count >= max_lines){
            break;
        }

        // If user requested stats print (SIGUSR1), print current stats to stderr
        if (stats_flag) {
            clock_t now = clock();
            double elapsed = ((double)(now - start_clock)) / CLOCKS_PER_SEC;
            if (elapsed <= 0.0) elapsed = 1e-9;
            double lines_per_sec = (double)line_count / elapsed;
            double bytes_per_sec = (double)char_count / elapsed;
            double mbps = (double)char_count / 1024.0 / 1024.0 / elapsed;
            fprintf(stderr, "[consumer SIGUSR1] lines: %lld bytes: %lld time(s): %.6f lines/s: %.6f bytes/s: %.6f MB/s: %.6f\n",
                    line_count, char_count, elapsed, lines_per_sec, bytes_per_sec, mbps);
            stats_flag = 0;
        }
    }

    // getline returns -1 on EOF or if interrupted by signal. If it was interrupted due to SIGINT, shutdown_flag will be set and we will proceed to print final stats
    // Stop time and compute final stats
    clock_t end_clock = clock();
    double elapsed = ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC;
    if (elapsed <= 0.0) elapsed = 1e-9;

    double lines_per_sec = (double)line_count / elapsed;
    double bytes_per_sec = (double)char_count / elapsed;
    double mbps = (double)char_count / 1024.0 / 1024.0 / elapsed;

    fprintf(stderr, "Consumer final stats:\n");
    fprintf(stderr, "Lines: %lld\n", line_count);
    fprintf(stderr, "Characters (bytes): %lld\n", char_count);
    fprintf(stderr, "Time (sec): %.6f\n", elapsed);
    fprintf(stderr, "Lines/s: %.6f\n", lines_per_sec);
    fprintf(stderr, "Bytes/s: %.6f\n", bytes_per_sec);
    fprintf(stderr, "Throughput (MB/s): %.6f\n", mbps);

    free(line);
    return 0; // Signal success
}
