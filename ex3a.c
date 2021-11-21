// --------include section------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

// ----------- const section ---------------------

const int ARGC_SIZE = 2;
const int SIZE = 50000;
const int NUM_OF_LOOPS = 50;

// -------prototype section-----------------------

void check_argv(int argc );
void calc_sort_times();
void bubble_sort(int arr[]);
void quick_sort(int arr[], int first_i, int last_i);
void parent_calc(int bubble_data[], int quick_data[]);
void randomize_array(int arr[]);
void handle_child(int child_num, int arr[], int pipe_fd[]);
void handle_bubble_sort(int arr[], int pipe_fd[]);
void handle_quick_sort(int arr[], int pipe_fd[]);
int partition (int arr[], int low, int high);
void swap(int* a, int* b);
void handle_father(int pipefd[], int bubble_data[], int quick_data[], int index);

//---------main section---------------------------

int main(int argc, char *argv[])
{
	struct timeval t0, t1;
	gettimeofday(&t0, NULL); //calculating start time

	check_argv(argc);
	srand(atoi(argv[1]));
	calc_sort_times(argv[1]);

	gettimeofday(&t1, NULL); //calculating end time
	//printing time parent took to run
	printf("%f\n",(double)(t1.tv_usec - t0.tv_usec)/1000000 +
									(double)(t1.tv_sec - t0.tv_sec));

	return EXIT_SUCCESS;
}

//-------------------------------------------------

//function receives filename and calculates sort time
//of bubble and quick sort via children
void calc_sort_times()
{
	int i, j;
	pid_t pid;
	int arr[SIZE], bubble_data[NUM_OF_LOOPS], quick_data[NUM_OF_LOOPS];
  int pipe_fd[2];

  if (pipe(pipe_fd) == -1)
  {
    perror("cannot open pipe");
    exit(EXIT_FAILURE) ;
  }

	for(i = 0; i < NUM_OF_LOOPS; i++)
	{
		randomize_array(arr); //putting random numbers in array

		//create two child proccess
		for(j = 0; j < 2; j++)
		{
			pid = fork();

			if(pid < 0) // handle error in fork()
			{
				perror("Cannot fork()");
				exit (EXIT_FAILURE);
			}
			if(pid == 0) //if child
				handle_child(j, arr, pipe_fd);
		}

		//parent wait for both children
		for(j = 0; j < 2; j++)
			wait(NULL);
		handle_father(pipe_fd, bubble_data, quick_data, i);
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	parent_calc(bubble_data, quick_data);
}

//------------------------------------------------

void handle_father(int pipe_fd[], int bubble_data[], int quick_data[], int index)
{
	char type1, type2;
	double time1, time2;
    close(STDIN_FILENO);
    dup(pipe_fd[0]);

		scanf("%s %lf %s %lf", &type1, &time1, &type2, &time2);
		if(type1 == 'b')
		{
			bubble_data[index] = time1;
			quick_data[index] = time2;
		}
		else
		{
			bubble_data[index] = time2;
			quick_data[index] = time1;

		}
}

//------------------------------------------------

//function receives child number, randomized array and opened file
//appropriate child sorts accordingly
void handle_child(int child_num, int arr[], int pipe_fd[])
{
	close(pipe_fd[0]);
	close(STDOUT_FILENO);
	dup(pipe_fd[1]);

	if(child_num == 0)
		handle_bubble_sort(arr, pipe_fd);
	else
		handle_quick_sort(arr, pipe_fd);

	close(pipe_fd[1]);
	exit(EXIT_SUCCESS); //child ends process
}

//------------------------------------------------

//function recieves array and enters randomized numbers into it
void randomize_array(int arr[])
{
	int index;

	for(index = 0; index < SIZE; index++)
		arr[index] = (rand() % 1000);
}

//------------------------------------------------

//function receives array and file, sorts array and puts calculated
//run time into file
void handle_bubble_sort(int arr[], int pipe_fd[])
{
	struct timeval t0, t1;

	gettimeofday(&t0, NULL);
	bubble_sort(arr);
	gettimeofday(&t1, NULL);

	printf("%s %f\n", "b", (double)(t1.tv_usec - t0.tv_usec)/1000000 +
 								(double)(t1.tv_sec - t0.tv_sec));
}

//------------------------------------------------

//function receives array and file, sorts array and puts calculated
//run time into file
void handle_quick_sort(int arr[], int pipe_fd[])
{
	int first = 0, last = SIZE -1;
	struct timeval t0, t1;

	gettimeofday(&t0, NULL);
	quick_sort(arr, first, last);
	gettimeofday(&t1, NULL);

  printf("%s %f\n", "q", (double)(t1.tv_usec - t0.tv_usec)/1000000 +
	 								(double)(t1.tv_sec - t0.tv_sec));
}
//------------------------------------------------

//function receives array and sorts it using the bubble sort algorithm
void bubble_sort(int arr[])
{
	int step, i, temp;
	// loop to access each array element
	for (step = 0; step < SIZE - 1; ++step) {
		// loop to compare array elements
		for (i = 0; i < SIZE - step - 1; ++i) {
			// compare two adjacent elements
			// change > to < to sort in descending order
			if (arr[i] > arr[i + 1]) {
				// swapping occurs if elements
				// are not in the intended order
				temp = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = temp;
			}
		}
	}
}

//------------------------------------------------

//function receives array, starting and ending point and sorts the array
//using the quick sort algorithm
void quick_sort(int arr[], int first_i, int last_i)
{
    if(first_i < last_i)
    {
        int q;
        q = partition(arr, first_i, last_i);
        quick_sort(arr, first_i, q-1);
        quick_sort(arr, q+1, last_i);
    }
}

//------------------------------------------------

//function receives array and partitions accordingly
int partition (int arr[], int low, int high)
{
    int pivot = arr[high];  // selecting last element as pivot
    int i = (low - 1);  // index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If the current element is smaller than or equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

//------------------------------------------------


void parent_calc(int bubble_data[], int quick_data[])
{
	double sum_bsort, sum_qsort, min_bsort, min_qsort,
				max_bsort, max_qsort, curr_b, curr_q, avg_bs, avg_qs;

	int index;
	sum_bsort = sum_qsort = max_bsort = max_qsort = 0;
	min_bsort = bubble_data[0];
	min_qsort = quick_data[0];

	for(index = 0; index < NUM_OF_LOOPS; index++)
	{
		curr_b = bubble_data[index];
		curr_q = quick_data[index];

		if (curr_b < min_bsort)
			min_bsort = curr_b;
		if(curr_b > max_bsort)
			max_bsort = curr_b;
		if(curr_q < min_qsort)
			min_qsort = curr_q;
		if(curr_q > max_bsort)
			max_qsort = curr_q;

		sum_bsort += curr_b;
		sum_qsort += curr_q;
	}
	avg_bs = sum_bsort / NUM_OF_LOOPS;
	avg_qs = sum_qsort / NUM_OF_LOOPS;

	printf("%lf %lf %lf %lf %lf %lf\n", avg_bs, avg_qs, min_bsort,
													min_qsort, max_bsort, max_qsort);
}

//------------------------------------------------

//function checks that both input file and output file names are
//given in argument vector
void check_argv(int argc )
{
	if(argc != ARGC_SIZE)
	{
		printf("Error! Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
}

//------------------------------------------------

//function swaps two numbers
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
