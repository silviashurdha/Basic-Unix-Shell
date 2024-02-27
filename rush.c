#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_INPUT 255

// Error message for whenever an error is encountered in the program
void error_message()
{
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	fflush(stdout);
}

void redirection(char *input_list[], char *path[], int i, int parallel)
{
	int j;
	char *output;
	char *output_list[i];
	int output_index = i - 1;
	output = input_list[i - 1]; // output file is set to end of the list

	for (j = 0; j < (i - 2); j++)
	{
		output_list[j] = input_list[j]; // add commands before > sign to new list for execv()
	}

	int greater_index = 0; // variable to keep track of > index in the list
	for (j = 0; j < i; j++)
	{
		if (strcmp(input_list[j], ">") == 0)
		{ // check to see if the element at j index is the > sign
			break;
		}
		greater_index++;
	}

	if ((output_index - greater_index) != 1)
	{ // check if there is only 1 argument after > sign
		error_message();
		return;
	}

	if (strcmp(output, ">") == 0)
	{					 // if no file name given after >, error message
		error_message(); // probably can remove this if i want
		return;
	}

	output_list[j] = NULL; // make end of output_list null for execv()

	for (j = 0; path[j] != NULL; j++)
	{
		char pathfile[MAX_INPUT] = ""; // pathfile for execv()
		strcat(pathfile, path[j]);	   // concatenate the path to the pathfile
		strcat(pathfile, "/");
		strcat(pathfile, input_list[0]); // concatenate the command to the end of the pathfile

		if (access(pathfile, X_OK) == 0)
		{					  // check to see if access path is valid
			int pid = fork(); // create child process

			if (pid == 0)
			{

				int file_id = open(output, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // open file for writing
				if (file_id == -1)
				{
					error_message();
				} // check that the file opened correctly

				if (dup2(file_id, STDOUT_FILENO) == -1)
				{ // shift the output to the file
					error_message();
				}

				execv(pathfile, output_list);
				close(file_id); // close the file
			}
			else
			{
				if (parallel == 1)
				{
					return;
				}
				wait(NULL);
			} // make the parent process wait

			return;
		}
		pathfile[0] = '\0'; // clear the pathfile for the next iteration
	}
	error_message(); // if the loop exits then no processes ran correctly and there was an error
}

void execute_commands(char *input_list[], char *path[], int i)
{

	int j;
	for (j = 0; path[j] != NULL; j++)
	{ // iterate through the different paths setup by the user
		char pathfile[MAX_INPUT] = "";
		strcat(pathfile, path[j]); // concatenate path to pathfile for execv()
		strcat(pathfile, "/");
		strcat(pathfile, input_list[0]); // concatenate command to end of pathfile

		if (access(pathfile, X_OK) == 0)
		{					  // check filepath to make sure it is valid
			int pid = fork(); // create child process
			if (pid == 0)
			{
				execv(pathfile, input_list); // execute command
			}
			else
			{
				wait(NULL);
			} // wait for parent process

			return;
		}
		pathfile[0] = '\0'; // clear pathfile for next iteration loop
	}
	error_message(); // error message if command does not execute
}

void parallel(char *input_list[], char *path[], int list_length, int re_counter)
{

	char *output_list[MAX_INPUT][MAX_INPUT]; // create 2D array to keep track of all commands for parallel processing
	int c = 0;								 // iterate through outer array
	int a = 0;								 // iterate through inner array string
	int process_num = 0;

	// Create loop to sort input_list into seperate commands
	int i;
	for (i = 0; i < list_length; i++)
	{

		// Check that the user did not input a bunch of & signs in a row without commands in between
		if ((i > 0) && (strcmp("&", input_list[i - 1]) == 0) && (strcmp("&", input_list[i]) == 0))
		{
			continue;
		}

		if (strcmp("&", input_list[i]) == 0)
		{
			output_list[c][a] = NULL; // execv needs null terminated list to run
			c++;					  // increment so it can start on next list
			a = 0;					  // reset to 0 for start of next list
			process_num++;			  // count number of processes
		}
		else
		{
			if (input_list[i] == NULL)
			{
				continue;
			}
			output_list[c][a] = input_list[i];
			a++; // increment for next arguments stored
		}
	}

	output_list[c][a] = NULL; // null terminate for last list
	int no_run = 0;			  // flag variable to check if process was successful or not

	int j;
	int redir = 0;

	for (j = 0; j < process_num + 1; j++)
	{
		int k;
		if (output_list[j][0] == NULL)
		{
			continue;
		} // if command in list is null, then skip over

		char *redirection_list[MAX_INPUT]; // create new list for redirection
		int l;

		for (l = 0; output_list[j][l] != NULL; l++)
		{											 // search through list for > symbols
			redirection_list[l] = output_list[j][l]; // add arguments to a new list for redirection
			if (strcmp(output_list[j][l], ">") == 0)
			{
				redir = 1; // if redirection is detected then mark the variable as such
			}
		}

		if (redir == 1)
		{
			redirection(redirection_list, path, l, 1); // call the redirection function
			redir = 0;
			continue;
		}
		else
		{

			for (k = 0; path[k] != NULL; k++)
			{ // loop through path

				char pathfile[MAX_INPUT] = ""; // concatenate path to file name
				strcat(pathfile, path[k]);
				strcat(pathfile, "/"); // concatenate command to end of filename
				strcat(pathfile, output_list[j][0]);

				if (access(pathfile, X_OK) == 0)
				{					  // check if file path for command is valid
					int pid = fork(); // start child process

					if (pid == 0)
					{
						execv(pathfile, output_list[j]); // run process
					}

					no_run = 1; // mark that process ran
					break;
				}

				pathfile[0] = '\0'; // reset the pathfile variable for the next iteration of the loop
			}

			if (no_run == 0)
			{ // if the process does not run, then print the error message
				error_message();
			}
			no_run = 0; // reset the no_run variable for the next iteration
		}
	}

	for (j = 0; j < process_num + 1; j++)
	{ // wait for all the processes
		wait(NULL);
	}
}

int main(int argc, char *argv[])
{

	// if there is any other arguments in the command line when running rush, then program fails
	if (argc > 1)
	{
		error_message();
		exit(1);
	}

	// Shells repeatedly prints the prompt rush>, when exit is typed, shell ends
	char *input = NULL; // create string for reading the user input
	size_t input_length = 0;
	char exit_str[] = "exit"; // exit string for comparison
	char cd_str[] = "cd";	  // cd string for comparision
	char path_str[] = "path"; // path string for comparision

	char delimiters[] = " \t\n\r"; // list of white space characters for the program to ignore
	char *input_list[MAX_INPUT] = {NULL};

	// Set up initial path for program
	char *path[MAX_INPUT] = {NULL};
	path[0] = malloc(strlen("/bin") + 1); // allocate memory for path
	strcpy(path[0], "/bin");			  // set path to defualt value of /bin'

	while (1)
	{
		printf("rush> "); // print rush>
		fflush(stdout);
		getline(&input, &input_length, stdin);	 // get user input from terminal
		char *token = strtok(input, delimiters); // get first word from user input

		int i = 0;
		int re_counter = 0;
		int and_counter = 0;
		while (token != NULL)
		{
			if ((strcmp(">", token)) == 0)
			{
				re_counter++;
			} // check for redirection
			if ((strcmp("&", token)) == 0)
			{
				and_counter++;
			} // check for parallel commands

			input_list[i] = malloc(strlen(token) + 1); // allocate space in the array for the input
			strcpy(input_list[i], token);			   // copy the input into the array

			token = strtok(NULL, delimiters); // Get the next word in the input
			i++;							  // increment the counter to keep track the length of the input
		}

		input_list[i] = NULL; // make sure the list is null terminated for execv()

		if (input_list[0] == NULL)
		{
			continue;
		} // If no user input was detected then continue the loop

		if (and_counter > 0)
		{
			parallel(input_list, path, i, re_counter); // go to parallel process function to execute commands
			continue;
		}

		if (re_counter > 1)
		{
			error_message();
			continue;
		} // if 2 or more arguments after >, then this is an error

		if (input_list[0] != NULL)
		{ // Detect exit input and end the program

			if ((strcmp(exit_str, input_list[0])) == 0)
			{ // Execute exit call
				if ((i) == 1)
				{
					free(input);
					int j;
					for (j = 0; input_list[j] != NULL; j++)
					{ // deallocate memory for input_list
						free(input_list[j]);
					}

					for (j = 0; path[j] != NULL; j++)
					{ // deallocate memory for path
						free(path[j]);
					}

					exit(0);
				}
				else
				{
					error_message();
				} // error if exit is passed with arguments

				// Detect cd command and call chdir() to change directory
			}
			else if ((strcmp(cd_str, input_list[0])) == 0)
			{
				if (i == 2)
				{
					if (chdir(input_list[1]) != 0)
					{
						error_message();
					}
				}
				else
				{
					error_message();
				}

				// Detect path input and update path accordingly
			}
			else if ((strcmp(path_str, input_list[0])) == 0)
			{

				if (input_list[1] == NULL)
				{
					int j;
					for (j = 0; path[j] != NULL; j++)
					{ // if path input by user is null, set path to null
						free(path[j]);
						path[j] = NULL;
					}
				}
				else
				{ // set path to user specification
					int j;
					for (j = 1; j < i; j++)
					{
						path[j - 1] = malloc(strlen(input_list[j]) + 1); // allocate memory for path array
						strcpy(path[j - 1], input_list[j]);				 // copy user input into path array
					}
				}

				// If input is not built in command, run as external process
			}
			else
			{
				if (re_counter == 1)
				{
					redirection(input_list, path, i, 0);
				} // execute redirection
				else
				{
					execute_commands(input_list, path, i);
				} // execute external command without redirection
			}
		}

		// reset input array for next iteration of loop
		int j;
		for (j = 0; input_list[j] != NULL; j++)
		{
			free(input_list[j]);
		}

		free(input); // free memory allocation getline input
		input = NULL;
		input_length = 0; // reset argument counter
	}

	return 0;
}
