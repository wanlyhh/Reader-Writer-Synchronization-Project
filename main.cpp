```cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>
#include <string>

using namespace std;

#define MAX_READERS 3
#define MAX_WRITERS 2
#define MAX_ITERATIONS 3

// ======================================================
// Module: Synchronization Engine
// Handles mutexes, semaphores, and shared counters
// ======================================================

// Semaphore controlling exclusive access to the shared file
sem_t file_access;

// Semaphore preventing new readers when writers are waiting
sem_t readTry;

// Mutex protecting reader_count
pthread_mutex_t r_mutex;

// Mutex protecting writer_count
pthread_mutex_t w_mutex;

// Shared counters
int reader_count = 0;
int writer_count = 0;

// Shared file name
const string target_file = "shared_resource.txt";

// ======================================================
// Module: Thread Management
// Reader thread routine
// ======================================================

void* reader_routine(void* arg)
{
    long id = (long)arg;

    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        cout << "\n[Reader " << id << "] Requesting access..." << endl;

        // ===== Module: Synchronization Engine =====
        sem_wait(&readTry);

        pthread_mutex_lock(&r_mutex);
        reader_count++;

        if (reader_count == 1)
        {
            // First reader blocks writers
            sem_wait(&file_access);
        }

        pthread_mutex_unlock(&r_mutex);

        sem_post(&readTry);

        // ===== Module: File Access Layer =====
        cout << "[Reader " << id << "] Reading shared file..." << endl;

        ifstream data_stream(target_file);

        string file_line;

        if (data_stream.is_open())
        {
            while (getline(data_stream, file_line))
            {
                cout << "    " << file_line << endl;
            }

            data_stream.close();
        }

        sleep(1);

        cout << "[Reader " << id << "] Finished reading." << endl;

        // ===== Module: Synchronization Engine =====
        pthread_mutex_lock(&r_mutex);

        reader_count--;

        if (reader_count == 0)
        {
            // Last reader allows writers
            sem_post(&file_access);
        }

        pthread_mutex_unlock(&r_mutex);

        sleep(1);
    }

    pthread_exit(nullptr);
}

// ======================================================
// Module: Thread Management
// Writer thread routine
// ======================================================

void* writer_routine(void* arg)
{
    long id = (long)arg;

    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        cout << "\n[Writer " << id << "] Requesting access..." << endl;

        // ===== Module: Synchronization Engine =====
        pthread_mutex_lock(&w_mutex);

        writer_count++;

        if (writer_count == 1)
        {
            // First waiting writer blocks new readers
            sem_wait(&readTry);
        }

        pthread_mutex_unlock(&w_mutex);

        // Exclusive access to file
        sem_wait(&file_access);

        // ===== Module: File Access Layer =====
        cout << "[Writer " << id << "] Writing to shared file..." << endl;

        ofstream data_stream(target_file, ios::app);

        if (data_stream.is_open())
        {
            data_stream << "Written securely by Writer "
                        << id
                        << " (Iteration "
                        << i + 1
                        << ")"
                        << endl;

            data_stream.close();
        }

        sleep(1);

        cout << "[Writer " << id << "] Finished writing." << endl;

        // ===== Module: Synchronization Engine =====
        sem_post(&file_access);

        pthread_mutex_lock(&w_mutex);

        writer_count--;

        if (writer_count == 0)
        {
            sem_post(&readTry);
        }

        pthread_mutex_unlock(&w_mutex);

        sleep(2);
    }

    pthread_exit(nullptr);
}

// ======================================================
// Module: Thread Management
// Main function
// ======================================================

int main()
{
    // ===== Module: File Access Layer =====
    // Initialize shared file

    ofstream init_file(target_file);

    init_file << "=== Shared Resource File ===" << endl;
    init_file << "Initial data created before thread execution." << endl;

    init_file.close();

    // ===== Module: Synchronization Engine =====
    // Initialize synchronization objects

    sem_init(&file_access, 0, 1);
    sem_init(&readTry, 0, 1);

    pthread_mutex_init(&r_mutex, nullptr);
    pthread_mutex_init(&w_mutex, nullptr);

    // ===== Module: Thread Management =====
    // Create reader and writer threads

    pthread_t readers[MAX_READERS];
    pthread_t writers[MAX_WRITERS];

    for (long i = 0; i < MAX_READERS; i++)
    {
        pthread_create(&readers[i], nullptr, reader_routine, (void*)(i + 1));
    }

    for (long i = 0; i < MAX_WRITERS; i++)
    {
        pthread_create(&writers[i], nullptr, writer_routine, (void*)(i + 1));
    }

    // Wait for all threads to finish

    for (int i = 0; i < MAX_READERS; i++)
    {
        pthread_join(readers[i], nullptr);
    }

    for (int i = 0; i < MAX_WRITERS; i++)
    {
        pthread_join(writers[i], nullptr);
    }

    // ===== Module: Synchronization Engine =====
    // Clean up resources

    sem_destroy(&file_access);
    sem_destroy(&readTry);

    pthread_mutex_destroy(&r_mutex);
    pthread_mutex_destroy(&w_mutex);

    cout << "\n======================================" << endl;
    cout << " Reader-Writer Synchronization Complete" << endl;
    cout << "======================================" << endl;

    return 0;
}
```
