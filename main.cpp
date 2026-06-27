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

// Handles mutexes, semaphores, and shared counters
sem_t resource_access;
sem_t read_try;
pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;

int read_count = 0;
int write_count = 0;

// Shared file name
const string target_file = "shared_resource.txt";

// Reader thread function
void* reader_routine(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < MAX_ITERATIONS; i++){
        cout << "\n[Reader " << id << "] Requesting access..." << endl;
        
        sem_wait(&read_try);
        pthread_mutex_lock(&read_mutex);
        read_count++;

        if (read_count == 1)
            sem_wait(&resource_access);
    
        pthread_mutex_unlock(&read_mutex);
        sem_post(&read_try);

        cout << "[Reader " << id << "] Reading shared file..." << endl;
        ifstream data_stream(target_file);
        string file_line;
        if (data_stream.is_open()){
            while (getline(data_stream, file_line))
                cout << "    " << file_line << endl;
            data_stream.close();
        }

        sleep(1);
        cout << "[Reader " << id << "] Finished reading." << endl;

        pthread_mutex_lock(&read_mutex);
        read_count--;
        if (read_count == 0)
            sem_post(&resource_access);

        pthread_mutex_unlock(&read_mutex);
        sleep(1);
    }
    pthread_exit(nullptr);
}

// Writer thread function
void* writer_routine(void* arg){
    long id = (long)arg;
    for (int i = 0; i < MAX_ITERATIONS; i++){
        cout << "\n[Writer " << id << "] Requesting access..." << endl;
        pthread_mutex_lock(&write_mutex);

        write_count++;
        if (write_count == 1)
            sem_wait(&read_try);

        pthread_mutex_unlock(&write_mutex);
        sem_wait(&resource_access);

        cout << "[Writer " << id << "] Writing to shared file..." << endl;

        ofstream data_stream(target_file, ios::app);
        if (data_stream.is_open()){
            data_stream << "Written securely by Writer "<< id << " (Iteration " << i + 1 << ")" << endl;
            data_stream.close();
        }

        sleep(1);
        cout << "[Writer " << id << "] Finished writing." << endl;

        sem_post(&resource_access);
        pthread_mutex_lock(&write_mutex);
        write_count--;
        if (write_count == 0)
            sem_post(&read_try);
        
        pthread_mutex_unlock(&write_mutex);
        sleep(2);
    }
    pthread_exit(nullptr);
}

// Main function
int main()
{
    ofstream init_file(target_file);

    init_file << "=== Shared Resource File ===" << endl;
    init_file << "Initial data created before thread execution." << endl;

    init_file.close();

    // Initialize synchronization objects
    sem_init(&resource_access, 0, 1);
    sem_init(&read_try, 0, 1);

    pthread_mutex_init(&read_mutex, nullptr);
    pthread_mutex_init(&write_mutex, nullptr);

    // Create reader and writer threads
    pthread_t readers[MAX_READERS];
    pthread_t writers[MAX_WRITERS];

    for (long i = 0; i < MAX_READERS; i++)
        pthread_create(&readers[i], nullptr, reader_routine, (void*)(i + 1));
    
    for (long i = 0; i < MAX_WRITERS; i++)
        pthread_create(&writers[i], nullptr, writer_routine, (void*)(i + 1));

    // Wait for all threads to finish
    for (int i = 0; i < MAX_READERS; i++)
        pthread_join(readers[i], nullptr);

    for (int i = 0; i < MAX_WRITERS; i++)
        pthread_join(writers[i], nullptr);

    // Clean up resources
    sem_destroy(&resource_access);
    sem_destroy(&read_try);

    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);

    cout << "\n======================================" << endl;
    cout << " Reader-Writer Synchronization Complete" << endl;
    cout << "======================================" << endl;

    return 0;
}
