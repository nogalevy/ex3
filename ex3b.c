// --------include section------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

// ----------- const section ---------------------

const int ARR_SIZE = 1000;
const int NUM_OF_CHILDREN = 3;

struct Data{
	pid_t _cpid;
	int _prime;
};

// -------prototype section-----------------------

void create_children();
void handle_child(int pipe_fd1[], int pipe_r[]);
bool prime(int num);
void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[]);
int count_primes(int arr[]);

//---------main section---------------------------

int main(int argc, char *argv[])
{
	create_children();

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void create_children()
{
	pid_t pid[NUM_OF_CHILDREN];
	int i, j;
	int pipe_fd1[2], pipe_fd2[2],pipe_fd3[2],pipe_fd4[2];
	
	//1 pipe for all children to write to parent
	//3 pipes for parent to write to children seperately
  	if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1 || 
  		pipe(pipe_fd3) == -1 || pipe(pipe_fd4) == -1)
  	{
   	 	perror("cannot open pipe");
    	exit(EXIT_FAILURE) ;
  	}

	for(i = 0; i < NUM_OF_CHILDREN; i++)
	{
		pid[i] = fork();

		if(pid[i] < 0) // handle error in fork()
		{
			perror("Cannot fork()");
			exit (EXIT_FAILURE);
		}
		if(pid[i] == 0) //if child
		{
			//each child gets different pipe to read from parent
			if(i = 0)
				handle_child(pipe_fd1, pipe_fd2);

			if(i = 1)
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

void handle_child(int pipe_fd1[], int pipe_r[])
{
	signal(SIGTERM, catch_sigterm);
	Data *data = (struct Data*)malloc(sizeof(struct Data));
	data->_cpid = getpid();
	int num, counter;

	close(pipe_fd1[0]);
	close(pipe_r[1]);

	srand(17); //turn 17 into const global seed

	while(true)
	{
		num = rand()%999 + 2;
		if(prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			data->_prime = num;
			write(pipe_fd1[1], data, sizeof(struct Data));
		}
	}
	//when kill signal received, SHOULD get out of loop
	//read number of 0's sent, prints
	read(pipe_r[0], num, sizeof(int)); //how does it know when to stop reading??
	while(num != eof)
	{
		if (num == 0)
			counter++;
		read(pipe_r[0], num, sizeof(int));
	}
	close(pipe_fd1[1]);
	close(pipe_r[0]);
	printf("Child process %d sent %d new primes", getpid(), counter);
	exit(EXIT_SUCCESS);
	
}

//-------------------------------------------------

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

void catch_sigterm(int signum)
{
	signal(SIGTERM, catch_sigterm);	
}

//-------------------------------------------------

void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[])
{
	int primes[ARR_SIZE],
		primes_count[ARR_SIZE] = {0};
	int filled = 0, index;
	struct Data *data = (struct Data*)malloc(sizeof(struct Data));

	close(pipe_fd1[1]);
	close(pipe_fd2[0]);
	close(pipe_fd3[0]);
	close(pipe_fd3[0]);

	while(filled < ARR_SIZE)
	{
		//read number
		//sends prime count to child accordingly
		//enters into primes
		//increases prime count
		//increases filled counter

		read(pipe_fd1[0], data, sizeof(struct Data));

		switch(data->_cpid)
		{
			case child[0]:
				write(pipe_fd2[1], primes_count[data->_prime], sizeof(int));
				break;
			case child[1]:
				write(pipe_fd3[1], primes_count[data->_prime], sizeof(int));
				break;
			case child[2]:
				write(pipe_fd3[1], primes_count[data->_prime], sizeof(int));
				break;
			default:
				break;
		}
		primes[filled] = data->_prime; //adds prime
		primes_count[data->_primes]++;	//adds to counter
		filled++;						//increases fill number
	}
	//kills children
	//counts how many different prime numbers
	//prints this counter^
	for(index = 0; index < NUM_OF_CHILDREN; index++)
		kill(child[index], SIGTERM);

	close(pipe_fd1[0]);
	close(pipe_fd2[1]);
	close(pipe_fd3[1]);
	close(pipe_fd3[1]);

	printf("The number of different integers received is: %d", count_primes(primes_count));
}

//-------------------------------------------------

int count_primes(int arr[])
{
	int index, counter = 0;
	for(index = 0; index < ARR_SIZE; index++)
		if(arr[index] != 0)
			counter++;

	return counter;
}







