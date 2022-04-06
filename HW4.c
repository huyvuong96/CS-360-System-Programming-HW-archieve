/***********************************************************************
name: Huy Vuong
	assignment4 -- acts as a pipe using ":" to seperate programs.
description:	
	See CS 360 Processes and Exec/Pipes lecture for helpful tips.
***********************************************************************/

/* Includes and definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

/**********************************************************************
./assignment4 <arg1> : <arg2>

    Where: <arg1> and <arg2> are optional parameters that specify the programs
    to be run. If <arg1> is specified but <arg2> is not, then <arg1> should be
    run as though there was not a colon. Same for if <arg2> is specified but
    <arg1> is not.
**********************************************************************/

//this funtion is used to check if we encounter ":"
int checkArg(int argv_index, char* p_argv[]){
	for (int i = 1; i < argv_index; i++) { 
        if (!strcmp(":", p_argv[i])) {
            return i;
        }
    }
    return 0;
}

//this function is used to point child to parent if we have more than 1 command
void setChildArgs(char *c_argv[], char **p_argv, int pipe_index) {

    for (int i = 0; i < pipe_index; i++) {
        c_argv[i] = *(p_argv + i);
    }
    c_argv[pipe_index] = '\0';
}

void pipe_exec(int argc, char*argv[]){
	//these are index elements helped to keep track the parent argv
	char **p_argv = argv +1;
	int argv_index = argc -1;

	//check if there is a pipe
	int pipe_index;
	

	if( !(pipe_index = checkArg(argv_index, p_argv))){
		if(execvp(*p_argv, p_argv)){
			fprintf(stdout,"%s\n", strerror(errno));
			exit(1);
		}
	}

	//at this point, there is 1 pipe, prepare fork
	//save stdout for final parent
	int f_out = dup2(1,9);
	if(f_out == -1){
		fprintf(stdout,"%s\n", strerror(errno));
		exit(1);
	}

	//file descriptors elements
	//0:stdin
	//1:stdout
	//close stdin and stdout first
	close(0);
	close(1);

	int fd[2];
	if(pipe(fd) == -1){
		fprintf(stdout,"%s\n", strerror(errno));
		exit(1);
	}

	int rdr = fd[0];
	int wtr = fd[1];

	//copy child to new array
	char *c_argv[argc];
	setChildArgs(c_argv, p_argv, pipe_index);

	//fork begin
	//0: child
	//1: parent

	if(fork()){
		argv_index = (argv_index -1) - pipe_index; //-- to skip ":"
		p_argv += pipe_index +1;

		while( (pipe_index = checkArg(argv_index, p_argv)) >0){
			close(wtr);

			if(pipe(fd) == -1){
				fprintf(stdout,"%s\n", strerror(errno));
				exit(1);
			}

			rdr = fd[0];
			wtr = fd[1];
			setChildArgs(c_argv, p_argv, pipe_index);
			argv_index = (argv_index -1) - pipe_index; //-- to skip ":"
			p_argv += pipe_index +1;

			if(fork()){
				close(0);
				dup(rdr);
				close(rdr);

				dup(wtr);
				close(wtr);
				wtr = 1;
			}
			else{
				close(rdr);
				dup(wtr);
				close(wtr);

				if(execvp(*c_argv, c_argv)){
					fprintf(stdout,"%s\n", strerror(errno));
					exit(1);
				}
			}
		}

		close(wtr);
		dup2(f_out, 1);
		close(f_out);

		execvp(*p_argv, p_argv);
	}


	else {
		close(rdr);
		execvp(c_argv[0], c_argv);
	}
}

int main(int argc, char *argv[]){
    /* code */

    if(argc == 1){

        fprintf(stdout, "%s\n",strerror(errno));

        exit(0);

    }

    //check if command begin with":" 

    if(!strcmp(":", argv[1])){

        char *n_argv[argc];
        int n_argc = argc -1;

        for(int i = 0; i < (argc-1); i++){

            n_argv[i] = argv[i+1];

        }
       	n_argv[argc-1] = '\0';
        pipe_exec(n_argc, n_argv);

    }


    //check if command end with ":"

    if(!strcmp(":", argv[argc-1])){

        char *n_argv[argc];
        int n_argc = argc -1;

        for(int i = 0; i < (argc-1); i++){

            n_argv[i] = argv[i];

        }

        n_argv[argc-1] = '\0';

        pipe_exec(n_argc, n_argv);

    }

    //if no errors or trailing, begin pipe
    if((strcmp(":", argv[1]) != 0) && (strcmp(":", argv[argc-1]) != 0)){   	
    	pipe_exec(argc, argv);    
    }
    
    return 0;

}
