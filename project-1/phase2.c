#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define NUM_ACCOUNTS 1 // Number of bank accounts
#define NUM_THREADS 3 // Number of threads
#define TRANSACTIONS_PER_TELLER 10 // Number of transactions per thread
#define INITIAL_BALANCE 1000.0 // Starting balance of each account

// Add mutex to account structure
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

// Deposit function (protected transaction)
void deposit(int account_id, double amount) {
	pthread_mutex_lock(&accounts[account_id].lock); // Lock before modifying
	double temp = accounts[account_id].balance;
        usleep(rand() % 10000); // Random short deley, but this time it won't increase any chances of race condition
        accounts[account_id].balance = temp + amount; // Add amount to balance
        accounts[account_id].transaction_count++; // Up transaction count by 1
	pthread_mutex_unlock(&accounts[account_id].lock); // Unlock after modifying
}

// Withdraw function (protected transaction)
void withdraw(int account_id, double amount) {
	pthread_mutex_lock(&accounts[account_id].lock); // Lock before modifying
        double temp = accounts[account_id].balance;
        usleep(rand() % 10000); // Random short deley, but this time it won't i>
        accounts[account_id].balance = temp - amount; // Add amount to balance
        accounts[account_id].transaction_count++; // Up transaction count by 1
        pthread_mutex_unlock(&accounts[account_id].lock); // Unlock after modifying
}

// Function executed by each teller thread
void* teller_thread(void* arg) {
	int teller_id = *(int*)arg; // Get teller ID from argument
	unsigned int seed = time(NULL) ^ teller_id; // Seed for random delay

	//Perform multiple transacations
	for(int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
		if(teller_id == 1 || teller_id == 2){ // Teller 1 and 2 always deposits
			double amount = 100.0;
			printf("Thread %d: Depositing %.2f\n", teller_id, amount); // Let user know
			deposit(0, amount); // Deposit into Account 0
		} else{ // Teller 3 always withdraws
			double amount = 50.0;
			printf("Thread %d: Withdrawing %.2f\n", teller_id, amount); //  Let user know
			withdraw(0, amount); // Withdraw from Account 0
		}
	}

	return NULL;
}

// Program entry point
int main(void) {
	// Initialize account details
	accounts[0].account_id = 0;
	accounts[0].balance = INITIAL_BALANCE;
	accounts[0].transaction_count = 0;
	pthread_mutex_init(&accounts[0].lock, NULL); // Initialize mutex

	printf("Intial balance: %.2f\n", accounts[0].balance);

	//Create teller threads
	for (int i = 0; i < NUM_THREADS; i++) {
		thread_ids[i] = i + 1; // Assign teller IDs 1, 2, 3
		if(pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]) != 0) {
               		perror("pthread_create"); // Error handling if thread creation fails
               		 exit(1);
       		 }
	}

	// Wait for all threads to complete
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	// Print final balance after all transactions
	printf("Final balance: %.2f\n", accounts[0].balance);

	// Destroy mutex to clean up resources
	pthread_mutex_destroy(&accounts[0].lock);
	return 0;
}
