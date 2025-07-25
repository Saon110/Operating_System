#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <random>

#define NUM_STATIONS 4
#define MAX_OPERATIVES 100

int N, M, x, y;
int completed_operations = 0;
int operatives_completed[MAX_OPERATIVES] = {0};
int unit_completion_count[MAX_OPERATIVES] = {0};

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unit_mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t stations[NUM_STATIONS];
sem_t write_sem;
pthread_mutex_t read_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int read_count = 0;

struct timespec start_time;

typedef struct
{
    int id;
    int unit_id;
    int is_leader;
    int station_id;
    int arrival_time;
} Operative;

void init_timing()
{
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}
long long get_time_ms()
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    long long elapsed = (current_time.tv_sec - start_time.tv_sec) * 1000LL +
                        (current_time.tv_nsec - start_time.tv_nsec) / 1000000LL;
    return elapsed;
}

int generate_poisson(double lambda)
{
    std::random_device rd;
    std::mt19937 generator(rd());

    std::poisson_distribution<int> poissonDist(lambda);
    return poissonDist(generator);
}

void init_sync()
{
    for (int i = 0; i < NUM_STATIONS; i++)
    {
        sem_init(&stations[i], 0, 1);
    }

    sem_init(&write_sem, 0, 1);
}

void cleanup_sync()
{
    for (int i = 0; i < NUM_STATIONS; i++)
    {
        sem_destroy(&stations[i]);
    }
    sem_destroy(&write_sem);
}

int check_unit_completion(int unit_id)
{
    int start_operative = unit_id * M + 1;
    int end_operative = (unit_id + 1) * M;

    for (int i = start_operative; i <= end_operative; i++)
    {
        if (!operatives_completed[i - 1])
        {
            return 0;
        }
    }
    return 1;
}

void *staff_reader_thread(void *arg)
{
    int staff_id = *(int *)arg;
    double read_interval = staff_id == 1 ? 2.8 : 3.5;

    while (1)
    {
        int delay = generate_poisson(read_interval) + 1;
        usleep(delay * 1000000);

        pthread_mutex_lock(&read_count_mutex);
        read_count++;
        if (read_count == 1)
        {
            sem_wait(&write_sem);
        }
        pthread_mutex_unlock(&read_count_mutex);
        
        pthread_mutex_lock(&output_mutex);
        printf("Intelligence Staff %d began reviewing logbook at time %lld. Operations completed = %d\n",
                   staff_id, get_time_ms() / 1000, completed_operations);
        fflush(stdout);
        pthread_mutex_unlock(&output_mutex);

        usleep((generate_poisson(1.5) + 1) * 1000000);

        pthread_mutex_lock(&read_count_mutex);
        read_count--;
        if (read_count == 0)
        {
            sem_post(&write_sem);
        }
        pthread_mutex_unlock(&read_count_mutex);
        if (completed_operations >= N / M)
        {
            break;
        }
    }
    return NULL;
}

void *operative_thread(void *arg)
{
    Operative *op = (Operative *)arg;

    int arrival_delay = generate_poisson(2.0) + 1;
    usleep(arrival_delay * 1000000);
    pthread_mutex_lock(&output_mutex);
    printf("Operative %d has arrived at typewriting station at time %lld\n",
               op->id, get_time_ms() / 1000);
    fflush(stdout);
    pthread_mutex_unlock(&output_mutex);

    // Blocking wait for station
    sem_wait(&stations[op->station_id]);

    pthread_mutex_lock(&output_mutex);
    printf("Operative %d started document recreation at station TS%d at time %lld\n",
               op->id, op->station_id + 1, get_time_ms() / 1000);
    fflush(stdout);
    pthread_mutex_unlock(&output_mutex);

    usleep(x * 100000);

    pthread_mutex_lock(&output_mutex);
    printf("Operative %d has completed document recreation at time %lld\n",
               op->id, get_time_ms() / 1000);
    fflush(stdout);
    pthread_mutex_unlock(&output_mutex);

    pthread_mutex_lock(&unit_mutex);
    operatives_completed[op->id - 1] = 1;
    // Check if this operative is the last in their unit
    if (check_unit_completion(op->unit_id)) {
        unit_completion_count[op->unit_id] = 1;
    }
    pthread_mutex_unlock(&unit_mutex);

    sem_post(&stations[op->station_id]);

    if (op->is_leader)
    {
        // Blocking wait using mutex lock/unlock
        while (1) {
            pthread_mutex_lock(&unit_mutex);
            if (unit_completion_count[op->unit_id]) {
                pthread_mutex_unlock(&unit_mutex);
                break;
            }
            pthread_mutex_unlock(&unit_mutex);
            usleep(10000);  // Minimal sleep to avoid pure busy-wait
        }

        pthread_mutex_lock(&output_mutex);
        printf("Unit %d has completed document recreation phase at time %lld\n",
                   op->unit_id + 1, get_time_ms() / 1000);
        fflush(stdout);
        pthread_mutex_unlock(&output_mutex);

        sem_wait(&write_sem);

        pthread_mutex_lock(&output_mutex);
        printf("Unit %d leader (Operative %d) accessing logbook at time %lld\n",
                   op->unit_id + 1, op->id, get_time_ms() / 1000);
        fflush(stdout);
        pthread_mutex_unlock(&output_mutex);

        usleep(y * 1000000);
        completed_operations++;

        pthread_mutex_lock(&output_mutex);
        printf("Unit %d has completed intelligence distribution at time %lld\n",
                   op->unit_id + 1, get_time_ms() / 1000);
        fflush(stdout);
        pthread_mutex_unlock(&output_mutex);

        sem_post(&write_sem);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    FILE *output_file = fopen(argv[2], "w");

    if (!input_file || !output_file)
    {
        printf("Error opening files\n");
        return 1;
    }
    fscanf(input_file, "%d %d", &N, &M);
    fscanf(input_file, "%d %d", &x, &y);
    fclose(input_file);

    dup2(fileno(output_file), STDOUT_FILENO);
    srand(time(NULL));
    init_timing();
    init_sync();

    Operative operatives[N];
    pthread_t operative_threads[N];

    for (int i = 0; i < N; i++)
    {
        operatives[i].id = i + 1;
        operatives[i].unit_id = i / M;
        operatives[i].is_leader = ((i + 1) % M == 0) ? 1 : 0;
        operatives[i].station_id = (i + 1) % 4;
        if (operatives[i].station_id == 0)
            operatives[i].station_id = 4;
        operatives[i].station_id--;
    }

    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};

    for (int i = 0; i < 2; i++)
    {
        pthread_create(&staff_threads[i], NULL, staff_reader_thread, &staff_ids[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_create(&operative_threads[i], NULL, operative_thread, &operatives[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(operative_threads[i], NULL);
    }

    sleep(2);

    for (int i = 0; i < 2; i++)
    {
        pthread_cancel(staff_threads[i]);
    }

    cleanup_sync();
    fclose(output_file);

    return 0;
}