CS 3502
Project 1: Multi-Threaded Banking System
Chernoh Bah

--------------------------------------------------------------------------------------------
Overview
--------------------------------------------------------------------------------------------

This project demonstrates multi-threaded programming concepts using a banking system example. 
The implementation is divided into four phases, each building on the previous one to illustrate 
the following concepts: race conditions, mutex synchronization, deadlock creation, and deadlock 
resolution.

--------------------------------------------------------------------------------------------
Phase 1: Basic Thread Operations
--------------------------------------------------------------------------------------------

Created multiple teller threads that access and update a shared account balance without 
synchronization. Each thread performs several deposits or withdrawals, resulting in 
inconsistent final balances due to race conditions. This phase demonstrates the need 
for mutual exclusion.

**Compilation:**
gcc -Wall -pthread phase1.c -o phase1

**Execution:**
./phase1

--------------------------------------------------------------------------------------------
Phase 2: Resource Protection
--------------------------------------------------------------------------------------------

Added a `pthread_mutex_t` lock to each account to ensure only one thread can modify it 
at a time. The program now produces consistent final balances, proving that synchronization 
eliminates race conditions.

**Compilation:**
gcc -Wall -pthread phase2.c -o phase2

**Execution:**
./phase2

--------------------------------------------------------------------------------------------
Phase 3: Deadlock Creation
--------------------------------------------------------------------------------------------

Introduced a transfer function that locks two accounts per transaction. Two threads performing 
opposite transfers can each hold one lock and wait for the other, producing a circular wait 
and a deadlock. The program stops making progress once both threads are stuck.

**Compilation:**
gcc -Wall -pthread phase3.c -o phase3

**Execution:**
./phase3

--------------------------------------------------------------------------------------------
Phase 4: Deadlock Resolution
--------------------------------------------------------------------------------------------

Modified the transfer logic to prevent deadlock using a timed lock mechanism. Each thread 
uses `pthread_mutex_timedlock` to attempt locking the second account. If it fails, it releases 
the first lock and retries later. This avoids circular waiting and allows all transfers to 
complete successfully without deadlock.

**Compilation:**
gcc -Wall -pthread phase4.c -o phase4

**Execution:**
./phase4

--------------------------------------------------------------------------------------------
