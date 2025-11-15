// ============================================
// buffer.h - Shared definitions (INCOMPLETE - You must complete this!)
// ============================================
#ifndef BUFFER_H
#define BUFFER_H

// Required includes for both producer and consumer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Constants for shared memory and semaphores
#define BUFFER_SIZE 10                  // Size of the buffer
#define SHM_KEY 0x1234                  // Shared memory key
#define SEM_MUTEX "/sem_mutex"          // Semaphore for mutual exclusion
#define SEM_EMPTY "/sem_empty"          // Semaphore for empty slots
#define SEM_FULL "/sem_full"            // Semaphore for full slots


// Item structure
typedef struct {
	int value;        // Data value
	int producer_id;  // Which producer created this
} item_t;

// Shared buffer structure
typedef struct {
	item_t buffer[BUFFER_SIZE];
	int head;        // Next write position
	int tail;        // Next read position
	int count;       // Current items in buffer
	int initialized; // 0 until first-time init under mutex
} shared_buffer_t;

#endif
