#include <iostream>
#include <fstream>
#include <semaphore.h>
#include <queue>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <pthread.h>
#include <unistd.h>
using namespace std;

int N, M, X, Y;
int totalGroupNumber;
int OperationCompleted = 0;

sem_t TS[4];
pthread_mutex_t TS_queues_mutex[4];

pthread_mutex_t grp_mutex[50];
int grp_mem_completed[50];
sem_t grp_completion[100];

sem_t readwrite_mutex;
sem_t readCnt_mutex;
int readCnt=0;

pthread_mutex_t output_mutex;

auto starting_time = chrono::high_resolution_clock::now();

int get_poisson_random(double lambda)
{
    static thread_local random_device rd;
    static thread_local mt19937 gen(rd());
    poisson_distribution<int> dist(lambda);
    return dist(gen);
}
long long get_time()
{
    auto currTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(currTime - starting_time);
    return duration.count();
}
void document_recreation_typewriting_station(int op_id)
{
    int station_no = op_id % 4;

    pthread_mutex_lock(&output_mutex);
    cout << "Operative " + to_string(op_id) + " has arrived at typewriting station at time " + to_string(get_time()) << endl;
    pthread_mutex_unlock(&output_mutex);

    sem_wait(&TS[station_no]);

    usleep(X*1000);

    pthread_mutex_lock(&output_mutex);
    cout << "Operative " + to_string(op_id) + " has completed document recreation at time " + to_string(get_time()) << endl;
    pthread_mutex_unlock(&output_mutex);

    sem_post(&TS[station_no]);
}

void Record_Logbook(int leader_id)
{
    int group_number = (leader_id - 1) / M + 1;

    sem_wait(&readwrite_mutex);

    pthread_mutex_lock(&output_mutex);
    cout << "Unit " + to_string(group_number) + " has started logbook entry at time " + to_string(get_time()) << endl;
    pthread_mutex_unlock(&output_mutex);

    usleep(Y*1000);

    OperationCompleted++;

    pthread_mutex_lock(&output_mutex);
    cout << "Unit " + to_string(group_number) + " has completed intelligence distribution at time " + to_string(get_time()) << endl;
    pthread_mutex_unlock(&output_mutex);

    sem_post(&readwrite_mutex);
}

void *operativeThread(void *arg)
{
    int op_id = *(int *)arg;
    int grp_num = (op_id - 1) / M + 1;

    int delay = get_poisson_random(2.0);
    usleep(delay * 100000);

    document_recreation_typewriting_station(op_id);

    sem_post(&grp_completion[grp_num - 1]);

    bool isLeader = (op_id % M == 0);
    if (isLeader)
    {
        for (int i = 0; i < M; i++)
        {
            sem_wait(&grp_completion[grp_num - 1]);
        }
        pthread_mutex_lock(&output_mutex);
        cout << "Unit " + to_string(grp_num) + " has completed document recreation phase at time " + to_string(get_time()) << endl;
        pthread_mutex_unlock(&output_mutex);

        Record_Logbook(op_id);
    }
    return nullptr;
}

void *intelligenceStaffThread(void* arg){
    int staff_id=*(int*)arg;
    while(OperationCompleted<totalGroupNumber)
    {
        int delay=get_poisson_random(3.0+staff_id);
        usleep(delay*500000);
        sem_wait(&readCnt_mutex);

        readCnt++;
        if(readCnt==1){
            sem_wait(&readwrite_mutex);
        }
        sem_post(&readCnt_mutex);
        pthread_mutex_lock(&output_mutex);
        cout<<"Intelligence Staff " + to_string(staff_id) + " began reviewing logbook at time " + to_string(get_time()) + ". Operations completed = " + to_string(OperationCompleted)<<endl;
        pthread_mutex_unlock(&output_mutex);

        usleep(500);

        sem_wait(&readCnt_mutex);
        readCnt--;
        if (readCnt == 0) {
            sem_post(&readwrite_mutex);  
        }
        sem_post(&readCnt_mutex);

    }
    return nullptr;

}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Error\n";
        return 1;
    }

    ifstream inputFile(argv[1]);
    ofstream outputFile(argv[2]);

    streambuf *original_cout = cout.rdbuf();
    cout.rdbuf(outputFile.rdbuf());

    inputFile >> N >> M >> X >> Y;
    inputFile.close();
    totalGroupNumber = N / M;

    for (int i = 0; i < 4; i++)
    {
        sem_init(&TS[i], 0, 1);
        pthread_mutex_init(&TS_queues_mutex[i], nullptr);
    }

    for (int i = 0; i < totalGroupNumber; i++)
    {
        pthread_mutex_init(&grp_mutex[i], nullptr);
        sem_init(&grp_completion[i], 0, 0);
        grp_mem_completed[i] = 0;
    }

    sem_init(&readwrite_mutex, 0, 1);
    sem_init(&readCnt_mutex, 0, 1);

    pthread_mutex_init(&output_mutex, nullptr);

    starting_time = chrono::high_resolution_clock::now();

    vector<pthread_t> operative_threads(N);
    vector<int> operative_ids(N);

    for (int i = 0; i < N; i++)
    {
        operative_ids[i] = i + 1;
        pthread_create(&operative_threads[i], nullptr, operativeThread, &operative_ids[i]);
    }


    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};

    for (int i = 0; i < 2; i++) {
        pthread_create(&staff_threads[i], nullptr, intelligenceStaffThread, &staff_ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(operative_threads[i], nullptr);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(staff_threads[i], nullptr);
    }

    for (int i = 0; i < 4; i++) {
        sem_destroy(&TS[i]);
        pthread_mutex_destroy(&TS_queues_mutex[i]);
    }
    
    for (int i = 0; i < totalGroupNumber; i++) {
        pthread_mutex_destroy(&grp_mutex[i]);
        sem_destroy(&grp_completion[i]);
    }
    
    sem_destroy(&readwrite_mutex);
    sem_destroy(&readCnt_mutex);
    pthread_mutex_destroy(&output_mutex);

    cout.rdbuf(original_cout);
    outputFile.close();
    return 0;
}