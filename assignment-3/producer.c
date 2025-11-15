// ============================================
// producer.c - Producer process starter
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
    
    // Close semaphores (don't unlink - other processes may be using)
    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig) { // Handle termination signals
    printf("\nProducer: Caught signal %d, cleaning up...\n", sig); // Notify user
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }
    
    int producer_id = atoi(argv[1]); // Unique ID for this producer
    int num_items = atoi(argv[2]);   // Number of items to produce
    
    // Set up signal handlers
    signal(SIGINT, signal_handler); 
    signal(SIGTERM, signal_handler);
    
    // Seed random number generator
    srand(time(NULL) + producer_id);
    
    // Attach to shared memory
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
    
    if (shm_id < 0) { // Error handling
        perror("producer shmget");
        exit(1);
    }

    buffer = (shared_buffer_t*)shmat(shm_id, NULL, 0);

    if (buffer == (void*)-1) { // Error handling
        perror("producer shmat");
        exit(1);
    }
    
    // Open semaphores
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0644, BUFFER_SIZE);
    full = sem_open(SEM_FULL, O_CREAT, 0644, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) { // Error handling
        perror("producer sem_open");
        cleanup();
        exit(1);
    }

    sem_wait(mutex); // Enter critical section to initialize buffer if needed

    if (!buffer->initialized) {
        buffer->head = 0;            // Producer write index
        buffer->tail = 0;            // Consumer read index
        buffer->count = 0;           // No items present yet
        buffer->initialized = 1;     // Mark as initialized
    }

    sem_post(mutex);

    printf("Producer %d: Starting to produce %d items\n", producer_id, num_items); // Notify user
    
    // Main production loop
    for (int i = 0; i < num_items; i++) {
        // Create item
        item_t item;
        item.value = producer_id * 1000 + i;  // Generate unique value
        item.producer_id = producer_id;       // Record producer ID
        
        // Wait for empty slot
        sem_wait(empty);
        
        // Enter critical section
        sem_wait(mutex);
        
        // Add item to buffer
        buffer->buffer[buffer->head] = item;               // Place item in buffer
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;   // Update head index
        buffer->count++;                                   // Increment item count
        
        printf("Producer %d: Produced value %d\n", producer_id, item.value); // Notify user
        
        // Exit critical section
        sem_post(mutex);
        
        // Signal item available
        sem_post(full);
        
        // Simulate production time
        usleep(rand() % 100000);
    }
    
    printf("Producer %d: Finished producing %d items\n", producer_id, num_items); // Notify user
    cleanup(); // Clean up resources
    return 0;  // Signal success
}