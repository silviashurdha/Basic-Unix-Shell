# Basic-Unix-Shell

 A simple yet efficient Unix Shell called **Rush (Rapid Unix SHell)**. It is a command line interpreter that operates by creating child processes to execute commands entered by the user.

 ## Technologies

This project was created using:

* **Programming Language:** C
* **Operating System:** Ubunutu 22.04.3 LTS

## Project Structure and Functionality

The shell runs in a loop, accepting user input, parsing it and executing the corresponding command. It use functions such **'fork()'**, **'execv()'** and **'wait()'** for process management. 

**Built-in Commands:** The program has built in commands for commands such as **'exit'**, **'cd'** and **'path'**.

**Path:** The shell path specifies the directories in which to search for the executable files.

**Redirection:** This shell supports the redirection of standard output using the **'>'** operator.

**Parallel Commands:** Parallel commands are executed concurrently when users enter multiple commands at one time using the **'&'** operator.

**Program Errors:** Any invalid user input results in an error message being printed to the stderr.

## Usage

To run the program, execute the command **./rush** in the terminal with no other arguments.

 
 
