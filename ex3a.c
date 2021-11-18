// --------include section------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //?
#include <sys/types.h>
//#include <time.h> //for clock()
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

// ----------- const section ---------------------

const int ARGC_SIZE = 2;
const int SIZE = 50000;
const int NUM_OF_LOOPS = 50;

// -------prototype section-----------------------

FILE* open_file(char* argv,  char *mode);
void close_file(FILE **fp);
void check_argv(int argc );
void calc_sort_times(char *filename);
void bubble_sort(int arr[]);
void quick_sort(int arr[], int first_i, int last_i);
void parent_calc(int pipe_fd_qs[], int pipe_fd_bs[]);
void randomize_array(int arr[]);
void handle_child(int child_num, int arr[], int pipe_fd[]);
void handle_bubble_sort(int arr[], int pipe_fd[]);
void handle_quick_sort(int arr[], int pipe_fd[]);
int partition (int arr[], int low, int high);
void swap(int* a, int* b);

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
	int arr[SIZE];
  int pipe_fd[2];

  if (pipe(pipe_fd) == -1)
  {
    perror("cannot open pipe");
    exit(EXIT_FAILURE) ;
  }

	for(i = 0; i < NUM_OF_LOOPS; i++)
	{
		randomize_array(arr); //putting random numbers in array

    close(pipe_fd[0]); //close input
    close(STDOUT_FILENO); //close standart output
    dup(pipe_fd[1]); //move standart output pointer to the output

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
    close(pipe_fd[1]);
		//parent wait for both children
		for(j = 0; j < 2; j++)
			wait(NULL);
	}

	parent_calc(pipe_fd); //parent prints data from file
}

//------------------------------------------------

//function receives child number, randomized array and opened file
//appropriate child sorts accordingly
void handle_child(int child_num, int arr[], int pipe_fd[])
{
	if(child_num == 0)
		handle_bubble_sort(arr, pipe_fd);
	else
		handle_quick_sort(arr, pipe_fd);

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

  //print to pipe
  // close(pipe_fd[0]); //close input
  // close(STDOUT_FILENO); //close standart output
  // dup(pipe_fd[1]); //move standart output pointer to the output


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

  //print to pipe
  // close(pipe_fd[0]); //close input
  // close(STDOUT_FILENO); //close standart output
  // dup(pipe_fd[1]); //move standart output pointer to the output

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

//function receives file filled with run time data and prints
//out average, min and max of all run time data
void parent_calc(int pipe_fd[])
{
	double sum_bsort, sum_qsort, min_bsort, min_qsort,
				max_bsort, max_qsort, curr, avg_bs, avg_qs;
	char type;
  int time_of_bs[NUM_OF_LOOPS] = {0},
      time_of_qs[NUM_OF_LOOPS] = {0};
  int bs_i  = 0, qs_i = 0; //loop on arr
	//setting initial values
	sum_bsort = sum_qsort = max_bsort = max_qsort = 0;
	min_qsort = min_bsort = 100;

  //get from pipe qs data
  //close(pipe_fd_bs[1]);
  close(STDIN_FILENO);
  dup(pipe_fd[1]);

  // get data from pipe
  // close(pipe_fd_bs[1]);
  // close(STDIN_FILENO);
  // dup(pipe_fd_bs[1]);

  scanf("%c", &type);

// (feof(stdin))?
// (ioctl(0, I_NREAD, &n) == 0 && n > 0) ?
	while(type != eof()) /*??????????*/)
	{
		scanf("%lf", &curr); //read data about specific sorting algorithm

		if(type == 'b') //if bubble sort
    {
      time_of_bs[bs_i] = curr; //insert to array of bubble sort
      bs_i++;
    }
		else	//if quick sort
    {
      time_of_qs[qs_i] = curr; //insert to array of quick sort
      qs_i++;
    }
		scanf("%c", &type); 	//read next sort type
	}

  wait(NULL);// wait for childs before calc

  // calc arr
  for (i = 0; i < NUM_OF_LOOPS; i++)
  {
    sum_bsort += time_of_bs[i];
    sum_qsort += time_of_qs[i];

    if(max_bsort < time_of_bs[i])
      max_bsort = time_of_bs[i];
    if(max_qsort < time_of_qs[i])
      max_qsort = time_of_qs[i];
    if(min_bsort > time_of_bs[i])
      min_bsort = time_of_bs[i];
    if(min_qsort > time_of_qs[i])
      min_qsort = time_of_qs[i];
  }

	//dividing sums to get average
	avg_bs = sum_bsort / NUM_OF_LOOPS;
	avg_qs = sum_qsort / NUM_OF_LOOPS;

	//prints onto screen values calculated
  //TODO: add time of parent
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
