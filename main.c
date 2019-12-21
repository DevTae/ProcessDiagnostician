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
#define CLR_BLUE "\x1b[34m"
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

char path_info[1024];
// During c <=> python bidirection process, storing data to below variables.
char scan_info[1024];
char* resource_code = NULL;

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
	//cprintf(CLR_BOLD, "%s\n", fpath);
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

int int_pow_10(int v) {
	int result = 1;
	while(--v >= 0) result *= 10;
	return result;
}

int int_length(int v) {
	for(int i = 9; i >= 0; i--) {
		if(v / int_pow_10(i) != 0) return i+1;
	}
}

char* itoa(int v) {
	int length = int_length(v);
	char* a = (char*)malloc(sizeof(char) * (length+1));
	for(int i = length-1; i >= 0; i--) {
		a[length-1-i] = v / int_pow_10(i) + 48;
		v %= int_pow_10(i);
	}
	a[length] = '\0';
	return a;
}

int python_post_cmd(char* argv) {
	FILE *fp;	
	char p_arg[1024] = "python vtapi_post.py ";
	strcat(p_arg, argv);
	// Open the command for reading.
	fp = popen(p_arg, "r");
	if(fp == NULL) {
		cprintf(CLR_RED, "Failed to run command.\n");
		return EXIT_FAILURE;
	}

	while(fgets(scan_info, sizeof(scan_info), fp) != NULL) {
		if(strstr(scan_info, "resource      :") != NULL) {
			resource_code = (char*)malloc(sizeof(char)*1024);
			strcpy(resource_code, scan_info);
		}
		cprintf(CLR_DEFAULT, "%s", scan_info);
	}
	
	if(strstr(resource_code, "resource      :") == NULL) {
		return EXIT_FAILURE;
	} else {
		resource_code += (size_t)16;
	}
	
	int i;
	for(i = 0; *(resource_code+i) != '\n'; i++);
	*(resource_code+i) = '\0';

	printf("\n");

	pclose(fp);
	
	return EXIT_SUCCESS;
}

int python_get_cmd(char* argv) {
	FILE *fp;	
	char p_arg[1024] = "python vtapi_get.py ";
	strcat(p_arg, argv);
	// Open the command for reading.
	fp = popen(p_arg, "r");
	if(fp == NULL) {
		cprintf(CLR_RED, "Failed to run command.\n");
		return EXIT_FAILURE;
	}
	
	char* pin;
	while(fgets(scan_info, sizeof(scan_info), fp) != NULL) {
		if(*scan_info == 'T')
			return EXIT_FAILURE;
		cprintf(CLR_DEFAULT, "%s", scan_info);
	}
	
	pclose(fp);
	
	return EXIT_SUCCESS;
}

int result_file_cmd(int pid) {
	FILE *fp;	
	char* cp_pid = itoa(pid);
	char p_arg[1024] = "file /proc/";
	strcat(p_arg, cp_pid);
	strcat(p_arg, "/exe");
	free(cp_pid);

	// Open the command for reading.
	fp = popen(p_arg, "r");
	if(fp == NULL) {
		cprintf(CLR_RED, "Failed to run command.\n");
		return EXIT_FAILURE;
	}

	fgets(path_info, sizeof(path_info), fp);
	for(int i = 0; i < 1024; i++) { // '\n' to '\0'
		if(path_info[i] == '\n') {
			path_info[i] = '\0';
			break;
		}
	}

	//cprintf(CLR_DEFAULT, "%s\n", path_info);
	pclose(fp);
	return EXIT_SUCCESS;
}

int diagnose(int pid) {
	result_file_cmd(pid);
	char* file_path;
	if((file_path = strstr(path_info, ": symbolic link to ")) == NULL) {
		cprintf(CLR_RED, "%s\n", "Program cannot be founded.");
		return EXIT_FAILURE;
	} else { // when program is founded.
		cprintf(CLR_BOLD, "%s\n", "Suspected program has been founded. Continue the process, So that this program do diagnosis by using VirusTotal API.");
		//  processing the char*
		file_path += (size_t)19; // as like using substring
		printf("filepath is %s\n", file_path);
		// collecting file path
		if(python_post_cmd(file_path) == EXIT_SUCCESS) {
			printf("resource data is %s\n", resource_code);
			int i;
			for(i = 0; *(resource_code+i) != '\n'; i++);		
			*(resource_code+i) = '\0';
			// Getting dianosis process
			cprintf(CLR_BOLD, "%s\n", "- You have only four chances that can see scan data per minute.");
			while(true) {
				cprintf(CLR_BLUE, "\n%s ", "Do you want to see scan result? [Enter 'y']");
				int trash; // waiting
				scanf("%d", &trash);
				getchar();
				if(python_get_cmd(resource_code) == EXIT_FAILURE) {
					cprintf(CLR_DEFAULT, "%s", "The result hasn't been revealed. Please try this after for a moment.");
					continue;
				} else break;
			}
			free(resource_code);
		} else {
			cprintf(CLR_RED, "\n%s\n", "Processing that uploads the suspected file failed.");
			return EXIT_FAILURE;
		}
	}
}

int main(void) {
	cprintf(CLR_BOLDGREEN, "\n%40s\n%40s\n%40s\n%40s\n%40s\n", 
				"----------------------------------------",
				"-      Process Diagnostician 1.0       -",
				"-          Made By ahdelron.           -",
				"-            BaseSrc : zps             -",
				"----------------------------------------");
	cprintf(CLR_DEFAULT, "\n%s\n%s\n%s\n%s\n%s\n",
				"** Select the number of operating options you want! **",
				"1. View all processes.",
				"2. Diagnose Process.",
				"3. Show help.",
				"4. Quit.");
	while(true) {
		cprintf(CLR_RED, "%s", "\nSelect the mode. : ");
		int c = 0;
		scanf("%d", &c);
		switch(c) {
			case 1:
				checkProcs();
				break;
			case 2:
				//cprintf(CLR_DEFAULT, "\n%s\n", "2: Not implemented.");
				{
					int pid;
					scanf("%d", &pid);
					diagnose(pid);
				}
				break;
			case 3:
				cprintf(CLR_RED, "\n%s",
						"--PROCESS DIAGNOSTICIAN MANUAL--");
				cprintf(CLR_DEFAULT, "\n%s\n%s",
						" Fisrt, View all processes.",
						" Second, If you find suspected process, you should enter the command \"2 pid\" as like below.");
				cprintf(CLR_BLUE, "\n%s\n", "> 2 1234");
				break;
			case 4:
				exit(0);
				break;
			default:
				cprintf(CLR_RED, "\n%s\n", "You entered invalid input.");
		}
		getchar(); // clean the buffer
	}
	
	return EXIT_SUCCESS;
}
