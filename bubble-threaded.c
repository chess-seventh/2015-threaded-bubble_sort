/**
 * @file bubble-threaded.c
 *
 * Functions we use in this program.
 *
 *
 * @author Francesco Piva
 */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


/// maximum size of array
#define MAX 50000 


/// array to hold integers
int arr[MAX]; 
int length;
int global = 0;

/// structure to be passed to thread function
struct data {
	int start; /// start index of array
	int end; /// end index of the array
	int id; /// id of each thread 
};
	
/// an array of mutext variables for adjacent threads
pthread_mutex_t mutex[MAX];

/// List of all functions in this TP.
	int checkSorted(int len);
	void fillArrayRandom(int len);
	void print(int len);
	void swap(int first, int second);
	void bubble_sort(int *tab, int length);
	void *doSorting(void *arg);

/**
 * The check if array is sorted.
 *
 * function to check if array is properly sorted 
 * it returns 1 on success and 0 on failure 
 *
 * @param int length - lenght of the array (of type int)
 * @return 1 if all went well 0 on error, array is not sorted.
 */
int checkSorted(int len) {
	int i;
	/// loop over elements-1 of the array
	/// each element must be less than or equal to its next element for the array to be sorted
	for(i =0;i<len-1;i++) {
		/// check if current element is greater then next element
		if(arr[i] > arr[i+1]) {
			return 0;
		}
	}
	/// Array is properly sorted here.
	global = 1;
	return 1;
}

/**
 * Function that fills array with random numbers..
 *
 * @param int length - lenght of the array (of type int)
 *
 */
void fillArrayRandom(int len) {
	int i;
	srand(time(NULL)); /* set the initial seed for generating a unique seed each time program is run */
	for(i = 0;i <len ; i++)	{
		arr[i] = rand()%10000;
	}
}


/**
 * Function to print the array...
 *
 * @param int length - lenght of the array (of type int)
 *
 */
 void print(int len) {
	int i;
	for(i =0 ; i < len; i++)
		printf("%d ",arr[i]);

	printf("\n");
}



/**
 * Function to swap two numbers in array
 *
 * @param int first- the first number of array to swap (of type int)
 * @param int second - the second number of array to swap (of type int)
 *
 */
void swap(int first, int second) {
	int temp;
	temp = arr[first];
	arr[first] = arr[second];
	arr[second] = temp;
	printf("SWAPPED !!! \n");
}


/**
 * Function of bubble sort.
 *
 * @param int *tab -  pointer to the array we are sorting (of type int*)
 * @param int length - lenght of the array (of type int)
 *
 */
void bubble_sort(int *tab, int length) {
	int i,j;
	for ( i = length-1; i > 0; i--) {
		for ( j = 0; j < i; j++) {
			if (tab[j] > tab[i]) {
				int tmp = tab[j];
				tab[j] = tab[i];
				tab[i] = tmp;
			}
		}
	}
	printf("Sorted !\n");
}


/**
 * Thread function to do the sorting.
 *
 * @param void *arg -  pointer to the thread 
 *
 */
void *doSorting(void *arg) {
	struct data *local_data = (struct data*)arg;
	
	/// loop started until sorting is completed 
	while(1) {
		/// do the sorting for non overlapping section
		/// if this is the first section
		if(local_data->start == 0) {
			int len = local_data->end - local_data->start + 1;
			/// bubble sort code
			bubble_sort(arr+local_data->start,len);	
		}
		/// if this is not the first section to sort
		else {
			int len = local_data->end - local_data->start;
			printf("i am in the first sectoin of the while else... ThreadID: %d \n", local_data->id);
			printf("Thread ID%d\n", local_data->id);
			bubble_sort(arr+local_data->start+1,len); /* bubble sort code */
		}

		/// lock on mutex variables for adjacent threads
		/// two locks will be taken . one for the current section and one for the adjacent section
		pthread_mutex_lock(&mutex[local_data->id]);
		pthread_mutex_lock(&mutex[local_data->id + 1]);

		/// first check if array has been fully sorted
		if(checkSorted(length) == 1) {
			/// if array is sorted, unlock both the mutex variables and break from the loop
			pthread_mutex_unlock(&mutex[local_data->id]);
			pthread_mutex_unlock(&mutex[local_data->id + 1]);
			break;
				
		}
		/// now we sort the part which is common
		
		printf("going to sort from indices = %d to %d\n",local_data->start,local_data->end);

		if(local_data->end != length-1)
			printf("i am in the if statement local_data->end != lengthg -1 !! ThreadID: %d \n", local_data->id);
		bubble_sort(arr+local_data->end,2);
		
		/// unlock the both mutex variables 
		pthread_mutex_unlock(&mutex[local_data->id]);
                pthread_mutex_unlock(&mutex[local_data->id + 1]);
		
	}
	printf("Thread  %d going to exit\n",local_data->id);
	pthread_exit(NULL);	
}



/**
 * Program entry point.
 *
 * @param int *argc - number of total arguments.
 * @param char **argv - values to input.
 *
 * @return int EXIT_SUCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Invalid Args! Use-> <program-name> <length of array> <no of threads>\n");
		exit(0);
	}

	/// extract values from variables input of user
	length = atoi(argv[1]);
	int noOfThreads = atoi(argv[2]);

	/// fill the array with random values
	fillArrayRandom(length);

	/// print the randomly generated array
	printf("The Randomly generated unsorted array is: \n");
	print(length);

	/// create the threads and do the sorting
	printf("Sorting Started...\n");
	
	/// define the maximum number of thread pool which can be used to pick the threads
	pthread_t threads[MAX];
	
	/// thread attribute variable 
	pthread_attr_t attr;
	
	/// thread initializer variable
	int k;
	for(k = 0;k<=noOfThreads;k++) {
		pthread_mutex_init(&mutex[k],NULL);
	}
	
	int i;

	/// devide work for each thread section
	int part = length/noOfThreads;
	int st = 0;
	int en = st + part;
	
	/// thread attribute initialiser
	pthread_attr_init(&attr);		

	/// create the number of threads inputted by user and do the sorting on each section by respective threads
	for(i =0; i <noOfThreads; i++) {

		/// create the structure to be passed to each thread
		struct data *my_data = malloc(sizeof(struct data));
		
		printf("start = %d and end = %d\n",st,en);	

		/// fill in the values in structure variable
		my_data->id = i;
		my_data->start = st;
		my_data->end = en;


		/// now we create the threads
		if ( (pthread_create(&threads[i],&attr,doSorting,(void *)my_data)) == 0) {
			if(i != noOfThreads - 2) {
				st = en;
				en += part;
			}
			/// for last thread due to size of array.
			else {
				st = en ;
				en  = length - 1;
			} 	
		}
		else {
			printf("Thread # %d failed to create. Exiting now\n", i);
			exit(EXIT_FAILURE);
		}
	}

	
	/// wait for all the worker threads to finish
	for (i=0; i<noOfThreads; i++) {
		pthread_join(threads[i], NULL);
	}
	
	 if(checkSorted(length) == 1) {
		/// print the sorted array
		printf("Sorting Ended...\n");
		printf("The sorted array is :\n");
		print(length);
	 }
	 else {
	 	printf("Something went wrong, the array is not sorted...\n");
	 	exit(EXIT_FAILURE);
	 }

	return 0;	
}

