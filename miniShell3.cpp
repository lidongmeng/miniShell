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
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#define MAX_CMD_LENGTH 1024
#define MAX_ARG 20
#define FILE_NAME_LEN 1024
#define CMD_NUM 20

struct CMD {
	char * args[MAX_ARG + 1];
	int infd;
	int outfd;
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
int cmd_cnt = 0;
int lastpid = 0;

int shell_exec();
int read_cmd();
int parse_cmd();
int exec_cmd();
int init();
void handler(int sig);
bool check(const char*);
void getFileName(char *);
void get_command(int );
void print_cmd(CMD *);
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
	for (int i = 0; i < CMD_NUM; ++i) {
		cmds[i].infd = 0;
		cmds[i].outfd = 1;
	}
	memset(&avLine, 0, sizeof(avLine));
	memset(infile, 0, sizeof(infile));
	memset(outfile, 0, sizeof(outfile));
	signal(SIGINT, handler);
	signal(SIGQUIT, SIG_IGN);

	lineptr = cmdLine;
	avptr = avLine;
	lastpid = 0;
}

// return 0 
int shell_exec() {
	while (1) {
		printf("miniShell$");
		fflush(stdout);
		init();
		if (-1 == read_cmd()) {
			printf("read_cmd error\n");
			exit(1);
		}
		if (-1 == parse_cmd()) {
			printf("parse_cmd error\n");
			exit(1);
		}
		print_cmd(cmds);
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
			case '<':
			case '>':
				if (isword == 0)
					cmds[i].args[num] = NULL;
			default :
				return ;
		}
	}
}

void print_cmd(CMD *cmds) {
	printf("command cnt: %d\n", cmd_cnt);
	for (int m = 0; m < cmd_cnt; ++m) {
		for (int i = 0; cmds[m].args[i] != NULL; ++i) {
			printf("%s ", cmds[m].args[i]);
		}
		printf("\n");
	}
}

bool check(const char * s) {
	//printf("check:%s, original string = %s\n", s, lineptr);
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
	while (*lineptr != ' ' && *lineptr != '\t' && *lineptr != '\n'
			&& *lineptr != '\0' && *lineptr != '|' && *lineptr != '&' 
			&& *lineptr != '<' && *lineptr != '>') {
		*name++ = *lineptr++;
	}
	*name = '\0';
}



int parse_cmd() {
	if (check("\n")) return 0;
	get_command(0);
	//print_cmd(cmds[0]);
	if (check("<")) {
		getFileName(infile);
//		printf("fileName: %s\n", infile);
	}
	int i;
    for (i = 1; i < CMD_NUM; ++i) {
		if (check("|")) {
			get_command(i);
			//print_cmd(cmds[i]);
		} else {
			break;
		}
	}
	if (check(">")) {
		if (check(">")) append = true;
		getFileName(outfile);
//		printf("outputFileName: %s\n", outfile);
	}
	if (check("&")) {
		backGroundExec = 1;
	}
	if (check("\n")) {
		cmd_cnt = i;
		return i; 
	} else {
		return 0;
	}
}

void print_cmd_info(CMD * cmd) {
	printf("	cmd: ");
	for (int i = 0; cmd->args[i] != NULL; ++i) {
		printf("i= %d--%s ", i, cmd->args[i]);
	}
	printf("\n");
	printf("	infd: %d, outfd: %d\n", cmd->infd, cmd->outfd);
}

void fork_exec(CMD * pcmd, int n) {
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		if (pcmd->infd != 0) {
			dup2(pcmd->infd, 0);
		}
		if (pcmd->outfd != 1) {
			dup2(pcmd->outfd, 1);
		}
	   // print_cmd_info(pcmd);
		// print the input file content
	    if (n == 1) {
			char buf[1024];
			memset(buf, 0, sizeof(buf));
			if (read(pcmd->infd, buf, 1024) != 0) {
				printf("	%s\n", buf);
			}
			lseek(pcmd->infd, 0, SEEK_SET);
		}
		execvp(pcmd->args[0], pcmd->args);
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("fork error\n");
		exit(1);
	} else {
		lastpid = pid;
	}
}

int exec_cmd() {
    // input redirection
	if (infile[0] != '\0') {
		// printf("infileName: %s\n", infile);
		cmds[0].infd = open(infile, O_RDONLY);
		// printf("inputFd: %d\n", cmds[0].infd);
	}
    // output redirection
	if (outfile[0] != '\0') {
		// printf("outputFileName: %s\n", outfile);
		if (append) {
			cmds[cmd_cnt-1].outfd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
		} else {
			cmds[cmd_cnt-1].outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		}
	}
	int fds[2];
	for (int i = 0; i < cmd_cnt; ++i) {
		if (i < cmd_cnt - 1) {
			pipe(fds);
			cmds[i].outfd = fds[1];
			cmds[i+1].infd = fds[0];
		}
		fork_exec(&cmds[i], i);
	    int fd;	
		if ((fd = cmds[i].infd) != 0) close(fd);
		if ((fd = cmds[i].outfd) != 1) close(fd);
	}
	while (wait(NULL) != lastpid) ;
	return 0;
}
