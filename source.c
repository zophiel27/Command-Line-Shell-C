#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAX_Length 100
#define MAX_HistoryLength 10

char* history[MAX_HistoryLength];
int history_size = 0;

void add_history(char* command) {

    if (history_size >= MAX_HistoryLength) {
        // remove the oldest command from the history
        free(history[0]);
        for (int i = 0; i < history_size - 1; i++) {
            history[i] = history[i + 1];
        }
        history_size--;
    }

    // append the command to the end of the history
    char* cmd_copy = strdup(command);
    history[history_size] = cmd_copy;
    history_size++;
}
void print_history(){

	if (!history_size){
		printf("No commands in history\n");
		return ;
	}
	printf("History: \n");
    for(int j = 0; j < history_size; j++) {
    	printf("%d- %s", j+1, history[j]);
    }
}

// get the nth command in the history
char* get_history(int n) {
    if (n < 0 || n >= history_size) {
        return NULL;
    }

    return history[n];
}

int main() 
{
    char input[MAX_Length]={'\0'};
    int flag = 0; // flag for background execution
    
    while(1)
    {  
        printf(">> ");
        fgets(input, sizeof(input), stdin); // getline function // sizeof(input)= MAX_Length
        
        //checking for 'exit' or 'history' commands
        if(strcmp(input, "exit\n") == 0) {
            break; // exit if command is "exit"
        }
		if(strcmp(input, "history\n") == 0) {
            print_history(); // print history on history command
            continue;
        }
        
        // checking for a history command
        int history_cmd_flag= 0;
		int n;
		if (input[0] == '!') 
		{
			history_cmd_flag= 1;
			//printf("history feature\n");
			if (input[1] == '!') {
				n = history_size - 1;
				//printf("most recent no. %d\n",n+1);
			} 
			else {
				n = input[1] - 48 - 1;
				//printf("%dth most recent\n", n+1);
			}
		}
	    char* input_copy;
		if (history_cmd_flag) 
		{
			// retrieve command from history
			char* history_command = get_history(n);
			if (history_command == NULL) {
				printf("No command in history\n");
				continue;
			}
			input_copy = strdup(history_command);
		} 
		else {
			// add command to history
			add_history(input);
			input_copy = strdup(input);
		}

		// tokenize the command copy
		int i = 0;
		char* args[MAX_Length];
		args[i] = strtok(input_copy, " \n");
		while (args[i] != NULL) {
			i++;
			args[i] = strtok(NULL, " \n");
		}
        
	//	printf("Commands: ");
    //    for(int j = 0; j < i; j++) {
    //    	printf("%s ", args[j]);
    //    }
    //    printf("\n");
        
        // checking for pipes
        int pipe_count = 0;
        for(int j = 0; j < i; j++) {
        		//printf("checking pipes, ");
            if(strcmp(args[j], "|") == 0) {
                pipe_count++;
            }
        }
        //printf("\n");
        
		//checking for I/O redirection
		int io_redirection = 0; // flag for I/O redirection
		char *file_path; // path for input/output file
		int output_fd= 0;
		int error_fd= 0;
		int outflag=0; int errflag=0;
		for(int j = 0; j < i; j++) 
		{
		    	//printf("checking i/o %d, ", j+1);
			if(strcmp(args[j], ">") == 0) {
				//output redirection
				io_redirection = 1;
				file_path = args[j+1];
				args[j] = NULL;
				break;
			} 
			else if(strcmp(args[j], "<") == 0) {
				//input redirection
				io_redirection = 2;
				file_path = args[j+1];
				args[j] = NULL;
				break;
			}
			else if(strcmp(args[j], "1>") == 0) {
				//also output
				io_redirection = 1;
				outflag=1;
				file_path = args[j+1];
				args[j] = NULL;
				output_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				dup2(output_fd, STDOUT_FILENO);
				break;
			}
			else if (strcmp(args[j], "2>") == 0) {
			    // stderr redirection
			    io_redirection = 1;
				errflag=1;			    
				file_path = args[j+1];
				args[j] = NULL;
			    error_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			    dup2(error_fd, STDERR_FILENO);
			    break;
			}
		}
		
		if(pipe_count > 0) 	// handles pipes
        {
        	//printf("%d pipes found\n", pipe_count);
            
            int pipefds[2*pipe_count];
            for(int j = 0; j < pipe_count; j++) {
                if(pipe(pipefds + j*2) < 0) {
                    printf("Erorr: pipe error\n");
                    continue;
                }
            }
            
            int command_start = 0;
            int command_end = 0;
            int current_pipe = 0;
            
            while(args[command_end] != NULL) 
            {
                if(strcmp(args[command_end], "|") == 0) 
                {
                    args[command_end] = NULL; // replace "|" with NULL to terminate command
                    pid_t pid = fork(); // create child process
                    
                    if(pid == 0) 
                    {
                        // child process
                        if(current_pipe > 0) 
                        {
                            // not the first command
                            dup2(pipefds[(current_pipe-1)*2], 0); // read from previous pipe
                        }
                        
                        if(current_pipe < pipe_count) 
                        {
                            // not the last command
                            dup2(pipefds[current_pipe*2+1], 1); // write to next pipe
                        }
                        
                        // close all pipe ends
                        for(int j = 0; j < 2*pipe_count; j++) {
                            close(pipefds[j]);
                        }
                        
                        execvp(args[command_start], &args[command_start]);
                        printf("Error: command not found\n"); // execvp only returns if there's an error
                        continue;
                    } 
                    else if(pid < 0) {
                        // fork failed
                        printf("Error: fork failed 1\n");
                        return 0;
                    } 
                    
                    // parent process
                    command_start = command_end + 1; // move to next command
                    current_pipe++; // move to next pipe
                }
                
                command_end++;
            }
            
            pid_t pid = fork(); // create child process
            if(pid == 0) {
                // last command
                if(pipe_count > 0) {
                    dup2(pipefds[(pipe_count-1)*2], STDIN_FILENO); // read from last pipe
                }
                
                // close all pipe ends in child
                for(int j = 0; j < 2*pipe_count; j++) {
                    close(pipefds[j]);
                }
                
                execvp(args[command_start], &args[command_start]);
                printf("Error: command not found\n"); // execvp only returns if there's an error
                return 0;
			}
        	else if(pid < 0){
                // fork failed
                printf("Error: fork failed 2\n");
            } 
			// close all pipe ends in the parent process as well
			for(int j = 0; j < 2*pipe_count; j++) {
				close(pipefds[j]);
			}
		}
		else	// handles normal commands and i/o redirection
		{
			pid_t pid = fork(); // create child process

			if(pid == 0) 
			{
				// child process
				if(io_redirection == 1) 
				{
						printf("\n");
						//printf("i/o found\n");
				    // output redirection
				    if (outflag) {
				        close(output_fd);
				    } 
				    else if (errflag) {
				        close(error_fd);
				    }
				    FILE *out_file = freopen(file_path, "w", stdout); // open output file in write mode
				    if(out_file == NULL) {
				        printf("Error: failed to open output file\n");
				        return 0;
				    }
				    execvp(args[0], args); // execute command
				    fclose(out_file); // close output file
				} 
				else if(io_redirection == 2) 
				{
				    // input redirection
						printf("\n");
						//printf("i/o found\n");
				    FILE *in_file = freopen(file_path, "r", stdin); // open input file in read mode
				    if(in_file == NULL) {
				        printf("Error: failed to open input file\n");
				        return 0;
				    }
				    execvp(args[0], args); // execute command
				    fclose(in_file); // close input file
				} 
				else 
				{
				    // no I/O redirection
				    	//printf("\nnormal command\n");
				    execvp(args[0], args); // execute command
				    printf("Error: command not found\n"); // execvp only returns if there's an error
				    return 0;
				}
			} 
			else if(pid < 0) 
			{
				    // fork failed
				    printf("Error: fork failed 3\n");
				} 
			else 
			{
		        // parent process
		        if(!flag) {
		            // foreground execution
		            waitpid(pid, NULL, 0); // wait for child process to finish
		        } 
		        else {
		            // background execution
		            printf("Command executing in background\n");
		        }
		    }
		}
		sleep(1);
	}
	return 0;
	
}