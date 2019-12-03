/**!
 * myMain.c
 * Process Diagnostician
 * Made by ahdelron.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <regex.h>
#include <stdarg.h>
#include <ftw.h>
#define PROC_FILESYSTEM "/proc" // Linux(Ubuntu) Process List Folder
#define STAT_FILE "stat"		// We can find buffer that have process information in stat file.
#define CMD_FILE "cmdline" 
#define CLR_DEFAULT "\x1b[0m"
#define CLR_BOLD "\x1b[1m"
#define CLR_RED "\x1b[31m"
#define MAX_FD 15				// Maximum number of file descriptors to use
#define BLOCK_SIZE 4096
#define REG_MAX_MATCH 8

int fd;

unsigned int defunctCount = 0;

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

ProcStats defunctProcs[BLOCK_SIZE/4];
va_list vargs;
regex_t regex;					// Regex struct
regmatch_t regMatch[REG_MAX_MATCH];

int cprintf(char *color, char *format, ...) {
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
	
	fd = open(fileName, O_RDONLY); // 뒤에 옵션 없어도 되지 않을까?
	//test
	//             ReadOnly. STAT INODE Read User, Group, Other.
	// #include <sys/stat.h>
	
	if(fd == -1) return NULL;
	fileContent[0] = '\0';

	for(int i = 0; read(fd, &buff, sizeof(buff)) != 0; i++) {
		fileContent[i] = buff;
	}
	
	close(fd);
	return fileContent;
}

int checkProcs() {
	cprintf(CLR_BOLD,"%-6s\t%-6s\t%-2s\t%16.16s %s\n",
			"PID", "PPID", "STATE", "NAME", "COMMAND");
	/*if(nftw(PROC_FILESYSTEM, procEntryRecv, // #include <ftw.h>
				maxFD, FTW_PHYS)) {
		cprintf(CLR_RED, "ftw failed.\n"); // fail to search.
		return EXIT_FAILURE;
	}*/
	
	for(unsigned int i = 0; i < defunctCount; i++) {
		cprintf(CLR_BOLD, "\n[%s%d%s]", CLR_RED,
				i+1, CLR_DEFAULT);
		// Print process's stats.
		fprintf(stderr, "\n Name:     %s\n PID:"
				"     %u\n PPID:     %u\n State:   %s\n",
				defunctProcs[i].name, defunctProcs[i].pid,
				defunctProcs[i].ppid, defunctProcs[i].state);
		if (strcmp(defunctProcs[i].cmd, "")) fprintf(stderr,
				" Command: %s\n", defunctProcs[i].cmd);
	}
	return EXIT_SUCCESS;
}

int main(void) {
	checkProcs();
	return EXIT_SUCCESS;
}
