// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

// ----------- const section ---------------------

const int ARR_SIZE = 10;
const int NUM_OF_CHILDREN = 3;
const int SEED = 17;
const int NEW_PRIME = 0;

enum CHILDREN { CHILD1, CHILD2, CHILD3};

struct Data {
	pid_t _cpid; //child pid
	int _prime; //
};

// -------prototype section-----------------------

void create_children();
void handle_child(int pipe_fd1[], int pipe_r[]);
bool prime(int num);
void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[]);
int count_primes(int arr[]);
void catch_sigterm(int signum);
void reset_arr(int arr[], int size_arr);

//---------main section---------------------------f

int main(int argc, char *argv[])
{
	create_children();

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void create_children()
{
	pid_t pid[NUM_OF_CHILDREN]; //create array of processes
	int i;
	int pipe_fd1[2], //pipe for all children to write to parent
	 		pipe_fd2[2], //pipe for parent to write child1
			pipe_fd3[2], //pipe for parent to write child2
			pipe_fd4[2]; //pipe for parent to write child3

	//create pipes and handle eroor
	if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1 ||
		pipe(pipe_fd3) == -1 || pipe(pipe_fd4) == -1)
	{
 	 	perror("cannot open pipe");
  	exit(EXIT_FAILURE) ;
	}

	// create 3 children
	for(i = 0; i < NUM_OF_CHILDREN; i++)
	{
		pid[i] = fork(); //create child process

		if(pid[i] < 0) // handle error in fork()
		{
			perror("Cannot fork()");
			exit (EXIT_FAILURE);
		}

		if(pid[i] == 0) //if child
		{
			//each child gets different pipe to read from parent
			if(i == CHILD1)
				handle_child(pipe_fd1, pipe_fd2);

			else if(i == CHILD2)
				handle_child(pipe_fd1, pipe_fd3);

			else
				handle_child(pipe_fd1, pipe_fd4);
		}
	}
	handle_father(pipe_fd1, pipe_fd2, pipe_fd3, pipe_fd4, pid);
}

//-------------------------------------------------

//all children write through the same pipe
//children read through 3 seperate pipes?
//gets pipe_fd1[] = pipe to write parent
//gets pipe_r[] = pipe to read from parent
void handle_child(int pipe_fd1[], int pipe_r[])
{
	signal(SIGTERM, catch_sigterm);
	struct Data *data = (struct Data*)malloc(sizeof(struct Data)); //alocate data
	data->_cpid = getpid();
	int num, counter = 0;

	close(pipe_fd1[0]); //close pipe for reading
	close(pipe_r[1]); //close pipe for writing

	srand(SEED); //turn 17 into const global seed

	while(true)
	{
		num = rand()%999 + 2; //randomize num between 2 to 1000
		if(prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			data->_prime = num; // save prime num in struct
			// write to pipe the data (pid and prime num)
			write(pipe_fd1[1], data, sizeof(struct Data)); //Q : send pointer to data or the data data?
		}

		read(pipe_r[0], &num, sizeof(int)); //how does it know when to stop reading??
		if (num == NEW_PRIME)
			counter++;

	}

	//when kill signal received, SHOULD get out of loop
	//read number of 0's sent, prints

	//close all
	close(pipe_fd1[1]);
	close(pipe_r[0]);
	printf("Child process %d sent %d new primes", getpid(), counter);
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------
//gets intiger and check if is prime
bool prime(int num)
{
	int i;
	for(i = 2; i*i < num; i++)
	{
		if(num % i == 0)
			return false;
	}
	return true;
}

//-------------------------------------------------
//catch signal of SIGTERM
void catch_sigterm(int signum)
{
	signal(SIGTERM, catch_sigterm);
}

//-------------------------------------------------
// gets pipe of all children and the pid array
void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[])
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, index;
	struct Data *data = (struct Data*)malloc(sizeof(struct Data));

	reset_arr(primes_count, ARR_SIZE);

	close(pipe_fd1[1]); //close for reading
	close(pipe_fd2[0]); //close for writing
	close(pipe_fd3[0]); //close for writing
	close(pipe_fd3[0]); //close for writing

	while(filled < ARR_SIZE)
	{
		//read number
		//sends prime count to child accordingly
		//enters into primes
		//increases prime count
		//increases filled counter

		// read from children pipe to get the data (prime number and child pid)
		read(pipe_fd1[0], data, sizeof(struct Data));

		//check which child sent the number depend on the pid
		//send the counter of the prime num

			if (data->_cpid == child[0])
				write(pipe_fd2[1], &primes_count[data->_prime], sizeof(int));
				//break;
			if (data->_cpid == child[1])
				write(pipe_fd3[1], &primes_count[data->_prime], sizeof(int));
				//break;
			if (data->_cpid == child[2])
				write(pipe_fd4[1], &primes_count[data->_prime], sizeof(int));
				//break;

		primes_count[data->_prime]++;	//adds to counter
		filled++;					//increases fill number
	}
	//kills children
	//counts how many different prime numbers
	//prints this counter^
	for(index = 0; index < NUM_OF_CHILDREN; index++)
		kill(child[index], SIGTERM);

	//close all
	close(pipe_fd1[0]);
	close(pipe_fd2[1]);
	close(pipe_fd3[1]);
	close(pipe_fd3[1]);

	printf("The number of different integers received is: %d", count_primes(primes_count));
}

//-------------------------------------------------
//
int count_primes(int arr[])
{
	int index, counter = 0;
	//start on i=2 - we can be sure that 0 and 1 is empty
	for(index = 2; index < ARR_SIZE; index++)
		if(arr[index] != 0)
			counter++;

	return counter;
}

void reset_arr(int arr[], int size_arr)
{
	int i;
	for(i = 0; i < size_arr; i++)
	{
		arr[i] = 0;
	}
}
