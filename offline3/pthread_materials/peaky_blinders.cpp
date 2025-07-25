#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <random>
using namespace std;

// Structure for operative data
typedef struct {
    int id;
    int unit_id;
    int station_id;
    double arrival_time;
} operative_t;

// Structure for intelligence staff
typedef struct {
    int staff_id;
    double read_interval;
} staff_t;


// Global variables
int N, M, x, y; // N=operatives, M=unit size, x=doc recreation time, y=logbook time
int completed_operations = 0; // Shared counter for completed operations
int *unit_completion_count; // Track completion count for each unit
pthread_mutex_t *unit_mutex; // Mutex for each unit
pthread_mutex_t logbook_mutex; // Mutex for logbook access
pthread_mutex_t reader_count_mutex; // Mutex for reader count
sem_t *station_semaphores; // Semaphores for 4 typewriting stations
sem_t write_sem; // Semaphore for writer access to logbook
int reader_count = 0; // Number of active readers
time_t start_time; // Start time of the program


// Random number generator using Poisson distribution
int get_random_number() {
    // Creates a random device for non-deterministic random number generation
    std::random_device rd;
    // Initializes a random number generator using the random device
    std::mt19937 generator(rd());

    // Lambda value for the Poisson distribution
    double lambda = 10000.234;

    // Defines a Poisson distribution with the given lambda
    std::poisson_distribution<int> poissonDist(lambda);

    // Generates and returns a random number based on the Poisson distribution
    return poissonDist(generator);
}

// Function to generate arrival delays (scaled down from the large Poisson values)
double get_arrival_delay() {
    int poisson_val = get_random_number();
    // Scale down the large Poisson value to reasonable seconds (0-20 seconds)
    return (poisson_val % 21); // 0 to 20 seconds
}

// Function to generate read intervals for staff
double get_read_interval(int staff_id) {
    int poisson_val = get_random_number();
    if (staff_id == 1) {
        return 5 + (poisson_val % 10); // 5-14 seconds for staff 1
    } else {
        return 8 + (poisson_val % 15); // 8-22 seconds for staff 2
    }
}

// Function to get current time since start
double get_current_time() {
    return difftime(time(NULL), start_time);
}

// Operative thread function
void* operative_thread(void* arg) {
    operative_t* op = (operative_t*)arg;
    
    // Random delay before arrival using Poisson distribution
    double delay = get_arrival_delay();
    sleep((int)delay);
    
    double arrival_time = get_current_time();
    printf("Operative %d has arrived at typewriting station at time %.0f\n", 
           op->id, arrival_time);
    
    // Wait for typewriting station
    sem_wait(&station_semaphores[op->station_id]);
    
    // Document recreation phase
    sleep(x);
    double completion_time = get_current_time();
    printf("Operative %d has completed document recreation at time %.0f\n", 
           op->id, completion_time);
    
    // Signal station availability
    sem_post(&station_semaphores[op->station_id]);
    
    // Update unit completion count
    pthread_mutex_lock(&unit_mutex[op->unit_id]);
    unit_completion_count[op->unit_id]++;
    
    // Check if unit is complete
    if (unit_completion_count[op->unit_id] == M) {
        printf("Unit %d has completed document recreation phase at time %.0f\n", 
               op->unit_id + 1, get_current_time());
        
        // Leader (highest ID in unit) goes to logbook
        // Wait for write access
        sem_wait(&write_sem);
        
        // Logbook entry phase
        sleep(y);
        completed_operations++;
        printf("Unit %d has completed intelligence distribution at time %.0f\n", 
               op->unit_id + 1, get_current_time());
        
        // Release write access
        sem_post(&write_sem);
    }
    
    pthread_mutex_unlock(&unit_mutex[op->unit_id]);
    
    return NULL;
}

// Intelligence staff thread function
void* staff_thread(void* arg) {
    staff_t* staff = (staff_t*)arg;
    
    while (1) {
        // Random delay between reads using Poisson distribution
        double delay = get_read_interval(staff->staff_id);
        sleep((int)delay);
        
        // Reader entry
        pthread_mutex_lock(&reader_count_mutex);
        reader_count++;
        if (reader_count == 1) {
            sem_wait(&write_sem); // First reader blocks writers
        }
        pthread_mutex_unlock(&reader_count_mutex);
        
        // Reading logbook
        double read_time = get_current_time();
        printf("Intelligence Staff %d began reviewing logbook at time %.0f. Operations completed = %d\n", 
               staff->staff_id, read_time, completed_operations);
        
        // Simulate reading time
        sleep(1);
        
        // Reader exit
        pthread_mutex_lock(&reader_count_mutex);
        reader_count--;
        if (reader_count == 0) {
            sem_post(&write_sem); // Last reader allows writers
        }
        pthread_mutex_unlock(&reader_count_mutex);
    }
    
    return NULL;
}

int main() {
    // Initialize start time
    start_time = time(NULL);
    
    // Read input from file
    FILE* input_file = fopen("./2105110/input.txt", "r");
    if (!input_file) {
        printf("Error: Cannot open input file\n");
        return 1;
    }
    
    fscanf(input_file, "%d %d", &N, &M);
    fscanf(input_file, "%d %d", &x, &y);
    fclose(input_file);
    
    // Validate input
    if (N % M != 0) {
        printf("Error: N must be divisible by M\n");
        return 1;
    }
    
    
    int c = N / M;  //number of units
    
    // Initialize synchronization primitives
    station_semaphores = (sem_t*)malloc(4 * sizeof(sem_t));
    unit_mutex = (pthread_mutex_t*)malloc(c * sizeof(pthread_mutex_t));
    unit_completion_count = (int*)calloc(c, sizeof(int));
    
    // Initialize semaphores and mutexes
    for (int i = 0; i < 4; i++) {
        sem_init(&station_semaphores[i], 0, 1); // Each station can handle 1 operative
    }
    
    for (int i = 0; i < c; i++) {
        pthread_mutex_init(&unit_mutex[i], NULL);
    }
    
    pthread_mutex_init(&logbook_mutex, NULL);
    pthread_mutex_init(&reader_count_mutex, NULL);
    sem_init(&write_sem, 0, 1);
    
    // Create operative threads
    pthread_t* operative_threads = (pthread_t*)malloc(N * sizeof(pthread_t));
    operative_t* operatives = (operative_t*)malloc(N * sizeof(operative_t));
    
    for (int i = 0; i < N; i++) {
        operatives[i].id = i + 1;
        operatives[i].unit_id = i / M;
        operatives[i].station_id = (i % 4); // Station assignment: (ID mod 4)
        
        pthread_create(&operative_threads[i], NULL, operative_thread, &operatives[i]);
    }
    
    // Create intelligence staff threads
    pthread_t staff_threads[2];
    staff_t staff[2];
    
    staff[0].staff_id = 1;
    staff[0].read_interval = 8.0; // Base interval, will be modified by Poisson
    staff[1].staff_id = 2;
    staff[1].read_interval = 12.0; // Base interval, will be modified by Poisson
    
    pthread_create(&staff_threads[0], NULL, staff_thread, &staff[0]);
    pthread_create(&staff_threads[1], NULL, staff_thread, &staff[1]);
    
    // wait for all operative threads to complete
    for (int i = 0; i < N; i++) {
        pthread_join(operative_threads[i], NULL);
    }
    
    // give some time for final staff readings
    sleep(5);

    // cancel staff threads (they run indefinitely)
    pthread_cancel(staff_threads[0]);
    pthread_cancel(staff_threads[1]);
    
    // Cleanup
    for (int i = 0; i < 4; i++) {
        sem_destroy(&station_semaphores[i]);
    }
    
    for (int i = 0; i < c; i++) {
        pthread_mutex_destroy(&unit_mutex[i]);
    }
    
    pthread_mutex_destroy(&logbook_mutex);
    pthread_mutex_destroy(&reader_count_mutex);
    sem_destroy(&write_sem);
    
    free(station_semaphores);
    free(unit_mutex);
    free(unit_completion_count);
    free(operative_threads);
    free(operatives);
    
    printf("All operations completed successfully!\n");
    
    return 0;
}