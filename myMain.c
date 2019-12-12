/**!
 * myMain.c
 * Process Diagnostician
 * Made by ahdelron.
 */

#define _XOPEN_SOURCE 700		// POSIX.1-2008 + XSI (SuSv4)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <regex.h>
#include <stdarg.h>
#include <ftw.h>
#include <sys/stat.h>
#include <fcntl.h>
//#define _XOPEN_SOURCE 700		// POSIX.1-2008 + XSI (SuSv4)
#define PROC_FILESYSTEM "/proc" // Linux(Ubuntu) Process List Folder
#define STAT_FILE "stat"		// We can find buffer that have process information in stat file.
#define CMD_FILE "cmdline" 
#define CLR_DEFAULT "\x1b[0m"
#define CLR_BOLD "\x1b[1m"
#define CLR_RED "\x1b[31m"
#define CLR_BOLDGREEN "\x1b[1m\x1b[32m"
#define MAX_FD 15				// Maximum number of file descriptors to use
#define BLOCK_SIZE 4096
#define REG_MAX_MATCH 8
#define DEFAULT_STATE "~"
#define STATE_ZOMBIE "Z"

int fd;

unsigned int maxFD = MAX_FD; 	// Maximum number of file describtors to use
char buff; 						// buffer
char fileContent[BLOCK_SIZE];	// Text content of a file
char match[BLOCK_SIZE/4];		// Regex(Regular Expression)  match
char fileName[BLOCK_SIZE/64];	// Name of file to read
char indexPrompt[BLOCK_SIZE/64];// Input value for the prompt option
char *strPath;					// String part of a path in '/proc'
char *statContent;				// Text content of the process's stat file
char *cmdContent;				// Text content of the process's command file
char *procEntryColor;			// Color of process entry to print

typedef struct {
	unsigned int pid;
	unsigned int ppid;
	char name[BLOCK_SIZE/64];
	char state[BLOCK_SIZE/64];
	char cmd[BLOCK_SIZE];
	bool defunct;
} ProcStats;

va_list vargs;

static int cprintf(char *color, char *format, ...) {
	fprintf(stderr, "%s", color);
	va_start(vargs, format);
	// variable argument will be used.
	vfprintf(stderr, format, vargs);
	va_end(vargs);
	fprintf(stderr, "%s", CLR_DEFAULT);
	return EXIT_SUCCESS;
}

static char* readFile(char *format, ...) {
	va_start(vargs, format);
	// variable argument will be used.
	vsnprintf(fileName, sizeof(fileName), format, vargs);
	// format will be put in char* called by fileName.
	va_end(vargs);
	
	fd = open(fileName, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH); // 뒤에 옵션 없어도 되지 않을까?
	//test
	//             ReadOnly. STAT INODE Read User, Group, Other.
	// #include <sys/stat.h>
	
	if(fd == -1) return NULL;
	//무한루프 방지
	fileContent[0] = '\0';

	for(int i = 0; read(fd, &buff, sizeof(buff)) != 0; i++) {
		fileContent[i] = buff;
	}
	
	close(fd);
	return fileContent;
}

static ProcStats getProcStats(const char *procPath) {
	ProcStats procStats = {.state=DEFAULT_STATE};
	statContent = readFile("%s/%s", procPath, STAT_FILE);
	//if(statContent == NULL || formatStatContent(statContent)) return procStats;
	sscanf(statContent, "%u %64s %64s %u", &procStats.pid,
			procStats.name, procStats.state, &procStats.ppid);
	procStats.name[strnlen(procStats.name, sizeof(procStats.name))-1] = '\0';
	
	// 목적 ??
	memmove(procStats.name, procStats.name+1,
			strnlen(procStats.name, sizeof(procStats.name)));
	procStats.defunct = (strstr(procStats.state, STATE_ZOMBIE) != NULL);
	cmdContent = readFile("%s/%s", procPath, CMD_FILE);
	if(cmdContent == NULL) return procStats;
	strcpy(procStats.cmd, cmdContent);
	return procStats;
}

// Process Entry Receiver
static int procEntryRecv(const char *fpath, const struct stat *sb,
		int tflag, struct FTW *ftwbuf) {
	(void)sb;
	if(ftwbuf->level != 1 || tflag != FTW_D ||
			!strtol(fpath + ftwbuf->base, &strPath, 10) ||
			strcmp(strPath, "")) {
		return EXIT_SUCCESS;
	}

	ProcStats procStats = getProcStats(fpath);
	if(!strcmp(procStats.state, DEFAULT_STATE)) {
		cprintf(CLR_RED, "Failed to parse \"%s\".\n", fpath);
		return EXIT_FAILURE;
	}
	cprintf(CLR_DEFAULT, "%-6d\t%-6d\t%-2s\t%16.16s %.64s\n",
			procStats.pid, procStats.ppid, procStats.state,
			procStats.name, procStats.cmd);
	cprintf(CLR_BOLD, "%s\n", fpath);
	return EXIT_SUCCESS;
}

int checkProcs() {
	cprintf(CLR_BOLD,"%-6s\t%-6s\t%-2s\t%16.16s %s\n",
			"PID", "PPID", "STATE", "NAME", "COMMAND");
	/*
	   Function Prototype
	int nftw(const char *path, int (*fn)(const char *, const struct stat *, int , struct FTW *), int depth, int flags);
	*/
	if(nftw(PROC_FILESYSTEM, procEntryRecv, // #include <ftw.h>
				maxFD, FTW_PHYS)) {
		cprintf(CLR_RED, "ftw failed.\n"); // fail to search.
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

int main(void) {
	while(true) {
		cprintf(CLR_BOLDGREEN, "\n\n\n%40s\n%40s\n%40s\n%40s\n%40s\n\n", 
				"----------------------------------------",
				"-      Process Diagnostician 1.0       -",
				"-          Made By ahdelron.           -",
				"-            BaseSrc : zps             -",
				"----------------------------------------");
		cprintf(CLR_DEFAULT, "%s\n%s\n%s\n%s\n%s\n",
				"Select the number of operating options you want!",
				"1. View all processes.",
				"2. Diagnose Process.",
				"3. Show help.",
				"4. Quit.");
		cprintf(CLR_RED, "%s", "SELECT THE NUMBER : ");
		int c;
		scanf("%d", &c);
		switch(c) {
			case 1:
				checkProcs();
				break;
			case 2:
				cprintf(CLR_DEFAULT, "%s\n", "Not implemented");
				break;
			case 3:
				break;
			case 4:
				exit(0);
			default:
				cprintf(CLR_RED, "%s\n", "invalid input.");
				continue;
		}
	}
	return EXIT_SUCCESS;
}
