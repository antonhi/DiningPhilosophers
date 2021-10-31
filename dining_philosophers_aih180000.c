#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <semaphore.h>

/********************************************************************************************************************************************************
* File Name: dining_philosophers_aih180000.c
* Author: Anton Horvath
* Modification History:
    > 10/13/2021 Added template pthread creation
    > 10/13/2021 Added content from example program
    > 10/13/2021 Added philosopher thread functionality using semaphores
    > 10/13/2021 Added sleep method
    > 10/13/2021 Added Random Number Generator method
    > 10/13/2021 Added room semaphore
    > 10/14/2021 Added stat information for threads
    > 10/14/2021 Added output for thread stats after thread join
* Procedures:
* main                - allocates memory for all semaphores before initializing all semaphores. Allocates memory for both timer variables
                        and captures an intial time for each. Allocates memory for five thread info structs and assigns information to each
                        struct before each thread is started. Creates 5 pthreads before waiting for 5 minutes. Joins each thread created
                        and outputs stats about thread execution.
* fiveMinuteDelay     - Loops for five minutes before returning
* getRandomInterval   - obtains a long between 25 and 49 and returns the value
* initializeForks     - intializes all five semaphore given the address of the first semaphore
* msleep              - Puts the thread to sleep for specific amount of time using nanosleep()
* philosopher         - thread function that continues to sleep two times (representing eating and philosophizing) for five minutes before exiting
********************************************************************************************************************************************************/

void fiveMinuteDelay();
static long getRandomInterval();
void initializeForks(sem_t *forks);
int msleep(long msec);
static void * philosopher(void *arg);

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)           // Error handler for pthread creation

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)                       // Error handler for standard error detection

struct thread_info {                                                            // Used as argument to thread_start(), all relevant thread information
    pthread_t thread_id;                                                        // ID returned by pthread_create()
    int       thread_num;                                                       // Application-defined thread #
    sem_t *   forks;                                                            // Semaphore array, will contain all five "forks"
    clock_t * start_t;                                                          // Start time address for timer
    clock_t * end_t;                                                            // End time address for timer
    int *     stats;                                                            // integer array where all stats for a given thread will be kept
};

/********************************************************************************************************************************************************
* int main (int argc, char *argv[])
* Author: Die.net
* Modifier: Anton Horvath
* Date: 13 October 2021
* Description:  allocates memory for all semaphores before initializing all semaphores. Allocates memory for both timer variables
                and captures an intial time for each using clock(). Allocates memory for five thread info structs and assigns information to each
                struct before each thread is started. Creates 5 pthreads using thread information structs before waiting for 5 minutes. Joins 
                each thread created and outputs stats about thread execution including the amount of actions and average time of actions.

* Parameters:
*    argc - int - number of arguments sent from the command line
*    argv - char*[] - arguments sent from the command line
********************************************************************************************************************************************************/

int main(int argc, char *argv[]) {
    int s, tnum;                                                                // s is our error detection integer, tnum is an index integer for arrays
    sem_t *forks;                                                               // address for the array of semaphores
    struct thread_info *tinfo;                                                  // address for the array of thread infos
    void *res;                                                                  // address for the return value of each thread
    clock_t *start_t, *end_t;                                                   // adresses for our start and end times, used by all threads to detect end

    forks = calloc(6, sizeof(sem_t));                                           // allocate memory for 6 semaphores

    if (forks == NULL)                                                          // check if forks is NULL
        handle_error("calloc");                                                 // use defined handler error function, passing in allocation as reason

    start_t = malloc(sizeof(clock_t));                                          // allocate memory for our start time
    end_t = malloc(sizeof(clock_t));                                            // allocate memory for our end time

    *start_t = clock();                                                         // initialize start time to be current time at moment of clock() call
    *end_t = clock();                                                           // initialize end time to be current time at moment of clock() call

    tinfo = calloc(5, sizeof(struct thread_info));                              // allocate memory for 5 thread info structs

    if (tinfo == NULL)                                                          // check if tinfo is NULL
        handle_error("calloc");                                                 // use defined handler error function, passing in allocation as reason

    initializeForks(forks);                                                     // initialize semaphores, passing in the address of the first semaphore

   for (tnum = 0; tnum < 5; tnum++) {                                           // use 5 iterations to create 5 philosopher threads
        tinfo[tnum].thread_num = tnum + 1;                                      // assign thread number to the thread information for thread at index
        tinfo[tnum].forks = forks;                                              // assign address of semaphores to thread information for thread at index
        tinfo[tnum].start_t = start_t;                                          // assign address of start time to thread information for thread at index
        tinfo[tnum].end_t = end_t;                                              // assign address of end time to thread information for thread at index

       s = pthread_create(&tinfo[tnum].thread_id, NULL,                         // create thread, capturing thread id inside thread information. Use philosopher
                           &philosopher, &tinfo[tnum]);                         // function as thread function. No attribute is needed
        if (s != 0)                                                             // failure to create thread, handle error using defined method
            handle_error_en(s, "pthread_create");                               // handler error with reason of thread creation
    }

    fiveMinuteDelay(start_t, end_t);                                            // call function that waits for 5 minutes
    
   for (tnum = 0; tnum < 5; tnum++) {                                           // join each thread that was created, iterating 5 times for 5 philosophers
        s = pthread_join(tinfo[tnum].thread_id, NULL);                          // join thread with the given thread ID
        if (s != 0)                                                             // check if thread join action was successful
            handle_error_en(s, "pthread_join");                                 // handle error with pthread join reason

        printf("Thread %d Information\n\n", tinfo[tnum].thread_num);            // Output thread information header
        printf("Number of Philosophize Actions: %i\n", tinfo[tnum].stats[0]);   // Output number of philosophize actions for joined thread execution
        printf("Average Philosophize Time: %i\n", tinfo[tnum].stats[2]);        // Output average philosophize time for joined thread execution
        printf("Number of Eat Actions: %i\n", tinfo[tnum].stats[1]);            // Ouput number of eat actions for joined thread execution
        printf("Average Eat Time: %i\n\n", tinfo[tnum].stats[3]);               // Output average eat time for joined thread execution
    }

    free(tinfo);                                                                // free memory allocated for all thread information
    free(forks);                                                                // free memory allocated for semaphores
    free(start_t);                                                              // free memory allocated for start time
    free(end_t);                                                                // free memory allocated for end time
    exit(EXIT_SUCCESS);                                                         // exit main successfully
}

/********************************************************************************************************************************************************
* void fiveMinuteDelay (clock_t *start_t, clock_t *end_t)
* Author: Anton Horvath
* Date: 13 October 2021
* Description:  delay function that remains in a while loop until the specified time in seconds is met, in which case the method ends

* Parameters:
*    * start_t - clock_t - start time address, which was also passed to all threads
*    * end_t - clock_t - end time address, which was also passed to all threads
********************************************************************************************************************************************************/

void fiveMinuteDelay(clock_t *start_t, clock_t *end_t) {
    while ((double)(*end_t-*start_t)/CLOCKS_PER_SEC < 300) {                     // we continue looping in the while loop until 
        *end_t = clock();                                                       // specified number of seconds is reached
    }                                                                           // end_t is continuously updated to be current time
}

/********************************************************************************************************************************************************
* void initializeForks (sem_t *forks)
* Author: Anton Horvath
* Date: 13 October 2021
* Description:  initializes all sempaphores that are contained within our 5-semaphore array

* Parameters:
*    * forks - sem_t - first semaphore address in our 5-semaphore array, representing forks
********************************************************************************************************************************************************/

void initializeForks(sem_t *forks) {
    for (int i = 0; i < 6; i++) {                                               // iterate 6 times for 6 semaphores
        if (i == 5) sem_init(&forks[i], 0, 4);                                  // last semaphore is our room semaphore, should be set to allow for 4
        else sem_init(&forks[i], 0, 1);                                         // initialize all 5 philosopher semaphores with value 1
    }
}

/********************************************************************************************************************************************************
* void msleep (long msec)
* Author: StackOverflow - Neuron - https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
* Modified By: Anton Horvath
* Date: 13 October 2021
* Description:  calls nanosleep function for specified number of milliseconds

* Parameters:
*    * msec - long - number of milliseconds requested to sleep for
********************************************************************************************************************************************************/

int msleep(long msec)
{
    struct timespec *ts = malloc(sizeof(struct timespec));                      // create timespec struct at address, used for nanosleep
    int res;                                                                    // res represents return value from nanosleep function

    if (msec < 0) { return -1; }                                                // if the long number provided is less than 0, not valid                    

    ts->tv_sec = msec / 1000;                                                   // regular seconds calculated by dividing milliseconds by 1000
    ts->tv_nsec = (msec % 1000) * 1000000;                                      // nanoseconds calculated from milliseconds

    res = nanosleep(ts, ts);                                                    // use the nanosleep function to sleep for specified period of time

    return res;                                                                 // res indicates the resultant of nanosleep, success or not
}

/********************************************************************************************************************************************************
* void getRandomInterval ()
* Author: Anton Horvath
* Date: 13 October 2021
* Description:  returns a random long value between 25 and 49

* Parameters:
*    none
********************************************************************************************************************************************************/

static long getRandomInterval() {
    return (long)((rand() % (49-25+1)) + 25);                                   // returns random value between 25-49, casted to long
}

/********************************************************************************************************************************************************
* static void * philosopher(void *arg)
* Author: Die.net
* Modifier: Anton Horvath
* Date: 13 October 2021
* Description:  Thread function that continues looping, sleeping once representing "philosophizing" and sleeping again for "eating".
                The thread will stop looping when the clock values represent an interbal of 5 minutes has passed. Stores all statistics
                into thread info struct, shared in memory with the main thread
* Parameters:
*    * info - void - void pointer that can represent any address, in this case treated as an address for a thread info struct, information
                     thread needs to run
********************************************************************************************************************************************************/

static void * philosopher(void *info) {

    struct thread_info *tinfo = info;                                           // address of the thread info for the thread
    long sleepTime;                                                             // initialize a long variable used for capturing random sleep times
    tinfo->stats = malloc(4);                                                   // allocate memory for stats, which will hold all number stat values
    int amtPhilosophize, amtEat, avgPhilosophizeTime, avgEatTime = 0;           // set all stat values to 0

    clock_t *start_t = tinfo->start_t;                                          // assign address of start time to a variable
    clock_t *end_t = tinfo->end_t;                                              // assign address of end time to a variable

    int num = tinfo->thread_num;                                                // assign thread number to a variable

    sem_t fork1 = tinfo->forks[num-1];                                          // assign left fork (semaphore) to a variable
    sem_t fork2 = tinfo->forks[num%5];                                          // assign right fork (semaphore) to a variable
    sem_t room = tinfo->forks[5];                                               // assign room semaphore to a variable

    while ((double)(*end_t-*start_t)/CLOCKS_PER_SEC < 300) {                     // continue looping until 5 minutes passes
        sleepTime = getRandomInterval();                                        // get random interval for philosophizing, save to variable
        avgPhilosophizeTime = (((int)sleepTime) + avgPhilosophizeTime)/2;       // calculate running average philosophizing time for stat information
        msleep(sleepTime);                                                      // sleep for specified interval in milliseconds (philosophizing)
        amtPhilosophize++;                                                      // increment stat of philosophize amount

        sem_wait(&room);                                                        // thread enters theoretical "room", only 4 allowed at once
        sem_wait(&fork1);                                                       // thread picks up left fork represented as semaphore
        sem_wait(&fork2);                                                       // thread picks up right fork represented as semaphore

        sleepTime = getRandomInterval();                                        // get random interval for eating, save to variable
        avgEatTime = (((int)sleepTime) + avgEatTime)/2;                         // calculate running average for eat time
        msleep(sleepTime);                                                      // sleep for specified interbal in milliseconds (eating)
        amtEat++;                                                               // increment number of times philosopher has eaten (thread sleeps)

        sem_post(&fork2);                                                       // thread (or philosopher) puts down right fork (releases semaphore)
        sem_post(&fork1);                                                       // thread (or philosopher) puts down left fork (releases semaphore)
        sem_post(&room);                                                        // release room semaphore, thread has left room
    }

    tinfo->stats[0] = amtPhilosophize;                                          // save philosophizing action amount to stat address
    tinfo->stats[1] = amtEat;                                                   // save eating action amount to stat address
    tinfo->stats[2] = avgPhilosophizeTime;                                      // save philosophizing average time to stat address
    tinfo->stats[3] = avgEatTime;                                               // save eating average time to stat address

   return info;                                                                 // return the info of the thread, where stats is saved
}