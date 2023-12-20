/*
----------------------------------------------------------
Program: test2.c
Date: October 24, 2023
Student Name & NetID: Kennedy Keyes kfk38
Description: This program simulates a guessing game between
a referee and two players. Each player uses a different strategy
to make their guess. Each player's step 2 is different. Player 1
always choose the average of the lower and upper bounds as the
next guess. Player 2 always chooses  random number between the upper
and lower bounds as the next guess. This program will play the game
10 times. This is used by threads.
----------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Problem 1 */
#include <pthread.h>
#include <time.h>
#include <errno.h>

/* Problem 2 */
static int guess[2] = {0, 0};
static int dirs[2] = {0, 0};
static int sgn[4] = {0, 0, 0, 0};
static int target;

/* Problem 3 */
pthread_mutex_t mutex[3];
pthread_cond_t cond[3];

void initMutexAndCond(void) 
{
    for (int i = 0; i < 3; i++) 
    {
        pthread_mutex_init(&mutex[i], NULL);
        pthread_cond_init(&cond[i], NULL);
    }
}

int checkThread(int val, const char *msg)
{
    if (val != 0) 
    {
        errno = val;
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int rngRand(int first, int last) 
{
    int rng = (last - first) + 1;
    double perc = ((double) rand()) / (((double) RAND_MAX) + 1);
    int offst = rng * perc;
    return first + offst;
}

void *player1(void *args) 
{
    /* Problem 4 */
    while (1) 
    {
        int min = 0;
        int max = 100;

        // wait for referee's signal to start the game
        pthread_mutex_lock(&mutex[2]);
        while (sgn[2] == 0)
            pthread_cond_wait(&cond[2], &mutex[2]);
        
        sgn[2] = 0;
        pthread_mutex_unlock(&mutex[2]);

        while (1) 
        {   // calculate the guess as the average of the current bounds
            int guess1 = (min + max) / 2;

            // inform the referee about the guess
            pthread_mutex_lock(&mutex[0]);
            guess[0] = guess1;
            pthread_cond_signal(&cond[0]);

            while (sgn[0] != 1)
                pthread_cond_wait(&cond[0], &mutex[0]);

            int dir = dirs[0];

            if (dir == -1) 
            {
                min = guess1;
            } else if (dir == 1) 
            {
                max = guess1;
            } else 
            {
                break;
            }
            pthread_mutex_unlock(&mutex[0]);
        }
    }
}

void *player2(void *args) 
{
    /* Problem 5 */
    while (1) 
    {
        int min = 0;
        int max = 100;

        pthread_mutex_lock(&mutex[2]);
        while (sgn[3] == 0)
            pthread_cond_wait(&cond[2], &mutex[2]);

        sgn[3] = 0;
        pthread_mutex_unlock(&mutex[2]);

        while (1) 
        {
            int guess2 = rngRand(min, max);

            pthread_mutex_lock(&mutex[1]);

            guess[1] = guess2;
            pthread_cond_signal(&cond[1]);

            while (sgn[1] != 1)
                pthread_cond_wait(&cond[1], &mutex[1]);

            int dir = dirs[1];

            if (dir == -1) 
            {
                min = guess2;
            } else if (dir == 1) 
            {
                max = guess2;
            } else 
            {
                break;
            }
            pthread_mutex_unlock(&mutex[1]);
        }
    }
}

void *referee(void *args) 
{
    /* Problem 6 */
    for (int game = 0; game < 10; game++) 
    {
        // choose a random target number
        target = rngRand(1, 100);

        // signal players to start
        pthread_mutex_lock(&mutex[2]);
        sgn[2] = 1;
        sgn[3] = 1;
        pthread_cond_broadcast(&cond[2]);
        pthread_mutex_unlock(&mutex[2]);

        printf("Game %d: Current Scores\n", game);
        printf("Player 1: 0\n");
        printf("Player 2: 0\n");

        for (int turn = 1; ; turn++) 
        {
            sleep(1);

            pthread_mutex_lock(&mutex[0]);
            sgn[0] = 0;
            dirs[0] = 0;
            dirs[1] = 0;
            pthread_mutex_lock(&mutex[1]);
            sgn[1] = 0;
            pthread_cond_broadcast(&cond[0]);
            pthread_cond_broadcast(&cond[1]);
            pthread_mutex_unlock(&mutex[1]);
            pthread_mutex_unlock(&mutex[0]);

            printf("Turn %d: Referee's number is %d\n", turn, target);

            pthread_mutex_lock(&mutex[0]);
            while (sgn[0] == 0)
                pthread_cond_wait(&cond[0], &mutex[0]);

            int dir1 = dirs[0];

            pthread_mutex_lock(&mutex[1]);
            while (sgn[1] == 0)
                pthread_cond_wait(&cond[1], &mutex[1]);

            int dir2 = dirs[1];

            pthread_mutex_lock(&mutex[2]);
            dirs[0] = dir1;
            dirs[1] = dir2;
            sgn[0] = 0;
            sgn[1] = 0;
            pthread_cond_broadcast(&cond[0]);
            pthread_cond_broadcast(&cond[1]);
            pthread_mutex_unlock(&mutex[2]);
            pthread_mutex_unlock(&mutex[1]);
            pthread_mutex_unlock(&mutex[0]);

            if (dirs[0] == 0 && dirs[1] == 0)
            {
                break;
            }
        }

        // who won?
        int winner = guess[0] == target ? 1 : guess[1] == target ? 2 : 0;

        printf("Game %d: Winner is Player %d\n", game, winner);
    }
    return 0;
}

int main(int argc, char *argv[]) 
{
    /* Problem 7 */
    srand((unsigned int)time(NULL));

    initMutexAndCond();
    sgn[0] = sgn[1] = sgn[2] = sgn[3] = 0;

    pthread_t threads[3];

    checkThread(pthread_create(&threads[0], NULL, player1, NULL), "Thread 1 creation failed");
    checkThread(pthread_create(&threads[1], NULL, player2, NULL), "Thread 2 creation failed");
    checkThread(pthread_create(&threads[2], NULL, referee, NULL), "Referee thread creation failed");
    checkThread(pthread_join(threads[2], NULL), "Referee thread join failed");

    return 0;
}