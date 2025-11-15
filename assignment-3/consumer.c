// ============================================
// consumer.c - Consumer process starter
// ============================================
#include "buffer.h"

// Global variables for cleanup
shared_buffer_t* buffer = NULL; // Pointer for shared memory
sem_t* mutex = NULL;            // Protects critical section
sem_t* empty = NULL;            // Tracks empty slots
sem_t* full = NULL;             // Tracks full slots
int shm_id = -1;                // Shared memory segment ID

void cleanup() {
    // Detach shared memory
    if (buffer != NULL) {
        shmdt(buffer);
    }
    
    // Close semaphores
    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig) { // Handle termination signals
    printf("\nConsumer: Caught signal %d, cleaning up...\n", sig); // Notify user
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        exit(1);
    }
    
    int consumer_id = atoi(argv[1]); // Unique ID for this consumer
    int num_items = atoi(argv[2]);   // Number of items to consume
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Seed random number generator
    srand(time(NULL) + consumer_id * 100);
    
    // Attach to shared memory
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);

    if (shm_id < 0) { // Error handling
        perror("consumer shmget");
        exit(1);
    }

    buffer = (shared_buffer_t*)shmat(shm_id, NULL, 0);

    if (buffer == (void*)-1) { // Error handling
        perror("consumer shmat");
        exit(1);
    }
    
    // Open semaphores
    mutex = sem_open(SEM_MUTEX, 0);
    empty = sem_open(SEM_EMPTY, 0);
    full = sem_open(SEM_FULL, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) { // Error handling
        perror("consumer sem_open");
        cleanup();
        exit(1);
    }
    
    printf("Consumer %d: Starting to consume %d items\n", consumer_id, num_items); // Notify user
    
    // Main consumption loop
    for (int i = 0; i < num_items; i++) {
        // Wait for full slot
        sem_wait(full);
        
        // Enter critical section
        sem_wait(mutex);
        
        // Remove item from buffer
        item_t item = buffer->buffer[buffer->tail];      // Consume item from tail
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE; // Update tail index
        buffer->count--; 					             // Decrement item count
        
        printf("Consumer %d: Consumed value %d from Producer %d\n", // Notify user
                consumer_id, item.value, item.producer_id);
        
        // Exit critical section
        sem_post(mutex);
        
        // Signal empty slot
        sem_post(empty);
        
        // Simulate consumption time
        usleep(rand() % 100000);
    }
    
    printf("Consumer %d: Finished consuming %d items\n", consumer_id, num_items); // Notify user
    cleanup(); // Clean up resources
    return 0;  // Signal success
}