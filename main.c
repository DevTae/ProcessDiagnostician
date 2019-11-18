/* main.c */
#include <stdio.h>
#include <stdlib.h> // function 'system()' is available.

int main(void) {
	system("pstree -p > pstree.txt");
	return 0;
}
