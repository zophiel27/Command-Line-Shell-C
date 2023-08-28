# Shell

A C program that implements a simple shell with features like command history, input/output redirection, and process management using forks and pipes. It allows you to run various commands, manage history, and execute commands with different options on linux/ubuntu.

<ol>
<li>Command Execution:</li>

The program reads user input and tokenizes it into an array of arguments.
The program supports both normal command execution and background execution (controlled by the flag variable).
The program checks for built-in commands like "exit" and "history," which are handled separately.
If the input starts with '!', it interprets the input as a history command and retrieves the appropriate command from the history.

<li>Command History:</li>

The history array stores the command history.
The add_history function adds a command to the history, making sure the history doesn't exceed the maximum length.
The print_history function prints the stored history which the user will be able to access by entering the command: $ history. When the user enters !!, the most recent command in the history is executed. When the user enters a single ! followed by an integer N, the Nth command in the history is executed.

<li>I/O Redirection:</li>

The program detects input and output redirection operators ("<", ">", "1>", "2>") and handles them.
It supports both input and output redirection simultaneously.
The file paths for input and output redirection are stored in the file_path variable.

<li>Pipes:</li>

The program detects pipe characters "|" in the input and handles them.
It sets up pipes for communication between commands and executes each command separately.
Pipes are implemented using the pipe system call, and file descriptors are manipulated using dup2.

<li>Execution Flow:</li>

The program creates child processes using the fork system call for command execution.
In the child process, it redirects standard input/output as needed and then executes the command using execvp.

<li>Foreground and Background Execution:</li>

The flag variable determines whether a command should be executed in the foreground or background.
If a command ends with an ampersand (&), it's considered a background command.

<li>Cleanup:</li>

The program uses sleep(1) to pause for a short time before accepting the next command.
It cleans up resources by freeing memory and closing file descriptors.

</ol>