#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/shm.h>
#include  <sys/wait.h>
#include  <pthread.h>
#include  <sys/resource.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <errno.h>
#include  <sys/mman.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/

int pids[64] = {-1};
int j = 0;
int flag = 0;
int k;
int back_proc;

void handler(int signum){
	sleep(1);
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void handler1(int signum){
	// kill(getpid(),15);
	exit(EXIT_SUCCESS);
}

void handler2(int signum){
	// printf("ctrl+c pressed\n");
	kill(k,15);
}

void handler3(int signum){
	kill(back_proc,15);
}

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
		token[tokenIndex] = '\0';
		if (tokenIndex != 0){
		tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		strcpy(tokens[tokenNo++], token);
		tokenIndex = 0; 
		}
		} else {
		token[tokenIndex++] = readChar;
		}
	}
	
	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}


// Background Function
void background_exec(char line[]){
	signal(SIGINT,SIG_IGN);
	signal(SIGUSR2,handler3);
	char **tokens;
	tokens = tokenize(line);
		int f = fork();
		if (f == 0){
			execvp(tokens[0],tokens);
			printf("Command execution failed.\n");
			_exit(1);
		}
		else if (f > 0){
			back_proc = f;
			int w = waitpid(f,NULL,0);
			printf("Shell: Background process finished\n");
			kill(getppid(),SIGUSR1);
		}
		else 
			fprintf(stderr, "%s\n", "Child process not created");
}

// EXIT FUNCTION
void exit_kill(int pids[], int j){
	j--;
	while (j >= 1){
		if (pids[j] != -1){
			kill(pids[j],SIGUSR2);
			pids[j] = -1;
			j--;
		}
	}
	if (j <= 0)
		kill(pids[0],SIGUSR2);
}


int main(int argc, char* argv[]) {
	signal(SIGUSR1,handler);
	signal(SIGUSR2,handler1);
	signal(SIGINT, handler2);
	char  line[MAX_INPUT_SIZE], path[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("%s:%s $ ", getlogin(), getcwd(path, sizeof(path)));
		scanf("%[^\n]", line);
		getchar();

		line[strlen(line)] = '\n';
		tokens = tokenize(line);

		// Simple Shell
		if (tokens[0] == NULL)
			continue;
		pids[j++] = getpid();
		int pro = fork();

		// Parent Shell
		if (pro < 0)
			fprintf(stderr, "%s\n", "Child process not created");
		else if (pro > 0){
			if (line[0] == 'c' && line[1] == 'd' && line[2] == ' '){
				// printf("Change directory to be implemented\n");
				tokens = tokenize(line+3);
				int err = chdir(tokens[0]);
				if (err == -1){
					if (errno == ENOENT)
						printf("Directory doesn't exist!\n");
					else if (errno == ENOTDIR)
						printf("Some Component of given path is not a directory!\n");
					else 
						printf("Shell : Incorrect command\n");
				}
			}
			if (line[0] == 'c' && line[1] == 'd'){
				if (line[2] == '\n')
					chdir("/home/niraj");
			}
			char temp = line[strlen(line)-2];
			if (temp == '&'){
				pids[j++] = pro;
			}
			if (temp != '&'){
				k = pro;
				waitpid(pro,NULL,0);
			}
		}
		else{
			char temp = line[strlen(line)-2];
			if (line[0] == 'c' && line[1] == 'd' && (line[2] == '\n' || line[2] == ' ')){
				exit(EXIT_SUCCESS);
			}
			else if (temp == '&'){
				line[strlen(line)-2] = '\n';
				line[strlen(line)-1] = '\0';
				background_exec(line);
				exit(EXIT_SUCCESS);
			}
			else if (line[0] == 'e' && line[1] == 'x' && line[2] == 'i' && line[3] == 't' && 
				(line[4] == ' ' || line[4] == '\n')){
				exit_kill(pids,j);
				exit(EXIT_SUCCESS);
			}
			else{
				execvp(tokens[0],tokens);
				printf("INCORRECT COMMAND.\n");
				exit(EXIT_FAILURE);
			}
		}
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}