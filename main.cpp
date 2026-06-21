#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>
#include <string>

#define MAX_READERS 3
#define MAX_WRITERS 2

using namespace std;

sem_t file_access;   
sem_t readTry;       
pthread_mutex_t r_mutex; 
pthread_mutex_t w_mutex; 

int reader_count = 0;
int writer_count = 0; 
const string target_file = "shared_resource.txt";

void* reader_routine(void* arg) {
    long id = (long)arg;
    
    while (true) {
        cout << "[Reader " << id << "] Status: Requesting access..." << endl;

        sem_wait(&readTry);              
        pthread_mutex_lock(&r_mutex);
        
        reader_count++;
        if (reader_count == 1) {
            sem_wait(&file_access);      
        }
        
        pthread_mutex_unlock(&r_mutex);
        sem_post(&readTry);              

        cout << " --> [Reader " << id << "] is actively reading shared data..." << endl;
        ifstream data_stream(target_file);
        string file_line;
        if (data_stream.is_open()) {
            while (getline(data_stream, file_line)) {
            }
            data_stream.close();
        }
        sleep(2); 
        cout << " <-- [Reader " << id << "] finished reading session." << endl;

        pthread_mutex_lock(&r_mutex);
        
        reader_count--;
        if (reader_count == 0) {
            sem_post(&file_access);      
        }
        
        pthread_mutex_unlock(&r_mutex);

        sleep(3); 
    }
    return nullptr;
}

void* writer_routine(void* arg) {
    long id = (long)arg;
    
    while (true) {
        cout << "[Writer " << id << "] Status: Requesting access..." << endl;

        pthread_mutex_lock(&w_mutex);
        
        writer_count++;
        if (writer_count == 1) {
            sem_wait(&readTry);          
        }
        
        pthread_mutex_unlock(&w_mutex);
        sem_wait(&file_access);          

        cout << " ==> [Writer " << id << "] STARTING WRITE BLOCK..." << endl;
        ofstream data_stream(target_file, ios::app); 
        if (data_stream.is_open()) {
            data_stream << "Data log updated securely by Writer " << id << ".\n";
            data_stream.close();
        }
        sleep(2); 
        cout << " <== [Writer " << id << "] COMPLETED WRITE BLOCK." << endl;

        sem_post(&file_access);          
        
        pthread_mutex_lock(&w_mutex);
        
        writer_count--;
        if (writer_count == 0) {
            sem_post(&readTry);          
        }
        
        pthread_mutex_unlock(&w_mutex);

        sleep(4); 
    }
    return nullptr;
}

int main() {
    sem_init(&file_access, 0, 1);
    sem_init(&readTry, 0, 1);
    pthread_mutex_init(&r_mutex, nullptr);
    pthread_mutex_init(&w_mutex, nullptr);

    pthread_t r_workers[MAX_READERS];
    pthread_t w_workers[MAX_WRITERS];

    for (long i = 0; i < MAX_READERS; ++i) {
        pthread_create(&r_workers[i], nullptr, reader_routine, (void*)(i + 1));
    }
    for (long i = 0; i < MAX_WRITERS; ++i) {
        pthread_create(&w_workers[i], nullptr, writer_routine, (void*)(i + 1));
    }

    for (int i = 0; i < MAX_READERS; ++i) {
        pthread_join(r_workers[i], nullptr);
    }
    for (int i = 0; i < MAX_WRITERS; ++i) {
        pthread_join(w_workers[i], nullptr);
    }

    return 0;
}
