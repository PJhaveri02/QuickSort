/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: Pranay Jhaveri
    UPI: pjha607

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.

 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/times.h>
#include <pthread.h> // Added pthread header

#define SIZE    10

struct block {
    int size;
    int *data;
};

/**
 * Lock and conditional Variables
*/ 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool thread_busy = false;
bool work_avaliable = false;
struct block left_side_global;

/**
 * Integer that has a pointer is like an array
 * 
*/
void print_data(struct block my_data) {
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}

/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block my_data) {
    int right = my_data.size - 1;
    int left = 0;
    int pivot = my_data.data[right];
    while (left < right) {
        int value = my_data.data[right - 1];
        if (value > pivot) {
            my_data.data[right--] = value;
        } else {
            my_data.data[right - 1] = my_data.data[left];
            my_data.data[left++] = value;
        }
    }
    my_data.data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void quick_sort(struct block my_data) {

    // pthread_mutex_lock(&lock);
    // thread_busy = true;   

    if (my_data.size < 2) {

        pthread_mutex_lock(&lock);
        work_avaliable = false;
        pthread_mutex_unlock(&lock);

        return;
    } else {

        pthread_mutex_unlock(&lock);
        work_avaliable = true;
        pthread_mutex_unlock(&lock);
        
        int pivot_pos = split_on_pivot(my_data);

        struct block left_side, right_side;

        left_side.size = pivot_pos;
        left_side.data = my_data.data;
        right_side.size = my_data.size - pivot_pos - 1;
        right_side.data = my_data.data + pivot_pos + 1;

        if (!thread_busy) {
            left_side_global = left_side;
            pthread_cond_signal(&cond);
            quick_sort(right_side);
        } else {

            quick_sort(left_side);
            quick_sort(right_side);

            pthread_mutex_lock(&lock);
            work_avaliable = false;
            pthread_mutex_unlock(&lock);
        }
    }
}

/* modified quick sort to use two threads */
void *quick_sort_threads(void *arg) {

    pthread_mutex_lock(&lock);
    thread_busy = true;
    pthread_mutex_unlock(&lock);

    if (left_side_global.size > 1) {
        int pivot_pos = split_on_pivot(left_side_global);

        struct block left_side, right_side;

        left_side.size = pivot_pos;
        left_side.data = left_side_global.data;
        right_side.size = left_side_global.size - pivot_pos - 1;
        right_side.data = left_side_global.data + pivot_pos + 1;


        quick_sort(left_side);
        quick_sort(right_side);

        pthread_mutex_lock(&lock);
        thread_busy = false;
        work_avaliable = false;
        pthread_mutex_unlock(&lock);

        while (!work_avaliable)
            pthread_cond_wait(&cond, &lock);
        quick_sort_threads(NULL);
        
    }
}


/* A function that creates 2 threads to sort the array */
void initialise_threads(struct block my_data) {
    if (my_data.size > 1) {

        int pivot_pos = split_on_pivot(my_data);
        
        struct block right_side;

        left_side_global.size = pivot_pos;
        left_side_global.data = my_data.data;
        right_side.size = my_data.size - pivot_pos - 1;
        right_side.data = my_data.data + pivot_pos + 1;

        pthread_mutex_lock(&lock);
        work_avaliable = true;
        pthread_mutex_unlock(&lock);

        // creating thread
        pthread_t lower_bound;

        // // If the thread/(s) are successfully created (thread == 0), wait for the thread to be completed
        if ((int) (pthread_create(&lower_bound, NULL, quick_sort_threads, NULL)) == 0) {
            quick_sort(right_side);
        } else {
            printf("Could not create Thread");
        }
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block my_data) {
    bool sorted = true;
    for (int i = 0; i < my_data.size - 1; i++) {
        if (my_data.data[i] > my_data.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block my_data) {
    srand(1); // the same random data seed every time
    for (int i = 0; i < my_data.size; i++) {
        my_data.data[i] = rand() % 1000;
    }
}

/**
 * argc --> how many arguments where written in terminal 
*/
int main(int argc, char *argv[]) {
	long size;

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    start_block.size = size;
    start_block.data = (int *)calloc(size, sizeof(int)); // returns a pointer once allocated the requested memory.

    // If the calloc function results in an error, it returns null
    if (start_block.data == NULL) {
        printf("Problem allocating memory.\n"); // Can us "perror()" instead of printf
        exit(EXIT_FAILURE);
    }

    produce_random_data(start_block);

    if (start_block.size < 1001)
        print_data(start_block);

    struct tms start_times, finish_times;
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    
    // initialise threads for quicksort
    initialise_threads(start_block);

    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);

    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    free(start_block.data); // Remove the memory allocated "start_block.data" when called "calloc"
    exit(EXIT_SUCCESS);
}