/*************************************************************************
	> File Name: miniShell.cpp
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Thu 31 Mar 2016 06:45:08 AM PDT
 ************************************************************************/

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define MAX_CMD_LENGTH 1024
#define MAX_ARG 20

struct CMD {
	char * args[MAX_ARG + 1];
};

CMD cmd;
char cmdLine[MAX_CMD_LENGTH + 1];
char avLine[MAX_CMD_LENGTH + 1];

int shell_exec();
int read_cmd();
int parse_cmd();
int exec_cmd();
int init();
void handler(int sig);

int main(int argc, char ** argv) {
    init();
	shell_exec();
	return 0;
}

void handler(int sig) {
	printf("\nminiShell$");
	fflush(stdout);
}

int init() {
	memset(cmdLine, 0, sizeof(cmdLine));
	memset(&cmd, 0, sizeof(cmd));
	memset(&avLine, 0, sizeof(avLine));
	signal(SIGINT, handler);
	signal(SIGQUIT, SIG_IGN);
}

// return 0 
int shell_exec() {
	while (1) {
		printf("miniShell$");
		fflush(stdout);
		if (-1 == read_cmd()) {
			printf("read_cmd error\n");
			exit(1);
		}
		if (-1 == parse_cmd()) {
			printf("parse_cmd error\n");
			exit(1);
		}
		if (-1 == exec_cmd()) {
			printf("exec_cmd error\n");
			exit(1);
		}
	}
	return 0;
}

int read_cmd() {
	if (fgets(cmdLine, MAX_CMD_LENGTH, stdin) == NULL) {
		return -1;
	}
	//	printf("%s\n", cmdLine);
	return 0;
}

int parse_cmd() {
	char * p = cmdLine;
	char * q = avLine;
	int i = 0;
	while (*p != '\0') {
		// skip all white space
		while (*p == ' ' || *p == '\t') ++p;
        // break if reach to the end
		if (*p == '\0' || *p == '\n') break;
		// point to the start address
		cmd.args[i] = q;
		while (*p != '\0' && *p != '\n' && *p != '\t' && *p != ' ') 
			*q++ = *p++;
		// end of one of the cmd arg
		*q++ ='\0';
		++i;
	}
	return 0;
}

int exec_cmd() {
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork error");
		exit(1);
	} else if (pid == 0) {
		// printf("exec_cmd\n");
		// printf("%s\n", cmd.args[0]);
		execvp(cmd.args[0], cmd.args);
	}
	wait(NULL);
	return 0;
}
