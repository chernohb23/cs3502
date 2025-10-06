#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define NUM_ACCOUNTS 2 // Number of bank accounts
#define NUM_THREADS 2 // Number of threads
#define TRANSACTIONS_PER_TELLER 5 // Number of transactions per thread
#define INITIAL_BALANCE 1000.0 // Starting balance of each account

typedef struct {
	int account_id; // Unique ID for the account
	double balance; // Current balance for the account
	int transaction_count; // Total number of transactions performed
	pthread_mutex_t lock; // Mutex lock for synchronizing access
} Account;

// Global accounts array (shared resource)
Account accounts[NUM_ACCOUNTS];

pthread_t threads[NUM_THREADS]; // Array that holds teller handles
int thread_ids[NUM_THREADS]; // Array that holds teller IDs

// try_lock function which will try to acquire a mutex lock with a timeout
int try_lock_with_timeout(pthread_mutex_t *lock, int milliseconds) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts); // Get current time

	ts.tv_sec += milliseconds / 1000; // Add timeout interval
	ts.tv_nsec += (milliseconds % 1000) * 1000000;

	// Normalize nanosecs if overflow
	if(ts.tv_nsec >= 1000000000) {
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}

	// Attempt to lock with a deadline
	return pthread_mutex_timedlock(lock, &ts);
}

// safe_transfer function to safely transfer money between two accounts without deadlock
void safe_transfer(int from_id, int to_id, double amount, int teller_id) {
	while(1){ // Keep running until the transfer succeeds
		// Lock source account first
		if(pthread_mutex_lock(&accounts[from_id].lock) != 0) {
			continue; // If this lock fails then we retry
		}

		// Try to lock the destination account
		int rc = try_lock_with_timeout(&accounts[to_id].lock, 100);
		if(rc == 0){ // Successfully locked both accounts
			// Perform transfer
			accounts[from_id].balance -= amount;
			accounts[to_id].balance += amount;

			// Release locks in reverse order
			pthread_mutex_unlock(&accounts[to_id].lock);
			pthread_mutex_unlock(&accounts[from_id].lock);

			// Confirm the transfer to the user
			printf("Thread %d: safe transfer %d -> %d of %.2f completed\n",
			teller_id, from_id, to_id, amount);

			break; // Exit loop after successful transfer
		} else {
			// If we couldn't lock the destination account, then unlock the source to avoid deadlock and try again
			pthread_mutex_unlock(&accounts[from_id].lock);
			usleep(1000); // Small sleep before retrying
		}
	}
}

// Thread function where teller 1 repeatedly transfers money from Account 0 to Account 1
void* tfunc1(void* arg) {
	int teller_id = *(int*)arg;
	for(int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
		safe_transfer(0, 1, 100.0, teller_id);
	}
	return NULL;
}

// Thread function where teller 2 repeatedly transfers money from Account 1 to Account 0
void* tfunc2(void* arg) {
	int teller_id = *(int*)arg;
        for(int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
               safe_transfer(1, 0, 100.0, teller_id);
        }
        return NULL;
}

int main(void) {
	// Initialize account details
	for(int i = 0; i < NUM_ACCOUNTS; i++) {
		accounts[i].account_id = i;
		accounts[i].balance = INITIAL_BALANCE;
		accounts[i].transaction_count = 0;
		pthread_mutex_init(&accounts[i].lock, NULL);
	}

	// Assign teller IDs
	thread_ids[0] = 1;
	thread_ids[1] = 2;

	// Create threads that will do opposite transfer but deadlock
	pthread_create(&threads[0], NULL, tfunc1, &thread_ids[0]);
	pthread_create(&threads[1], NULL, tfunc2, &thread_ids[1]);

	// Wait for all threads to complete
	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	// Print final balances after all transfers
	printf("Final balances: Account 0 = %.2f & Account 1 = %.2f\n",
		accounts[0].balance, accounts[1].balance);

	// Destroy mutex to clean up resources
	for (int i = 0; i < NUM_ACCOUNTS; i++) {
		pthread_mutex_destroy(&accounts[i].lock);
	}

	return 0;
}
