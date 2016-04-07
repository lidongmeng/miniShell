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
#define FILE_NAME_LEN 1024
#define CMD_NUM 20

struct CMD {
	char * args[MAX_ARG + 1];
};

char infile[FILE_NAME_LEN];
char outfile[FILE_NAME_LEN];

CMD cmds[CMD_NUM];
char cmdLine[MAX_CMD_LENGTH + 1];
char avLine[MAX_CMD_LENGTH + 1];
int cmd_num;
bool backGroundExec = false;
bool append = false;
char * lineptr;
char * avptr;

int shell_exec();
int read_cmd();
int parse_cmd();
int exec_cmd();
int init();
void handler(int sig);
bool check(const char*);
void getFileName(char *);
void get_command(int );

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
	memset(cmds, 0, sizeof(cmds));
	memset(&avLine, 0, sizeof(avLine));
	memset(infile, 0, sizeof(infile));
	memset(outfile, 0, sizeof(outfile));
	signal(SIGINT, handler);
	signal(SIGQUIT, SIG_IGN);

	lineptr = cmdLine;
	avptr = avLine;
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
	//	if (-1 == exec_cmd()) {
	//		printf("exec_cmd error\n");
	//		exit(1);
	//	}
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

void get_command(int i) {
	int num = 0;
	int isword = 0;
	while (lineptr != '\0') {
		while (*lineptr == ' ' || *lineptr == '\t') ++lineptr;
		if (*lineptr == '\n' || *lineptr == '\0') break;
		cmds[i].args[num] = avptr;
		char ch = *lineptr;
		while (ch != '\0'&& ch != '\n' && ch != ' ' && ch != '\t' && ch != '&' && ch != '<' && ch != '>' && ch != '|') {
			*avptr = ch;
			avptr++;
			lineptr++;
			ch = *lineptr;
			isword = 1;
		}
		*avptr++ = '\0';
		switch (*lineptr) {
			case ' ':
			case '\t':
				num++;
				isword = 0;
				break;
			case '|':
			case '&':
			case '\n':
				if (isword == 0)
					cmds[i].args[num] = NULL;
			default :
				return ;
		}
	}
}

void print_cmd(CMD cmd) {
	for (int i = 0; cmd.args[i] != NULL; ++i) {
		printf("%s ", cmd.args[i]);
	}
	printf("\n");
}

bool check(const char * s) {
	printf("check:%s, original string = %s\n", s, lineptr);
	char * p;
	while (*lineptr == ' ' || *lineptr == '\t') ++lineptr;
	p = lineptr;
	while (*s != '\0' && *s == *p) {
		++s;
		++p;
	}
	if (*s == '\0') {
		lineptr = p;
		return true;
	} else {
		return false;
	}
}

void getFileName(char * name) {
	while (*lineptr == ' ' || *lineptr == '\t') ++lineptr;
	while (*lineptr != ' ' && *lineptr != '\t' && *lineptr != '\n' && *lineptr != '\0' && *lineptr != '|' && *lineptr != '&' && *lineptr != '<' && *lineptr != '>') {
		*name++ = *lineptr++;
	}
	*name = '\0';
}



int parse_cmd() {
	if (check("\n")) return 0;
	get_command(0);
	print_cmd(cmds[0]);
	if (check("<")) {
		getFileName(infile);
		printf("fileName: %s\n", infile);
	}
	int i;
    for (i = 1; i < CMD_NUM; ++i) {
		if (check("|")) {
			get_command(i);
			print_cmd(cmds[i]);
		} else {
			break;
		}
	}
	if (check(">")) {
		if (check(">")) append = true;
		getFileName(outfile);
		printf("outputFileName: %s\n", outfile);
	}
	if (check("&")) {
		backGroundExec = 1;
	}
	if (check("\n")) {
		return i; 
	} else {
		return 0;
	}
}

int exec_cmd() {
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork error");
		exit(1);
	} else if (pid == 0) {
		// printf("exec_cmd\n");
		// printf("%s\n", cmd.args[0]);
		//execvp(cmd.args[0], cmd.args);
	}
	wait(NULL);
	return 0;
}
